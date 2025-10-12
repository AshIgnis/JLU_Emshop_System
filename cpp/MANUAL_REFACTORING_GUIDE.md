# C++ 模块化手动提取指南

> 创建时间: 2025年10月12日  
> 适用于: CartService 及后续所有服务模块的提取

---

## 📌 当前状态

### 文件信息
- **主文件**: `cpp/emshop_native_impl_oop.cpp` (9902行)
- **当前任务**: 提取 CartService (第3个服务模块)
- **已完成**: UserService, ProductService (已有模块文件但未集成)

### CartService 位置信息
- **开始行**: 2455 (注释: `// 购物车服务类`)
- **结束行**: 2750 (CartService类结束的`};`)
- **总行数**: 296行
- **下一个类**: AddressService (从2751行开始)

---

## ✅ 我已经为您准备好的文件

### 1. CartService.h
- **路径**: `cpp/services/CartService.h`
- **状态**: ✅ 已创建完成 (100行)
- **内容**: 完整的类声明、方法签名、文档注释

### 2. CartService.cpp
- **路径**: `cpp/services/CartService.cpp`  
- **状态**: ✅ 已创建完成 (300行)
- **内容**: 6个方法的完整实现
  - addToCart
  - getCart
  - removeFromCart
  - updateCartItemQuantity
  - updateCartSelected
  - clearCart

---

## 🎯 您需要手动完成的操作

### 步骤1: 添加 include 语句

**位置**: 在文件开头,查找其他service的include位置

**查找方法**:
1. 打开 `cpp/emshop_native_impl_oop.cpp`
2. 按 `Ctrl+F` 搜索: `#include "services/`
3. 如果找不到,在 `#include "emshop_EmshopNativeInterface.h"` 这行后面

**添加内容** (在合适位置添加这两行):
```cpp
#include "services/CartService.h"
#include "services/CartService.cpp"
```

**注意事项**:
- 保持与其他include的缩进一致
- 如果已有UserService/ProductService的include,放在它们下面
- 确保在所有系统头文件(如 `<iostream>`)之后

---

### 步骤2: 删除 CartService 类定义

**重要提示**: 请在删除前**先备份文件**!

**删除范围**:
- **开始**: 第 2455 行 (包含注释 `// 购物车服务类`)
- **结束**: 第 2750 行 (CartService类的结束 `};` 以及后面的空行)

**精确标记**:

**删除开始处** (第2455行开始):
```cpp
// 购物车服务类
class CartService : public BaseService {
private:
    std::mutex cart_mutex_;
```

**删除结束处** (第2750行附近):
```cpp
        return result;
    }
};

// 用户地址服务类
```

**保留**: `// 用户地址服务类` 这一行及以后的内容

**删除方法**:

**方法A: 使用VSCode选择删除**
1. 点击第2455行的行号
2. 按住Shift,滚动到第2750行,点击行号
3. 按Delete键
4. 检查 `// 用户地址服务类` 是否在删除范围的下一行

**方法B: 使用行号跳转**
1. 按 `Ctrl+G`,输入 `2455` 跳转
2. 按 `Ctrl+Shift+K` 删除当前行
3. 重复步骤2共296次 (或使用宏录制)

**方法C: 使用搜索替换** (最精确)
1. 按 `Ctrl+H` 打开替换
2. 启用正则表达式模式 (点击 `.*` 按钮)
3. 查找: `// 购物车服务类\nclass CartService.*?\n\};(\n)*(?=// 用户地址服务类)`
4. 替换为: (空,什么都不填)
5. 点击"全部替换"

---

### 步骤3: 验证文件完整性

**删除后检查清单**:

1. **检查include是否添加**
   ```cpp
   // 应该能找到这两行
   #include "services/CartService.h"
   #include "services/CartService.cpp"
   ```

2. **检查CartService是否完全删除**
   - 按 `Ctrl+F` 搜索 `class CartService`
   - 应该找不到任何匹配项

3. **检查AddressService是否完整**
   - 跳转到原来2751行位置
   - 应该看到:
   ```cpp
   // 用户地址服务类
   class AddressService : public BaseService {
   private:
       std::mutex address_mutex_;
   ```

4. **检查文件行数**
   - 原始: 9902行
   - 预期结果: 约9608行 (9902 - 296 + 2 = 9608)
   - 在VSCode右下角可以看到当前行数

---

### 步骤4: 保存并通知我

**保存文件**: `Ctrl+S`

**然后在聊天中告诉我**:
```
已完成CartService提取:
- include已添加: ✅
- 类定义已删除: ✅  
- 文件行数: XXXX行 (填入实际行数)
- AddressService位置正常: ✅
```

---

## 🔧 我接下来会做什么

收到您的确认后,我会:

1. **编译验证**
   ```bash
   cd cpp
   .\build_oop_jni.bat
   ```
   - 检查是否有编译错误
   - 查看DLL文件大小

2. **运行测试**
   ```bash
   cd java
   mvn test -Dtest=ErrorCodeTest
   ```
   - 验证13/13测试通过
   - 确认功能正常

3. **更新文档**
   - TODO.md (标记CartService完成)
   - 创建 WORK_SUMMARY_2025-10-12_CartService.md
   - 记录代码行数变化

4. **提交到Git** (建议)
   ```bash
   git add cpp/services/CartService.*
   git add cpp/emshop_native_impl_oop.cpp
   git commit -m "feat: 提取CartService模块 (手动方式)"
   ```

5. **继续下一个服务**
   - 为您准备 AddressService 的提取指南
   - 或其他您指定的服务

---

## ❓ 常见问题

### Q1: 删除后编译报错怎么办?
**A**: 不用担心,我会帮您修复:
1. 把错误信息完整复制给我
2. 我会分析问题并提供精确的修复方案

### Q2: 不确定删除范围是否正确?
**A**: 可以先不删除,把那部分代码复制给我确认:
1. 复制第2455-2465行 (开头10行)
2. 复制第2745-2755行 (结尾10行)
3. 我会告诉您是否正确

### Q3: 想先测试一下是否可行?
**A**: 很好的想法!可以:
1. 先复制一份文件作为备份
2. 在备份上练习删除操作
3. 确认无误后再在正式文件操作

### Q4: 能否提供自动化脚本?
**A**: 可以!但手动更安全。如果您熟悉脚本:
```python
# Python脚本示例 (需要Python 3.6+)
with open('emshop_native_impl_oop.cpp', 'r', encoding='utf-8') as f:
    lines = f.readlines()

# 删除2455-2750行 (注意Python索引从0开始)
new_lines = lines[:2454] + lines[2750:]

with open('emshop_native_impl_oop.cpp', 'w', encoding='utf-8') as f:
    f.writelines(new_lines)
```

---

## 📊 进度追踪

| 服务模块 | 模块文件 | include添加 | 类定义删除 | 编译测试 | 状态 |
|---------|---------|------------|-----------|---------|------|
| UserService | ✅ | ❌ | ❌ | ❌ | 待手动提取 |
| ProductService | ✅ | ❌ | ❌ | ❌ | 待手动提取 |  
| **CartService** | ✅ | 👈您操作 | 👈您操作 | 我验证 | **当前任务** |
| AddressService | ⏳ | - | - | - | 下一个 |
| OrderService | ⏳ | - | - | - | 排队中 |

---

## 💡 温馨提示

1. **不要紧张** - 这个操作很安全,有备份可恢复
2. **仔细确认** - 删除前务必检查行号
3. **保持沟通** - 任何疑问随时问我
4. **分步进行** - 可以先添加include,确认编译通过,再删除类定义

---

准备好了就开始吧! 您可以随时问我任何问题。🚀
