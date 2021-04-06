# Environment 

> 实验所需基础环境为ubuntu 16.04 x64 System

## 基础环境

```
apt update
apt install -y vim gcc g++  build-essential make git  uuid-dev libssl-dev pkg-config
```

安装高版本`cmake`

```
wget -c https://github.com/Kitware/CMake/releases/download/v3.19.6/cmake-3.19.6.tar.gz
tar -xvf cmake-3.19.6.tar.gz
cd cmake-3.19.6
./configure
make && make install 
```

安装`RapidJson`
```
git clone https://github.com/Tencent/rapidjson
cd rapidjson && git submodule update --init
mkdir build && cd build
cmake ..
make
make install
```

## Java环境
`antlr4`赖于`java`环境，因此需要先配置`java`
解压`jdk`压缩包

```
tar -xvf jdk-8u221-linux-x64.tar.gz 
mv jdk1.8.0_221 java8
```
配置`java`环境变量
```
# java env
export JAVA_HOME=/home/ucascompile/ucascompile/java8
export JRE_HOME=${JAVA_HOME}/jre 
export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib 
export PATH=${JAVA_HOME}/bin:$PATH
```

## ANTLR4环境
配置`antlr4 jar`包环境变量
```
# antlr env
export CLASSPATH=".:/home/ucascompile/ucascompile/antlr4/antlr-4.9.1-complete.jar:$CLASSPATH"
alias grun='java org.antlr.v4.gui.TestRig'
alias antlr4='java org.antlr.v4.Tool'
```

## 编译antlr4 编程时需要的依赖库
```
cd antlr4-4.9.1/runtime/Cpp/
mkdir build && cd build
cmake ..
make 
make install 
```

编译安装后`ANTLR4`的头文件位于` /usr/local/include/antlr4-runtime` ，运行时动态链接库位于`/usr/local/lib`

## LLVM环境
从`github`下载`11.0`版本的`LLVM`并以`Release`模式编译
```
// https://github.com/llvm/llvm-project/releases/tag/llvmorg-11.0.0

tar -xvf llvm-11.0.0.src.tar.xz
mv llvm-11.0.0.src llvm
tar -xvf clang-11.0.0.src.tar.xz
mv clang-11.0.0.src clang
mv clang llvm/tools/
tar -xvf compiler-rt-11.0.0.src.tar.xz
mv compiler-rt-11.0.0.src llvm/projects/compiler-rt
tar -xvf libcxx-11.0.0.src.tar.xz
mv libcxx-11.0.0.src llvm/projects/libcxx
tar -xvf libcxxabi-11.0.0.src.tar.xz
mv libcxxabi-11.0.0.src llvm/projects/libcxxabi
tar -xvf lldb-11.0.0.src.tar.xz
mv lldb-11.0.0.src llvm/tools/lldb
tar -xvf lld-11.0.0.src.tar.xz
mv lld-11.0.0.src llvm/tools/lld
tar -xvf clang-tools-extra-11.0.0.src.tar.xz
mv clang-tools-extra-11.0.0.src llvm/tools/clang/tools/extra
cd llvm
mkdir build && cd build
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j4
```
最后一步编译的`make -jn`需根据自身`CPU`核数而定，一般一两个小时可完成`llvm`的编译，如果编译过程中出现错误，在解决问题后重新编译时，会从出错位置继续编译，不用过于担心，主机内存较小时，可以使用`make -j1`缓慢编译

