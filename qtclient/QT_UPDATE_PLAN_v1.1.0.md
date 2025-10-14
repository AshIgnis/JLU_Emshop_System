# Qt 客户端更新计划 - v1.1.0

## 概述
更新 Qt 客户端以支持 v1.1.0 新增的业务逻辑功能:退款流程、通知系统、优惠券增强。

## 需要更新的功能

### 1. 退款功能增强 (OrdersTab)
**现状**: 使用旧的 `REFUND_PAYMENT` 命令  
**目标**: 更新为新的 `REQUEST_REFUND` 命令

**更新内容**:
- ✅ 移除金额输入(服务器自动计算)
- ✅ 只需要退款原因
- ✅ 更新命令格式: `REQUEST_REFUND <orderId> <reason>`
- 🆕 添加"查看我的退款"功能 (`GET_USER_REFUND_REQUESTS`)
- 🆕 添加退款状态显示

### 2. 管理员退款审批 (新增 RefundManagementTab)
**功能**:
- 查看所有退款申请 (`GET_REFUND_REQUESTS`)
- 审批/拒绝退款 (`APPROVE_REFUND`)
- 按状态筛选(待处理/已批准/已拒绝)
- 分页显示

### 3. 通知中心 (新增 NotificationsTab)
**功能**:
- 查看所有通知 (`GET_NOTIFICATIONS`)
- 标记通知为已读 (`MARK_NOTIFICATION_READ`)
- 显示未读通知数量徽章
- 自动刷新(定时器)

### 4. 优惠券功能 (新增 CouponsTab)
**功能**:
- 查看我的优惠券
- 查看优惠券模板 (`GET_COUPON_TEMPLATES`)
- 下单时选择优惠券 (`GET_AVAILABLE_COUPONS_FOR_ORDER`)
- 显示折扣金额 (`CALCULATE_COUPON_DISCOUNT`)

### 5. 管理员优惠券管理 (新增 CouponManagementTab)
**功能**:
- 创建优惠券活动 (`CREATE_COUPON_ACTIVITY`)
- 分发优惠券给用户 (`DISTRIBUTE_COUPONS`)
- 查看优惠券使用统计

## 实施步骤

### Phase 1: 核心功能更新 (当前)
1. ✅ 更新 OrdersTab 的退款功能
2. 🆕 添加"我的退款"对话框
3. 🆕 创建 NotificationsTab 基础框架

### Phase 2: 管理员功能 (后续)
1. 创建 RefundManagementTab
2. 创建 CouponManagementTab
3. 添加权限检查

### Phase 3: 用户优惠券功能 (后续)
1. 创建 CouponsTab
2. 集成到下单流程
3. 添加优惠券选择器

## 技术细节

### 命令映射
| 功能 | 旧命令 | 新命令 |
|------|-------|--------|
| 申请退款 | `REFUND_PAYMENT <orderId> <amount> <reason>` | `REQUEST_REFUND <orderId> <reason>` |
| 查看退款 | (无) | `GET_USER_REFUND_REQUESTS` |
| 审批退款 | (无) | `APPROVE_REFUND <refundId> <approve> <reply>` |
| 查看通知 | (无) | `GET_NOTIFICATIONS <unreadOnly>` |
| 标记已读 | (无) | `MARK_NOTIFICATION_READ <notificationId>` |
| 优惠券模板 | (无) | `GET_COUPON_TEMPLATES` |
| 可用优惠券 | (无) | `GET_AVAILABLE_COUPONS_FOR_ORDER <orderAmount>` |
| 计算折扣 | (无) | `CALCULATE_COUPON_DISCOUNT <couponCode> <orderAmount>` |

### UI 更新
- OrdersTab: 更新退款按钮行为
- MainWindow: 添加通知图标(带未读徽章)
- CartTab: 添加优惠券选择器
- 新增 3 个 Tab (Notifications, Coupons, RefundManagement)

## 当前实施
Phase 1 - 更新 OrdersTab 退款功能
