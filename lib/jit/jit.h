#pragma once

#include <memory>

#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>

namespace jit
{
/// Just-in-time compiler.
class JIT
{
public:
    explicit JIT( llvm::orc::JITTargetMachineBuilder i_targetMachineBuilder, llvm::DataLayout i_dataLayout );

    /// Create a new instance of the JIT.
    static llvm::Expected< std::unique_ptr< JIT > > Create();

private:
    llvm::orc::ExecutionSession         m_executionSession;
    llvm::orc::RTDyldObjectLinkingLayer m_objectLayer;
    llvm::orc::IRCompileLayer           m_compileLayer;
    llvm::orc::MangleAndInterner        m_mangler;
    llvm::orc::ThreadSafeContext        m_context;

    llvm::DataLayout m_dataLayout;
};
} // namespace jit
