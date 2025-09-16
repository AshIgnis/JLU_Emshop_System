# Emshop 系统架构与实现细节（供 AI / 开发者直接使用）

> 目的：把系统整体架构、接口契约、通信协议、运维及测试细节以结构化、可机读/可复制的形式列出，便于日后直接喂给助手或作为开发/测试规范。

## 使用说明（给助手与开发者）

- 优先级：阅读顺序按章节布局（概览 → 通信契约 → 客户端实现细节 → 服务端实现细节 → 运维/CI/测试）。
- 如果要喂给 AI：可直接复制本文档全文，或摘取 `REST API` / `WebSocket 协议` / `消息格式` 等小节；也可把 `"示例交互"` 里的 JSON 片段给助手作为输入样例。
- 假设：服务端主实现是 Java + Netty；当前 repo 路径为 `d:/codehome/jlu/JLU_Emshop_System`。

---

## 1. 总体概览

- 架构风格：C/S（桌面客户端为 Qt），服务端基于 Java Netty 提供实时通道与 REST 接口；Web 管理后台（可选）使用 Spring Boot + Vue。
- 主要模块：
  - 客户端（Qt）
  - 服务端实时通道（Netty WebSocket/WSS 或 TCP）
  - 服务端业务层（Java）
  - 数据库层（MySQL）
  - 缓存/消息总线（Redis / Kafka 可选）
  - 持久化/日志/监控（Prometheus / Grafana）

---

## 2. 设计契约与接口（可直接喂给程序或助手）

### 2.1 REST API 约定（通用包装）

- 所有响应格式：

```
{ "code": 200, "message": "OK", "data": { ... } }
```

- 鉴权：使用 HTTP Header `Authorization: Bearer <jwt>`。

### 2.2 最小必要 REST 列表（路径、方法、请求/响应示例）

- POST /api/auth/login
  - 请求：{ "username": "", "password": "" }
  - 响应：{ code:200, data:{ token:"<jwt>", user:{id,username,...} } }

- GET /api/products?page=0&size=20&category=...
  - 响应：{ code:200, data:{ items:[{id,name,price,stock,imageUrl}], total: N } }

- POST /api/cart
  - 请求：{ productId: 1, quantity: 2 }
  - 响应：{ code:200, data:{ cartId: ... } }

- GET /api/cart
- POST /api/checkout
  - 请求：{ cartId, addressId, couponCode }
  - 响应：{ code:200, data:{ orderId, paymentUrl? } }

- GET /api/orders/{id}

- POST /api/payments/notify （服务端回调接口，内部使用）

> 注：以上只列核心 CRUD，更多 admin/运营 API 请按相同风格补充。

---

## 3. WebSocket / WSS 协议（实时）

> 基本思想：REST 负责 CRUD 与慢交互，WebSocket 负责推送、实时更新和即时通信。

### 3.1 连接流程

1. 客户端打开 WSS 到 `wss://<host>/ws`。
2. 建连后立即发送鉴权包：

```
{ "type": "auth", "token": "<jwt>" }
```

1. 服务端验证 token，返回：

```
{ "type": "auth_result", "code": 200, "userId": 123 }
```

1. 通过鉴权后可订阅频道或接收推送。

### 3.2 消息 envelope（统一格式）

```
{
  "type": "message_type",    // e.g., product_update, order_status, chat
  "seq": 12345,              // 可选：用于幂等/确认
  "from": "server|client",
  "userId": 123,            // 可选：目标用户/来源
  "to": "userId|roomId|all",
  "body": { ... }           // 业务载荷
}
```

### 3.3 心跳与重连

- 客户端：每 20s 发送心跳包 `{ type:"ping" }`；服务端回复 `pong`。
- 服务端：使用 Netty IdleStateHandler，15s 无活动则标记超时并关闭连接。
- 断线重连：客户端使用指数退避（首次 1s, 2s, 4s, 最大 60s）。

### 3.4 会话管理（服务端）

- 在内存维护 Map<userId, Set<Channel>>（支持多端登录）。
- 提供 ChannelGroup 做广播。
- 多实例部署时使用 Redis pub/sub 将消息广播到其他实例再由实例推送到本机连接。

