  
#ifndef _SAFEC_RUNTIME_H_
#define _SAFEC_RUNTIME_H_

#include <vector>
#include <tuple>

#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

class runtime_info
{
    llvm::GlobalVariable *input_var;
    llvm::GlobalVariable *output_var;
    llvm::GlobalVariable *line;
    llvm::GlobalVariable *pos;
    llvm::GlobalVariable *str;
    llvm::Function *input_func;
    llvm::Function *output_func;
    llvm::Function *obc_check_error_func;

  public:
    runtime_info(llvm::Module *module);

    std::vector<std::tuple<std::string, llvm::GlobalValue *, bool, bool, bool, bool, int>> get_language_symbols();

    std::vector<std::tuple<std::string, void *>> get_runtime_symbols();
};

#endif
