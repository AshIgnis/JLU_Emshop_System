#!/bin/bash

echo "=========================================="
echo "JLU Emshop System - JNI 编译脚本 (Linux)"
echo "=========================================="

# 设置环境变量
export JAVA_HOME=${JAVA_HOME:-/usr/lib/jvm/java-21-openjdk-amd64}

# 检查必要的环境
echo "检查编译环境..."

if [ ! -d "$JAVA_HOME" ]; then
    echo "错误: 未找到 JAVA_HOME: $JAVA_HOME"
    echo "请确保安装了 JDK 21 并设置正确的 JAVA_HOME 环境变量"
    exit 1
fi

# 检查必要的库
echo "检查依赖库..."

pkg-config --exists mysqlclient
if [ $? -ne 0 ]; then
    echo "错误: 未找到 MySQL 客户端库"
    echo "请运行: sudo apt-get install libmysqlclient-dev"
    exit 1
fi

pkg-config --exists jsoncpp
if [ $? -ne 0 ]; then
    echo "错误: 未找到 JsonCpp 库"
    echo "请运行: sudo apt-get install libjsoncpp-dev"
    exit 1
fi

# 创建输出目录
mkdir -p bin

echo
echo "开始编译 JNI 库..."

# 编译命令
g++ -shared -fPIC \
    -I"$JAVA_HOME/include" \
    -I"$JAVA_HOME/include/linux" \
    $(pkg-config --cflags mysqlclient) \
    $(pkg-config --cflags jsoncpp) \
    $(pkg-config --libs mysqlclient) \
    $(pkg-config --libs jsoncpp) \
    -o bin/libemshop.so \
    emshop_native_impl.cpp

if [ $? -eq 0 ]; then
    echo
    echo "=========================================="
    echo "编译成功!"
    echo "输出文件: bin/libemshop.so"
    echo "=========================================="
    
    # 复制SO文件到Java项目的lib目录
    mkdir -p ../java/lib
    cp bin/libemshop.so ../java/lib/
    
    echo "SO文件已复制到Java项目目录"
    
    echo
    echo "现在可以运行Java项目测试JNI集成:"
    echo "cd ../java"
    echo "mvn exec:java@server -Dexec.args=\"8090\""
else
    echo
    echo "=========================================="
    echo "编译失败! 错误代码: $?"
    echo "=========================================="
    echo
    echo "可能的解决方案:"
    echo "1. 检查是否安装了 GCC 编译器"
    echo "2. 安装缺少的依赖库:"
    echo "   sudo apt-get update"
    echo "   sudo apt-get install build-essential"
    echo "   sudo apt-get install libmysqlclient-dev"
    echo "   sudo apt-get install libjsoncpp-dev"
    echo "   sudo apt-get install openjdk-21-jdk"
    echo "3. 确认 JAVA_HOME 环境变量设置正确"
fi

echo
