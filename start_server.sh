#!/bin/bash

# JLU Emshop 系统启动脚本
echo "=== JLU Emshop 电商系统启动 ==="
echo "作者: JLU 55240425 屈熙宸"
echo "系统: Java 21 + WebSocket + JNI + MySQL"
echo

cd "d:/codehome/jlu/JLU_Emshop_System/java"

echo "1. 编译Java项目..."
mvn compile -q
if [ $? -eq 0 ]; then
    echo "✅ 编译成功"
else
    echo "❌ 编译失败"
    exit 1
fi

echo
echo "2. 启动WebSocket服务器..."
echo "服务器地址: ws://localhost:8082/ws"
echo "按 Ctrl+C 停止服务器"
echo

java -cp "target/classes:dependency/*" emshop.EmshopWebSocketServer