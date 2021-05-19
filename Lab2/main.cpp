#include <iostream>
#include <string.h>
#include "antlr4-runtime.h"
#include "SafeCLexer.h"
#include "SafeCParser.h"
#include "SafeCParserVisitor.h"
#include "AstSerializer.h"
#include "AstBuilder.h"

using namespace antlrcpp;
using namespace antlr4;
using namespace std;

int main(int argc, const char **argv)
{
  bool debug = false;
  std::ifstream stream;
  std::string filename;
  if (argc != 2 && argc != 3)
  {
    cout << "usage : ./astbuilder filepath [-d]" << endl;
    cout << "help  : -d debug" << endl;
    return -1;
  }
  filename = argv[1];
  if (argc == 3 && strcmp(argv[2],"-d")==0)
  {
    debug = true;
  }
  stream.open(filename);
  ANTLRInputStream input(stream);

  SafeCLexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  tokens.fill();
  for (auto token : tokens.getTokens())
  {
    cout << token->toString() << endl;
  }

  SafeCParser parser(&tokens);
  auto tree = parser.compUnit();

  cout << tree->toStringTree(&parser) << endl;

  AstBuilder ast_builder;
  rapidjson::StringBuffer s;
  rapidjson::PrettyWriter<decltype(s)> writer(s);

  ast_builder.debug = debug;
  auto ast = ast_builder(tree);
  AstSerializer<decltype(writer)> astserializer(writer);
  astserializer.debug = debug;
  astserializer.serialize(*ast);

  cout << "Ast:" << endl;
  cout << s.GetString() << endl;
  return 0;
}
