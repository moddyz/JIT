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
    explicit JIT( llvm::JITTargetMachineBuilder i_targetMachineBuilder, llvm::DataLayout i_dataLayout );

private:
    llvm::ExecutionSession         m_executionSession;
    llvm::RTDyldObjectLinkingLayer m_objectLayer;
    llvm::IRCompileLayer           m_compileLayer;

    llvm::DataLayout        m_dataLayout;
    llvm::MangleAndInterner m_mangler;
    llvm::ThreadSafeContext m_context;
}
} // namespace jit
