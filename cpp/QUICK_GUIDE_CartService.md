# CartService 手动提取 - 快速操作清单

## 📋 您需要做的两件事

### ✅ 任务1: 添加include (约1分钟)

**文件**: `cpp/emshop_native_impl_oop.cpp`

**操作**: 在文件开头附近添加这两行
```cpp
#include "services/CartService.h"
#include "services/CartService.cpp"
```

**位置建议**: 
- 搜索 `#include "emshop_EmshopNativeInterface.h"`
- 在这行下面添加上面两行
- 或者在所有 `#include` 的最后面

---

### ✅ 任务2: 删除CartService类 (约2分钟)

**文件**: `cpp/emshop_native_impl_oop.cpp`

**删除范围**: 
- **从第 2455 行开始** (包含 `// 购物车服务类`)
- **到第 2750 行结束** (CartService的 `};` 及空行)
- **总共删除**: 296行

**删除后应该看到**:
```cpp
// (前面是其他代码)

// 用户地址服务类
class AddressService : public BaseService {
    // (AddressService的代码继续)
```

**推荐删除方法**:
1. 按 `Ctrl+G` 跳转到第 2455 行
2. 选中从2455到2750行的所有内容
3. 按 `Delete` 删除
4. 确认下一行是 `// 用户地址服务类`

---

## ✅ 完成后告诉我

在聊天中回复:
```
已完成CartService手动提取
文件行数: XXXX行 (在VSCode右下角查看)
```

然后我会:
1. 编译测试 (./build_oop_jni.bat)
2. 运行测试 (mvn test)
3. 如果有问题,指导您修复
4. 成功后,继续下一个服务模块

---

## 🆘 需要帮助?

- **不确定删除范围?** → 把第2455-2465行和第2745-2755行的代码复制给我看
- **编译报错?** → 把完整错误信息发给我
- **想先练习?** → 可以先复制文件做备份,在备份上操作

准备好了就开始吧!我随时在线帮您。🚀
