//===-- examples/clang-interpreter/main.cpp - Clang C Interpreter Example -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string GetExecutablePath( const char* Argv0, void* mainAddr )
{
    return llvm::sys::fs::getMainExecutable( Argv0, mainAddr );
}

class SimpleJIT
{
private:
    llvm::orc::ExecutionSession            m_executionSession;
    std::unique_ptr< llvm::TargetMachine > m_targetMachine;
    const llvm::DataLayout                 m_dataLayout;
    llvm::orc::MangleAndInterner           m_mangler{m_executionSession, m_dataLayout};
    llvm::orc::RTDyldObjectLinkingLayer    m_objectLayer{m_executionSession, createMemMgr};
    llvm::orc::IRCompileLayer              m_compileLayer{m_executionSession,
                                             m_objectLayer,
                                             llvm::orc::SimpleCompiler( *m_targetMachine )};

    static std::unique_ptr< llvm::SectionMemoryManager > createMemMgr()
    {
        return std::make_unique< llvm::SectionMemoryManager >();
    }

    SimpleJIT( std::unique_ptr< llvm::TargetMachine >   i_targetMachine,
               llvm::DataLayout                         i_dataLayout,
               llvm::orc::DynamicLibrarySearchGenerator i_processSymbolsGenerator )
        : m_targetMachine( std::move( i_targetMachine ) )
        , m_dataLayout( std::move( i_dataLayout ) )
    {
        llvm::sys::DynamicLibrary::LoadLibraryPermanently( nullptr );
        m_executionSession.getMainJITDylib().setGenerator( std::move( i_processSymbolsGenerator ) );
    }

public:
    static llvm::Expected< std::unique_ptr< SimpleJIT > > Create()
    {
        llvm::Expected< llvm::orc::JITTargetMachineBuilder > jitTargetMachineBuilder =
            llvm::orc::JITTargetMachineBuilder::detectHost();
        if ( !jitTargetMachineBuilder )
        {
            return jitTargetMachineBuilder.takeError();
        }

        llvm::Expected< std::unique_ptr< llvm::TargetMachine > > targetMachine =
            jitTargetMachineBuilder->createTargetMachine();
        if ( !targetMachine )
        {
            return targetMachine.takeError();
        }

        llvm::DataLayout dataLayout = ( *targetMachine )->createDataLayout();

        llvm::Expected< llvm::orc::DynamicLibrarySearchGenerator > processSymbolsGenerator =
            llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess( dataLayout.getGlobalPrefix() );

        if ( !processSymbolsGenerator )
        {
            return processSymbolsGenerator.takeError();
        }

        return std::unique_ptr< SimpleJIT >( new SimpleJIT( std::move( *targetMachine ),
                                                            std::move( dataLayout ),
                                                            std::move( *processSymbolsGenerator ) ) );
    }

    const llvm::TargetMachine& getTargetMachine() const
    {
        return *m_targetMachine;
    }

    llvm::Error addModule( llvm::orc::ThreadSafeModule i_module )
    {
        return m_compileLayer.add( m_executionSession.getMainJITDylib(), std::move( i_module ) );
    }

    llvm::Expected< llvm::JITEvaluatedSymbol > findSymbol( const llvm::StringRef& i_name )
    {
        return m_executionSession.lookup( {&m_executionSession.getMainJITDylib()}, m_mangler( i_name ) );
    }

    llvm::Expected< llvm::JITTargetAddress > getSymbolAddress( const llvm::StringRef& i_name )
    {
        llvm::Expected< llvm::JITEvaluatedSymbol > symbol = findSymbol( i_name );
        if ( !symbol )
        {
            return symbol.takeError();
        }

        return symbol->getAddress();
    }
};

llvm::ExitOnError g_exitOnError;