### 3.5 示例消息类型

- product_update：商品库存/价格变更
- order_status：订单状态更新
- chat_message：客服或系统消息
- presence：用户上下线通知

---

## 4. 客户端（Qt）实现细节

### 4.1 项目结构建议

```
EmshopQt/
├─ CMakeLists.txt
├─ src/
│  ├─ main.cpp
│  ├─ network/
│  │  ├─ NetworkManager.h/.cpp   // 封装 QNetworkAccessManager（REST）
│  │  └─ WSClient.h/.cpp         // 封装 QWebSocket（WSS）
│  ├─ ui/
│  │  ├─ LoginWindow.*
│  │  ├─ ProductList.*
│  │  └─ CartWindow.*
│  ├─ models/
│  │  ├─ ProductModel.*          // QAbstractListModel
│  └─ storage/
│     └─ LocalCache.*            // SQLite 或 QSettings
```

### 4.2 网络层（NetworkManager）要点

- 提供 async 请求接口：get/post/put/delete 返回 signal/slot 或 QFuture。
- 自动在 Header 中注入 `Authorization: Bearer <token>`。
- 统一错误处理：状态码映射、重试策略（针对幂等请求）。
- 支持文件上传 multipart/form-data。

### 4.3 WebSocket 客户端（WSClient）要点

- 连接建立后发送鉴权包并等待 `auth_result`。
- 实现心跳发送/重连策略。
- 提供 subscribe(topic) / send(message) / onMessage(callback)。
- 所有回调切换回主线程以便安全更新 UI（使用 `QMetaObject::invokeMethod` 或信号传递）。

### 4.4 本地存储/离线支持

- 缓存最近 N 页商品到 SQLite；在网络不可用时读取缓存显示。
- 本地保留未发送的操作（例如添加到购物车/未完成订单），重连后重放（需幂等 ID）。

---

## 5. 服务端（Java Netty）实现细节

### 5.1 模块划分

- net/  (Netty handlers)
  - WebSocketInitHandler
  - AuthHandler
  - HeartbeatHandler
  - BusinessDispatchHandler
- service/  (核心业务)
  - AuthService
  - ProductService
  - OrderService
  - PaymentService
- dao/ (MyBatis 或 JPA)
- cache/ (Redis 封装)
- util/ (JSON 序列化/反序列化)

### 5.2 Netty 要点

- 使用 `IdleStateHandler` 检测读写空闲并触发心跳检查。
- 使用 `AttributeKey` 在 Channel 上存储 userId 或 sessionId。
- 对消息处理使用线程池（业务执行）以避免阻塞 IO 线程。
- 将长耗时调用（例如第三方支付）异步化，并在完成后通过 WebSocket 推送或写 DB 状态。

### 5.3 多实例横向扩展

- 使用 Redis pub/sub 或 Kafka 做消息广播/事件总线：当某实例需要推送消息给某 userId 时，先发布到 Redis，所有实例订阅后判断本地是否有该连接并推送。
- 会话粘性可用作临时方案，但不推荐作为扩展方案。

---

## 6. 数据库与缓存策略

- 主 DB：MySQL，读写分离（必要时）。
- 事务：订单支付流程使用 DB 事务与幂等校验（orderId + tradeId）防止重复扣款。
- 缓存：Redis 保存会话 token、热点商品缓存、短期订单状态；并用于分布式锁（限库存场景）。
- 缓存失效策略：库存强一致性场景避免缓存过期失效导致超卖，优先 DB + 分布式锁/乐观锁。

---

## 7. 支付与第三方集成

- 支付流程：客户端发起 `POST /api/checkout` → 后端创建支付单并返回 `paymentUrl` → 客户端在 `QWebEngineView` 中打开 `paymentUrl` → 第三方回调到后端 `POST /api/payments/notify` → 后端更新订单并通过 WebSocket 推送客户端。
- 不建议直接在桌面端实现回调监听（需要公网）。
- 回调需做幂等处理（tradeNo 唯一校验）。

---

## 8. 日志、监控与告警

