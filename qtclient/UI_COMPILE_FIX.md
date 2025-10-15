# Qt客户端UI美化 - 快速应用脚本

由于原始字符串字面量（R"()"）在某些编译器版本中可能出现兼容性问题，我们提供两种解决方案：

## 方案1：使用传统字符串（推荐用于老版本编译器）

将所有样式字符串从：
```cpp
QString style = R"(
    QWidget {
        color: red;
    }
)";
```

改为：
```cpp
QString style = 
    "QWidget {"
    "  color: red;"
    "}";
```

## 方案2：确保C++11或更高标准

在CMakeLists.txt中添加：
```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## 当前状态

部分文件已使用方案1修复：
- ✅ LoginDialog.cpp (部分)

需要继续修复的文件：
- ⏳ LoginDialog.cpp (剩余部分)
- ⏳ MainWindow.cpp  
- ⏳ DashboardTab.cpp
- ⏳ ProductsTab.cpp

## 临时解决方案 - 禁用复杂样式

如果您希望快速编译，可以临时简化样式：

```cpp
// 简化版本 - 无渐变
m_loginButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border-radius: 8px; padding: 10px 20px; }");
```

## 编译命令

```powershell
cd d:\codehome\jlu\JLU_Emshop_System\qtclient\build
mingw32-make -j8
```
