#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;

class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor>
{
public:
    explicit FindNamedClassVisitor(ASTContext* context)
      : context(context)
    {}

    bool VisitCXXRecordDecl(CXXRecordDecl* Declaration)
    {
        if (Declaration->getQualifiedNameAsString() == "n::m::C") {
            FullSourceLoc fullLocation =
                context->getFullLoc(Declaration->getBeginLoc());
            if (fullLocation.isValid())
                llvm::outs() << "Found declaration at "
                             << fullLocation.getSpellingLineNumber() << ":"
                             << fullLocation.getSpellingColumnNumber() << "\n";
        }
        return true;
    }

private:
    ASTContext* context;
};

class FindNamedClassConsumer : public clang::ASTConsumer
{
public:
    explicit FindNamedClassConsumer(ASTContext* context)
      : visitor(context)
    {}

    virtual void HandleTranslationUnit(clang::ASTContext& context)
    {
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    FindNamedClassVisitor visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction
{
public:
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& Compiler,
        llvm::StringRef InFile)
    {
        return std::unique_ptr<clang::ASTConsumer>(
            new FindNamedClassConsumer(&Compiler.getASTContext()));
    }
};

int
main(int argc, char** argv)
{
    if (argc > 1) {
        clang::tooling::runToolOnCode(std::make_unique<FindNamedClassAction>(),
                                      argv[1]);
    }
}