- 日志：JSON 格式日志，字段包括 timestamp, level, module, userId, traceId, message, extra。
- 日志上报：本地保存 + ELK/EFK（Filebeat -> Elasticsearch）或直接推送到集中日志系统。
- 监控：Prometheus 指标（连接数、消息 QPS、延迟、错误率），Grafana 仪表盘。
- 告警：基于 Prometheus Alertmanager（连接数骤降、错误率上升、延迟异常）。

---

## 9. CI / 构建 / 打包

- 服务端：Maven 构建，单元测试 + 集成测试（本地 MySQL/测试 DB）。
- 客户端：CMake + Qt（Kit 选择 MSVC 或 MinGW），构建后利用 NSIS 打包 Windows 安装器。
- 自动化：推荐 GitHub Actions / GitLab CI 实现矩阵构建（Windows/Linux）。

---

## 10. 测试策略（要点）

- 单元测试：业务逻辑（JUnit / Google Test）。
- 集成测试：REST 接口（使用 Postman + Newman 或 pytest + requests）。
- 压力测试：WebSocket 并发连接测试（使用 k6/gatling 或自写多线程 ws 客户端）。
- UI 自动化：Qt AutoTest / Squish 对关键流程做冒烟测试（登录→下单→支付模拟）。

---

## 11. 运维与恢复

- 数据备份：每日全库备份 + 每小时 binlog 归档。
- 灾备：跨机房异地备份；关键服务（Redis）做主从或哨兵集群。
- 回滚策略：发布流水线保存 artifact，支持回滚到上一个稳定版本。

---

## 12. 可供助手调用的样例片段（直接粘贴以便快速复用）

### 12.1 REST 登录示例（JSON）

```
POST /api/auth/login
{ "username": "alice", "password": "pwd123" }

RESPONSE:
{ "code":200, "message":"OK", "data":{ "token":"ey...", "user":{ "id":1, "username":"alice" } } }
```

### 12.2 WS 鉴权示例

```
CLIENT -> SERVER:
{ "type":"auth", "token":"ey..." }

SERVER -> CLIENT:
{ "type":"auth_result", "code":200, "userId":1 }
```

### 12.3 产品更新推送示例

```
{ "type":"product_update", "seq": 98765, "body": { "productId": 123, "stock": 5, "price": 99.9 } }
```

---

## 13. 业务要求清单（便于自动化/验收）

- 登录/鉴权（JWT）
- 商品列表/详情/搜索
- 购物车（本地 + 服务端同步）
- 结算 + 支付（后端中转）
- 订单管理 + 物流记录
- 优惠券/促销基础框架
- 实时推送（订单状态、库存变更、系统消息）

---

## 14. 供 AI 使用的额外输入格式建议（便于你后续喂我）

- 以 YAML 或 JSON 提供接口清单（推荐 OpenAPI）。例如 OpenAPI/Swagger 文件 `openapi.yaml`。
- 提供最小可运行样例（1) 一个可登录账号，用于测试；2) 一个测试商品）。
- 提供待对接模块的源路径（例如 `java/src/main/java/emshop/...`），并说明入口类。

---

## 15. 假设与限制（写明以免误解）

- 假设当前后端可增加 WebSocket/REST 端点（或已有）。
- 假设桌面客户端优先为 Windows，后续可扩展到 Linux/macOS。
- 本文档不包含完整 DB 字段设计，建议将现有 SQL（`cpp/emshop_database_init.sql`）与本设计对齐。

---

## 16. 后续我可以帮你生成的工件（可选）

- 完整的 `EmshopQt` 最小项目骨架（CMake + main + NetworkManager + WSClient + 登录界面示例）。
- Netty WSS 最小服务端示例（鉴权 + Redis pub/sub 钩子）。
- OpenAPI 草案文件 `openapi.yaml`（包含上面 REST 接口）。


---

最后：如需我立即把某部分（比如 `EmshopQt` 骨架或 Netty WSS 服务端示例）生成到仓库，请回复要生成的目标（例如：`生成 Qt 骨架` 或 `生成 Netty WSS`）。
