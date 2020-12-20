/// findNamedClass: Simple utility for finding and printing occurrances of
/// a specified class name in a given source file.

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <fstream>

using namespace clang;

class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor>
{
public:
    explicit FindNamedClassVisitor(const char* className, ASTContext* context)
      : _className(className)
      , _context(context)
    {}

    bool VisitCXXRecordDecl(CXXRecordDecl* Declaration)
    {
        llvm::outs() << "Checking: " << Declaration->getQualifiedNameAsString()
                     << "\n";
        if (Declaration->getQualifiedNameAsString() == _className) {
            FullSourceLoc fullLocation =
                _context->getFullLoc(Declaration->getBeginLoc());
            if (fullLocation.isValid())
                llvm::outs() << "Found declaration at "
                             << fullLocation.getSpellingLineNumber() << ":"
                             << fullLocation.getSpellingColumnNumber() << "\n";
        }
        return true;
    }

private:
    ASTContext* _context = nullptr;
    const char* _className = nullptr;
};

class FindNamedClassConsumer : public clang::ASTConsumer
{
public:
    explicit FindNamedClassConsumer(const char* className, ASTContext* context)
      : _visitor(className, context)
    {}

    virtual void HandleTranslationUnit(clang::ASTContext& context)
    {
        _visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    FindNamedClassVisitor _visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction
{
public:
    explicit FindNamedClassAction(const char* className)
      : _className(className)
    {}

    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& Compiler,
        llvm::StringRef InFile)
    {
        return std::unique_ptr<clang::ASTConsumer>(
            new FindNamedClassConsumer(_className, &Compiler.getASTContext()));
    }

private:
    const char* _className = nullptr;
};

int
main(int argc, char** argv)
{
    if (argc != 3) {
        llvm::outs() << "usage: findNamedClass <CLASS_NAME> <FILE>\n";
        return EXIT_FAILURE;
    }

    // Open file.
    std::ifstream inputFile(argv[2]);
    std::string code;

    // Pre-allocate code string size.
    inputFile.seekg(0, std::ios::end);
    code.reserve(inputFile.tellg());
    inputFile.seekg(0, std::ios::beg);

    // Copy entire contents of file into code.
    code.assign((std::istreambuf_iterator<char>(inputFile)),
                std::istreambuf_iterator<char>());

    clang::tooling::runToolOnCode(
        std::make_unique<FindNamedClassAction>(argv[1]), code.c_str());

    return EXIT_SUCCESS;
}
