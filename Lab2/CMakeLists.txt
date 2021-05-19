CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)
project(astbuilder)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# compiler must be 11 or 14
set(CMAKE_CXX_STANDARD 11)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)
# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE /home/ucascompile/ucascompile/antlr4/antlr-4.9.1-complete.jar)
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)
find_package(RapidJSON)

#Call macro to add lexer and grammar to your build dependencies.
antlr_target(SafeCGrammarLexer SafeCLexer.g4 LEXER
             PACKAGE antlrcpp)
antlr_target(SafeCGrammarParser SafeCParser.g4 PARSER
             PACKAGE antlrcpp
             DEPENDS_ANTLR SafeCGrammarLexer
             COMPILE_FLAGS -visitor -no-listener -lib ${ANTLR_SafeCGrammarLexer_OUTPUT_DIR})

# include generated files in project environment
include_directories(${ANTLR_SafeCGrammarLexer_OUTPUT_DIR})
include_directories(${ANTLR_SafeCGrammarParser_OUTPUT_DIR})

include_directories(include)
# add generated grammar to demo binary target
add_library(AstBuilder src/AstBuilder.cpp)
target_link_libraries(AstBuilder antlr4_static)

add_executable(astbuilder main.cpp  
               ${ANTLR_SafeCGrammarLexer_CXX_OUTPUTS}
               ${ANTLR_SafeCGrammarParser_CXX_OUTPUTS}
               )
target_link_libraries(astbuilder AstBuilder)
