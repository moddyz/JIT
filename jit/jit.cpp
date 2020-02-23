#include <jit/jit.h>

namespace jit
{
JIT::JIT( llvm::JITTargetMachineBuilder i_targetMachineBuilder, llvm::DataLayout i_dataLayout )
    : m_objectLayer( m_executionSession, []() { return std::make_unique< llvm::SectionMemoryManager >(); } )
    , m_compileLayer( m_executionSession, m_objectLayer, ConcurrentIRCompiler( std::move( i_targetMachineBuilder ) ) )
    , m_dataLayout( std::move( i_dataLayout ) )
    , m_mangler( m_executionSession, this->m_dataLayout )
    , m_context( std::make_unique< LLVMContext >() )
{
    m_executionSession.getMainJITDylib().setGenerator(
        cantFail( DynamicLibrarySearchGenerator::GetForCurrentProcess( m_dataLayout ) ) );
}

} // namespace jit
