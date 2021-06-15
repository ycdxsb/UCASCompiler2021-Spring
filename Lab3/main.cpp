#include <iostream>
#include <fstream>
#include <string.h>

#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>

#include "antlr4-runtime.h"
#include "SafeCLexer.h"
#include "SafeCParser.h"
#include "SafeCParserVisitor.h"
#include "AstBuilder.h"
#include "SafeCIRBuilder.h"

using namespace antlrcpp;
using namespace antlr4;

using namespace llvm;
using namespace std;

int main(int argc, const char **argv)
{
  bool debug = false;
  std::ifstream stream;
  std::string filename;
  if (argc != 2 && argc != 3)
  {
    cout << "usage : ./irbuilder filepath [-d]" << endl;
    cout << "help  : -d debug" << endl;
    return -1;
  }
  filename = argv[1];
  if (argc == 3 && strcmp(argv[2], "-d") == 0)
  {
    debug = true;
  }
  stream.open(filename);
  ANTLRInputStream input(stream);

  SafeCLexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  tokens.fill();
  if (debug)
  {
    for (auto token : tokens.getTokens())
    {
      cout << token->toString() << endl;
    }
  }

  SafeCParser parser(&tokens);
  auto tree = parser.compUnit();
  if (debug)
  {
    cout << tree->toStringTree(&parser) << endl;
  }
  AstBuilder ast_builder;

  ast_builder.debug = debug;
  auto ast = ast_builder(tree);
  string module_name = filename;
  module_name = module_name.substr(module_name.find_last_of("/\\") + 1);

  LLVMContext ctx;
  SafeCIRBuilder irbuilder(ctx);
  irbuilder.debug = debug;
  irbuilder.build(module_name, ast);
  auto module = irbuilder.get_module();
  auto runtime = irbuilder.get_runtime_info();
  if (!module)
  {
      cerr << "Semantic failed. Exiting." << endl;
      exit(-1);
  }
  cout << endl
       << "[+]SAFEC IR GEN BEGIN[+]" << endl;
  cout << "Module Name " << module_name << endl;
  module->print(outs(), nullptr);
  cout << endl
       << "[+]SAFEC IR GEN END[+]" << endl;
  auto entry_func = module->getFunction("main");
  if (!entry_func)
  {
    cerr << "No 'main' function presented. Exiting." << endl;
    exit(-1);
  }
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  for (auto t : runtime->get_runtime_symbols())
    sys::DynamicLibrary::AddSymbol(get<0>(t), get<1>(t));

  string error_info;
  unique_ptr<ExecutionEngine> engine(EngineBuilder(move(module))
                                         .setEngineKind(EngineKind::JIT)
                                         .setErrorStr(&error_info)
                                         .create());
  if (!engine)
  {
    cerr << "EngineBuilder failed: " << error_info << endl;
    exit(-1);
  }
  cout << endl
       << "[+]SAFEC EXECUTE BEGIN[+]" << endl;
  engine->runFunction(entry_func, {});
  cout << endl
       << "[+]SAFEC EXECUTE END[+]" << endl;
  return 0;
}
