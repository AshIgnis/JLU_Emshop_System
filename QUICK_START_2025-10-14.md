# 🚀 快速启动指南 - 硬编码修复版本

## 启动服务器

```powershell
cd d:\codehome\jlu\JLU_Emshop_System\java
mvn exec:java@server
```

## 启动Qt客户端

```powershell
cd d:\codehome\jlu\JLU_Emshop_System\qtclient\build\src
.\emshop_qtclient.exe
```

## 快速测试

### 1. 测试优惠券实时刷新
- 登录管理员账号
- 创建新优惠券
- 观察列表是否自动刷新 ✅

### 2. 测试购物车折扣
```
ADD_TO_CART 1 101 2
CALCULATE_CART_DISCOUNT 1 TEST2025
```

### 3. 测试系统状态
```
GET_SYSTEM_STATUS
```

## 关键改进

✅ 优惠券创建后立即显示  
✅ 所有数据从数据库读取  
✅ 支付方式动态配置  
✅ 会员等级根据真实积分  
✅ 系统状态显示真实信息  

## 文档

- **修复详情**: `HARDCODED_DATA_FIX_2025-10-14.md`
- **测试指南**: `HARDCODED_FIX_TEST_GUIDE_2025-10-14.md`
- **完成报告**: `HARDCODED_FIX_SUMMARY_2025-10-14.md`

---

**版本**: v1.1.0 - 无硬编码版  
**日期**: 2025-10-14
