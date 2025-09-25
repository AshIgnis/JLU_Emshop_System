Emshop Netty Console Client (遗留)

说明

- 项目同时包含基于 Qt 的图形化客户端（qt_client/），以及一个基于 Netty 的控制台客户端 `EmshopNettyClient`。
- `EmshopNettyClient` 保留用于：快速功能/连通性测试、示例/教学、或在没有图形环境时的备选客户端。

建议

- 如果你已经使用 Qt 客户端作为主力客户端，可将 `EmshopNettyClient` 标记为弃用（已由代码注解），并将其移动到 `examples/` 或 `legacy/` 目录以减少主代码树噪声。
- 若决定完全移除，建议先在代码库中添加替代说明并保证 Qt 客户端覆盖相同测试场景再删除。

如何使用

- 在 `java/` 目录下运行（使用 Maven exec）：

```bat
cd java
mvn exec:java -Dexec.mainClass="emshop.EmshopNettyClient" -Dexec.args="localhost 8080"
```

- 或使用 `run.bat` 脚本选择“3. 启动客户端”，脚本现在会优先提示使用 Qt 客户端并提供一个选项来运行遗留客户端示例。

维护者备注

- 已把示例源码与文档移动到 `java/examples/legacy/`。如果需要，我可以：
  - 把该示例打包成独立的示例模块；
  - 或完全删除示例并在 README 中写明替代方案。
