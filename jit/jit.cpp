#include <jit/jit.h>

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

namespace jit
{
JIT::JIT( llvm::orc::JITTargetMachineBuilder i_targetMachineBuilder, llvm::DataLayout i_dataLayout )
    : m_objectLayer( m_executionSession, []() { return std::make_unique< llvm::SectionMemoryManager >(); } )
    , m_compileLayer( m_executionSession,
                      m_objectLayer,
                      llvm::orc::ConcurrentIRCompiler( std::move( i_targetMachineBuilder ) ) )
    , m_dataLayout( std::move( i_dataLayout ) )
    , m_mangler( m_executionSession, this->m_dataLayout )
    , m_context( std::make_unique< llvm::LLVMContext >() )
{
    m_executionSession.getMainJITDylib().setGenerator( llvm::cantFail(
        llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess( m_dataLayout.getGlobalPrefix() ) ) );
}

llvm::Expected< std::unique_ptr< JIT > > JIT::Create()
{
    llvm::Expected< llvm::orc::JITTargetMachineBuilder > machineBuilder =
        llvm::orc::JITTargetMachineBuilder::detectHost();
    if ( !machineBuilder )
    {
        return machineBuilder.takeError();
    }

    llvm::Expected< llvm::DataLayout > dataLayout = machineBuilder->getDefaultDataLayoutForTarget();
    if ( !dataLayout )
    {
        return dataLayout.takeError();
    }

    return std::make_unique< JIT >( std::move( *machineBuilder ), std::move( *dataLayout ) );
}

} // namespace jit
