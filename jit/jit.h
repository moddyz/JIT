#pragma once

#include <memory>

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"

namespace jit
{
class JIT
{
public:
    explicit JIT( llvm::JITTargetMachineBuilder JTMB, llvm::DataLayout m_dataLayout );
        : m_objectLayer( m_executionSession, []() { return std::make_unique< llvm::SectionMemoryManager >(); } )
        , m_compileLayer( m_executionSession, m_objectLayer, ConcurrentIRCompiler( std::move( JTMB ) ) )
        , m_dataLayout( std::move( m_dataLayout ) )
        , m_mangler( m_executionSession, this->m_dataLayout )
        , m_context( std::make_unique< LLVMContext >() )
    {
        m_executionSession.getMainJITDylib().setGenerator(
            cantFail( DynamicLibrarySearchGenerator::GetForCurrentProcess( m_dataLayout ) ) );
    }

private:
    llvm::ExecutionSession         m_executionSession;
    llvm::RTDyldObjectLinkingLayer m_objectLayer;
    llvm::IRCompileLayer           m_compileLayer;

    llvm::DataLayout        m_dataLayout;
    llvm::MangleAndInterner m_mangler;
    llvm::ThreadSafeContext m_context;
}
} // namespace jit