int main( int argc, const char** argv )
{
    // This just needs to be some symbol in the binary; C++ doesn't
    // allow taking the address of ::main however.
    void*                                                 mainAddr       = ( void* ) ( intptr_t ) GetExecutablePath;
    std::string                                           executablePath = GetExecutablePath( argv[ 0 ], mainAddr );
    clang::IntrusiveRefCntPtr< clang::DiagnosticOptions > DiagOpts       = new clang::DiagnosticOptions();
    clang::TextDiagnosticPrinter* DiagClient = new clang::TextDiagnosticPrinter( llvm::errs(), &*DiagOpts );

    clang::IntrusiveRefCntPtr< clang::DiagnosticIDs > diagnosticIds( new clang::DiagnosticIDs() );
    clang::DiagnosticsEngine                          diagnosticsEngine( diagnosticIds, &*DiagOpts, DiagClient );

    const std::string TripleStr = llvm::sys::getProcessTriple();
    llvm::Triple      T( TripleStr );

    // Use ELF on Windows-32 and MingW for now.
#ifndef CLANG_INTERPRETER_COFF_FORMAT
    if ( T.isOSBinFormatCOFF() )
        T.setObjectFormat( llvm::Triple::ELF );
#endif

    g_exitOnError.setBanner( "clang interpreter" );

    clang::driver::Driver clangDriver( executablePath, T.str(), diagnosticsEngine );
    clangDriver.setTitle( "clang interpreter" );
    clangDriver.setCheckInputsExist( false );

    // FIXME: This is a hack to try to force the driver to do something we can
    // recognize. We need to extend the driver library to support this use model
    // (basically, exactly one input, and the operation mode is hard wired).
    clang::SmallVector< const char*, 16 > Args( argv, argv + argc );
    Args.push_back( "-fsyntax-only" );
    std::unique_ptr< clang::driver::Compilation > clangCompilation( clangDriver.BuildCompilation( Args ) );
    if ( !clangCompilation )
        return 0;

    // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

    // We expect to get back exactly one command job, if we didn't something
    // failed. Extract that job from the compilation.
    const clang::driver::JobList& Jobs = clangCompilation->getJobs();
    if ( Jobs.size() != 1 || !clang::isa< clang::driver::Command >( *Jobs.begin() ) )
    {
        clang::SmallString< 256 > Msg;
        llvm::raw_svector_ostream OS( Msg );
        Jobs.Print( OS, "; ", true );
        diagnosticsEngine.Report( clang::diag::err_fe_expected_compiler_job ) << OS.str();
        return 1;
    }

    const clang::driver::Command& Cmd = clang::cast< clang::driver::Command >( *Jobs.begin() );
    if ( llvm::StringRef( Cmd.getCreator().getName() ) != "clang" )
    {
        diagnosticsEngine.Report( clang::diag::err_fe_expected_clang_command );
        return 1;
    }

    // Initialize a compiler invocation object from the clang (-cc1) arguments.
    const llvm::opt::ArgStringList&              compilerArgs = Cmd.getArguments();
    std::unique_ptr< clang::CompilerInvocation > compilerInvocation( new clang::CompilerInvocation );
    clang::CompilerInvocation::CreateFromArgs( *compilerInvocation,
                                               const_cast< const char** >( compilerArgs.data() ),
                                               const_cast< const char** >( compilerArgs.data() ) + compilerArgs.size(),
                                               diagnosticsEngine );

    // Show the invocation, with -v.
    if ( compilerInvocation->getHeaderSearchOpts().Verbose )
    {
        llvm::errs() << "clang invocation:\n";
        Jobs.Print( llvm::errs(), "\n", true );
        llvm::errs() << "\n";
    }

    // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

    // Create a compiler instance to handle the actual work.
    clang::CompilerInstance compilerInstance;
    compilerInstance.setInvocation( std::move( compilerInvocation ) );

    // Create the compilers actual diagnostics engine.
    compilerInstance.createDiagnostics();
    if ( !compilerInstance.hasDiagnostics() )
        return 1;

    // Infer the builtin include path if unspecified.
    if ( compilerInstance.getHeaderSearchOpts().UseBuiltinIncludes &&
         compilerInstance.getHeaderSearchOpts().ResourceDir.empty() )
    {
        compilerInstance.getHeaderSearchOpts().ResourceDir =
            clang::CompilerInvocation::GetResourcesPath( argv[ 0 ], mainAddr );
    }

    // Create and execute the frontend to generate an LLVM bitcode module.
    std::unique_ptr< clang::CodeGenAction > codeGenAction( new clang::EmitLLVMOnlyAction() );
    if ( !compilerInstance.ExecuteAction( *codeGenAction ) )
        return 1;

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    int                                  result = 255;
    std::unique_ptr< llvm::LLVMContext > llvmContext( codeGenAction->takeLLVMContext() );
    std::unique_ptr< llvm::Module >      Module = codeGenAction->takeModule();

    if ( Module )
    {
        std::unique_ptr< SimpleJIT > J = g_exitOnError( SimpleJIT::Create() );

        g_exitOnError( J->addModule( llvm::orc::ThreadSafeModule( std::move( Module ), std::move( llvmContext ) ) ) );
        int ( *Main )( ... ) = ( int ( * )( ... ) ) g_exitOnError( J->getSymbolAddress( "main" ) );
        result               = Main();
        printf( "Main: %d\n", result );
    }

    // Shutdown.
    llvm::llvm_shutdown();

    return result;
}
