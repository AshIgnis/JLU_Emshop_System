# 文件恢复说明

## 问题分析

您复制了 `emshop_native_impl_oop_backup.cpp` 恢复文件,但这个备份包含了**未提取**的UserService和ProductService类定义。

**当前状态:**
- ✅ UserService模块文件已创建: `services/UserService.h` 和 `services/UserService.cpp`
- ✅ ProductService模块文件已创建: `services/ProductService.h` 和 `services/ProductService.cpp`  
- ✅ CartService模块文件已创建: `services/CartService.h` 和 `services/CartService.cpp`
- ✅ CartService类已从主文件删除(您已手动完成)
- ❌ UserService类仍在主文件中(行1176-1751,约575行)
- ❌ ProductService类仍在主文件中(行1752-2461,约709行)

## 需要执行的操作

### 操作1: 删除UserService类定义

**位置:** 行1176-1751 (共575行)

**开始标记** (第1176行):
```cpp
// 用户服务类
```

**结束标记** (第1751行):
```cpp
};
```

**下一行应该是** (第1752行):
```cpp
// 商品服务类
```

**删除方法:**
1. 按 `Ctrl+G` 跳转到第1176行
2. 向下滚动确认是 `// 用户服务类`
3. 按住 `Shift`,向下滚动到第1751行的 `};` 
4. 松开 `Shift`,按 `Delete` 键
5. 验证第1176行现在应该是 `// 商品服务类`

**预期结果:** 文件从9616行减少到9041行 (9616 - 575 = 9041)

---

### 操作2: 删除ProductService类定义

**位置:** 删除UserService后,ProductService将从行1176开始

**新位置:** 行1176-1885 (共709行)

**开始标记**:
```cpp
// 商品服务类
```

**结束标记:**
```cpp
};
```

**下一行应该是**:
```cpp
// ====================================================================
// 用户地址服务类
```

**删除方法:**
1. 按 `Ctrl+G` 跳转到第1176行(UserService删除后的新位置)
2. 确认是 `// 商品服务类`
3. 按住 `Shift`,向下滚动到 `};` (ProductService结束)
4. 确认下一行是 `// 用户地址服务类`
5. 松开 `Shift`,按 `Delete` 键

**预期结果:** 文件从9041行减少到8332行 (9041 - 709 = 8332)

---

## 验证清单

完成删除后,请验证以下几点:

### 1. 文件行数检查
```powershell
(Get-Content emshop_native_impl_oop.cpp).Length
```
应该显示约 **8332行**

### 2. 类定义检查
```powershell
Select-String "^class UserService" emshop_native_impl_oop.cpp
Select-String "^class ProductService" emshop_native_impl_oop.cpp
Select-String "^class CartService" emshop_native_impl_oop.cpp
```
**都应该返回空结果**(没有匹配)

### 3. Include语句检查
```powershell
Select-String '#include "services/' emshop_native_impl_oop.cpp
```
应该显示6个include:
- `#include "services/UserService.h"`
- `#include "services/UserService.cpp"`
- `#include "services/ProductService.h"`
- `#include "services/ProductService.cpp"`
- `#include "services/CartService.h"`
- `#include "services/CartService.cpp"`

### 4. 文件结构检查
查看行1176附近,应该直接是:
```cpp
// ====================================================================
// 用户地址服务类
// ====================================================================
class AddressService : public BaseService {
```

## 快速操作方案

如果您想快速完成,可以使用PowerShell脚本自动删除:

```powershell
# 读取文件
$lines = Get-Content emshop_native_impl_oop.cpp

# 删除UserService (行1176-1751, 数组索引1175-1750)
$part1 = $lines[0..1174]
$part2 = $lines[1751..($lines.Length-1)]
$lines = $part1 + $part2

# 删除ProductService (现在在行1176-1885, 数组索引1175-1884)  
$part1 = $lines[0..1174]
$part2 = $lines[1885..($lines.Length-1)]
$lines = $part1 + $part2

# 保存文件
$lines | Set-Content emshop_native_impl_oop_fixed.cpp -Encoding UTF8

# 验证
Write-Host "新文件行数: $(($lines | Measure-Object -Line).Lines)"
Write-Host "检查UserService: $(Select-String 'class UserService' emshop_native_impl_oop_fixed.cpp)"
Write-Host "检查ProductService: $(Select-String 'class ProductService' emshop_native_impl_oop_fixed.cpp)"
```

然后复制修复后的文件:
```powershell
Copy-Item emshop_native_impl_oop_fixed.cpp emshop_native_impl_oop.cpp -Force
```

## 完成后通知

删除完成后,请在聊天中回复:
```
已完成UserService和ProductService的删除,文件行数: XXXX行
```

然后我会进行编译和测试验证。
