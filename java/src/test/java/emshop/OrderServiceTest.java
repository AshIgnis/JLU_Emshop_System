package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.*;
import static org.junit.jupiter.api.Assertions.*;

import java.io.IOException;

/**
 * OrderService关键流程集成测试
 * 测试订单创建、支付、取消、退款等核心业务流程
 * 
 * @author JLU Emshop Team
 * @date 2025-10-13
 */
@TestMethodOrder(MethodOrderer.OrderAnnotation.class)
public class OrderServiceTest {

    private static EmshopNativeInterface nativeInterface;
    private static ObjectMapper objectMapper;
    private static long testUserId;
    private static long testProductId;
    private static long testAddressId;
    private static long testOrderId;

    @BeforeAll
    static void setUp() {
        System.out.println("=".repeat(60));
        System.out.println("OrderService 关键流程测试");
        System.out.println("=".repeat(60));
        
        // 加载JNI库
        try {
            System.loadLibrary("emshop_native_oop");
            System.out.println("✓ JNI库加载成功");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("❌ JNI库加载失败: " + e.getMessage());
            System.err.println("请确保 emshop_native_oop.dll 在库路径中");
            throw e;
        }

        nativeInterface = new EmshopNativeInterface();
        objectMapper = new ObjectMapper();
        
        // 准备测试数据
        setupTestData();
    }

    /**
     * 准备测试数据：创建测试用户、商品、地址
     */
    private static void setupTestData() {
        try {
            // 1. 创建测试用户
            String registerResult = nativeInterface.register(
                "test_order_user_" + System.currentTimeMillis(),
                "Test123456",
                "13800138000"
            );
            JsonNode registerNode = objectMapper.readTree(registerResult);
            testUserId = registerNode.get("user_id").asLong();
            System.out.println("✓ 测试用户创建成功 (ID: " + testUserId + ")");

            // 2. 创建测试商品（需要管理员权限，这里简化为直接使用已有商品ID）
            // 实际测试中应该先查询可用商品
            testProductId = 1; // 假设数据库中存在ID为1的商品
            System.out.println("✓ 使用测试商品 (ID: " + testProductId + ")");

            // 3. 创建测试地址
            String addressResult = nativeInterface.addUserAddress(
                testUserId,
                "张三",
                "13800138000",
                "吉林省",
                "长春市",
                "朝阳区",
                "前进大街2699号",
                "130012",
                true
            );
            JsonNode addressNode = objectMapper.readTree(addressResult);
            testAddressId = addressNode.get("address_id").asLong();
            System.out.println("✓ 测试地址创建成功 (ID: " + testAddressId + ")");

        } catch (Exception e) {
            System.err.println("❌ 测试数据准备失败: " + e.getMessage());
            e.printStackTrace();
        }
    }

    @Test
    @Order(1)
    @DisplayName("1. 订单创建流程测试")
    void testOrderCreation() throws IOException {
        System.out.println("\n--- 测试：订单创建 ---");

        // 从购物车创建订单（先添加商品到购物车）
        String addToCartResult = nativeInterface.addToCart(testUserId, testProductId, 2);
        JsonNode cartNode = objectMapper.readTree(addToCartResult);
        assertEquals(0, cartNode.get("error_code").asInt(), "添加到购物车应该成功");

        // 创建订单
        String createOrderResult = nativeInterface.createOrderFromCart(
            testUserId,
            testAddressId,
            "",  // 不使用优惠券
            "测试订单"
        );

        JsonNode orderNode = objectMapper.readTree(createOrderResult);
        System.out.println("订单创建结果: " + createOrderResult);

        assertEquals(0, orderNode.get("error_code").asInt(), "订单创建应该成功");
        assertTrue(orderNode.has("order_id"), "应该返回订单ID");
        assertTrue(orderNode.has("order_no"), "应该返回订单号");
        assertTrue(orderNode.has("total_amount"), "应该返回订单总金额");

        testOrderId = orderNode.get("order_id").asLong();
        System.out.println("✓ 订单创建成功 (ID: " + testOrderId + ", 订单号: " + orderNode.get("order_no").asText() + ")");
    }

    @Test
    @Order(2)
    @DisplayName("2. 订单查询测试")
    void testOrderQuery() throws IOException {
        System.out.println("\n--- 测试：订单查询 ---");

        // 获取订单详情
        String orderDetailResult = nativeInterface.getOrderDetail(testOrderId);
        JsonNode detailNode = objectMapper.readTree(orderDetailResult);
        
        System.out.println("订单详情: " + orderDetailResult);
        
        assertEquals(0, detailNode.get("error_code").asInt(), "获取订单详情应该成功");
        assertEquals("pending", detailNode.get("status").asText(), "新创建的订单状态应该是pending");
        assertTrue(detailNode.has("items"), "订单详情应该包含商品明细");

        // 获取用户订单列表
        String userOrdersResult = nativeInterface.getOrderList(testUserId);
        JsonNode ordersNode = objectMapper.readTree(userOrdersResult);
        
        assertEquals(0, ordersNode.get("error_code").asInt(), "获取用户订单列表应该成功");
        assertTrue(ordersNode.has("orders"), "应该返回订单列表");
        assertTrue(ordersNode.get("orders").isArray(), "订单列表应该是数组");
        assertTrue(ordersNode.get("orders").size() > 0, "应该至少有一个订单");

        System.out.println("✓ 订单查询成功");
    }

    @Test
    @Order(3)
    @DisplayName("3. 订单支付流程测试")
    void testOrderPayment() throws IOException {
        System.out.println("\n--- 测试：订单支付 ---");

        // 支付订单
        String paymentResult = nativeInterface.payOrder(testOrderId, "alipay");
        JsonNode paymentNode = objectMapper.readTree(paymentResult);
        
        System.out.println("支付结果: " + paymentResult);
        
        assertEquals(0, paymentNode.get("error_code").asInt(), "订单支付应该成功");
        assertTrue(paymentNode.has("transaction_id"), "应该返回交易ID");
        
        // 验证订单状态变更为paid
        String orderDetailResult = nativeInterface.getOrderDetail(testOrderId);
        JsonNode detailNode = objectMapper.readTree(orderDetailResult);
        assertEquals("paid", detailNode.get("status").asText(), "支付后订单状态应该是paid");

        System.out.println("✓ 订单支付成功 (交易ID: " + paymentNode.get("transaction_id").asText() + ")");
    }

    @Test
    @Order(4)
    @DisplayName("4. 订单发货流程测试")
    void testOrderShipping() throws IOException {
        System.out.println("\n--- 测试：订单发货 ---");

        // 发货
        String shipResult = nativeInterface.shipOrder(
            testOrderId,
            "SF123456789",
            "顺丰速运"
        );
        JsonNode shipNode = objectMapper.readTree(shipResult);
        
        System.out.println("发货结果: " + shipResult);
        
        assertEquals(0, shipNode.get("error_code").asInt(), "订单发货应该成功");
        
        // 验证订单状态变更为shipped
        String orderDetailResult = nativeInterface.getOrderDetail(testOrderId);
        JsonNode detailNode = objectMapper.readTree(orderDetailResult);
        assertEquals("shipped", detailNode.get("status").asText(), "发货后订单状态应该是shipped");
        assertEquals("SF123456789", detailNode.get("tracking_number").asText(), "快递单号应该正确");

        System.out.println("✓ 订单发货成功");
    }

    @Test
    @Order(5)
    @DisplayName("5. 订单确认收货测试")
    void testOrderConfirmDelivery() throws IOException {
        System.out.println("\n--- 测试：确认收货 ---");

        // 确认收货
        String confirmResult = nativeInterface.confirmDelivery(testOrderId);
        JsonNode confirmNode = objectMapper.readTree(confirmResult);
        
        System.out.println("确认收货结果: " + confirmResult);
        
        assertEquals(0, confirmNode.get("error_code").asInt(), "确认收货应该成功");
        
        // 验证订单状态变更为completed
        String orderDetailResult = nativeInterface.getOrderDetail(testOrderId);
        JsonNode detailNode = objectMapper.readTree(orderDetailResult);
        assertEquals("completed", detailNode.get("status").asText(), "确认收货后订单状态应该是completed");

        System.out.println("✓ 确认收货成功");
    }

    @Test
    @Order(6)
    @DisplayName("6. 订单取消流程测试（新订单）")
    void testOrderCancellation() throws IOException {
        System.out.println("\n--- 测试：订单取消 ---");

        // 创建一个新订单用于测试取消
        nativeInterface.addToCart(testUserId, testProductId, 1);
        String createResult = nativeInterface.createOrderFromCart(
            testUserId,
            testAddressId,
            "",
            "取消测试订单"
        );
        JsonNode createNode = objectMapper.readTree(createResult);
        long cancelOrderId = createNode.get("order_id").asLong();

        // 取消订单
        String cancelResult = nativeInterface.cancelOrder(testUserId, cancelOrderId);
        JsonNode cancelNode = objectMapper.readTree(cancelResult);
        
        System.out.println("取消订单结果: " + cancelResult);
        
        assertEquals(0, cancelNode.get("error_code").asInt(), "订单取消应该成功");
        
        // 验证订单状态变更为cancelled
        String orderDetailResult = nativeInterface.getOrderDetail(cancelOrderId);
        JsonNode detailNode = objectMapper.readTree(orderDetailResult);
        assertEquals("cancelled", detailNode.get("status").asText(), "取消后订单状态应该是cancelled");

        System.out.println("✓ 订单取消成功");
    }

    @Test
    @Order(7)
    @DisplayName("7. 订单退款流程测试")
    void testOrderRefund() throws IOException {
        System.out.println("\n--- 测试：订单退款 ---");

        // 创建并支付一个新订单用于测试退款
        nativeInterface.addToCart(testUserId, testProductId, 1);
        String createResult = nativeInterface.createOrderFromCart(
            testUserId,
            testAddressId,
            "",
            "退款测试订单"
        );
        JsonNode createNode = objectMapper.readTree(createResult);
        long refundOrderId = createNode.get("order_id").asLong();

        // 支付订单
        nativeInterface.payOrder(refundOrderId, "wechat");

        // 申请退款 - 使用新的方法签名 (orderId, userId, reason)
        String refundResult = nativeInterface.requestRefund(refundOrderId, testUserId, "商品不满意");
        JsonNode refundNode = objectMapper.readTree(refundResult);
        
        System.out.println("退款申请结果: " + refundResult);
        
        assertEquals(0, refundNode.get("error_code").asInt(), "退款申请应该成功");
        
        // 验证订单状态变更为refunding或refunded
        String orderDetailResult = nativeInterface.getOrderDetail(refundOrderId);
        JsonNode detailNode = objectMapper.readTree(orderDetailResult);
        String status = detailNode.get("status").asText();
        assertTrue(status.equals("refunding") || status.equals("refunded"), 
                  "退款后订单状态应该是refunding或refunded");

        System.out.println("✓ 订单退款申请成功");
    }

    @Test
    @Order(8)
    @DisplayName("8. 订单状态转换验证测试")
    void testInvalidStatusTransition() throws IOException {
        System.out.println("\n--- 测试：非法状态转换验证 ---");

        // 创建新订单
        nativeInterface.addToCart(testUserId, testProductId, 1);
        String createResult = nativeInterface.createOrderFromCart(
            testUserId,
            testAddressId,
            "",
            "状态测试订单"
        );
        JsonNode createNode = objectMapper.readTree(createResult);
        long statusTestOrderId = createNode.get("order_id").asLong();

        // 尝试在未支付的情况下发货（应该失败）
        String shipResult = nativeInterface.shipOrder(statusTestOrderId, "TEST123", "测试");
        JsonNode shipNode = objectMapper.readTree(shipResult);
        
        System.out.println("非法发货尝试结果: " + shipResult);
        
        assertNotEquals(0, shipNode.get("error_code").asInt(), "未支付订单不应该允许发货");

        System.out.println("✓ 非法状态转换正确拒绝");
    }

    @Test
    @Order(9)
    @DisplayName("9. 库存一致性测试")
    void testStockConsistency() throws IOException {
        System.out.println("\n--- 测试：库存一致性 ---");

        // 获取商品当前库存
        String productDetailBefore = nativeInterface.getProductDetail(testProductId);
        JsonNode productBefore = objectMapper.readTree(productDetailBefore);
        int stockBefore = productBefore.get("stock").asInt();

        // 创建订单（会扣减库存）
        nativeInterface.addToCart(testUserId, testProductId, 3);
        String createResult = nativeInterface.createOrderFromCart(
            testUserId,
            testAddressId,
            "",
            "库存测试订单"
        );
        JsonNode createNode = objectMapper.readTree(createResult);
        long stockTestOrderId = createNode.get("order_id").asLong();

        // 获取商品扣减后库存
        String productDetailAfter = nativeInterface.getProductDetail(testProductId);
        JsonNode productAfter = objectMapper.readTree(productDetailAfter);
        int stockAfter = productAfter.get("stock").asInt();

        // 验证库存正确扣减
        assertEquals(stockBefore - 3, stockAfter, "创建订单后库存应该减少3");

        // 取消订单（应该返还库存）
        nativeInterface.cancelOrder(testUserId, stockTestOrderId);

        // 验证库存返还
        String productDetailRestored = nativeInterface.getProductDetail(testProductId);
        JsonNode productRestored = objectMapper.readTree(productDetailRestored);
        int stockRestored = productRestored.get("stock").asInt();

        assertEquals(stockBefore, stockRestored, "取消订单后库存应该恢复");

        System.out.println("✓ 库存一致性验证通过");
    }

    @AfterAll
    static void tearDown() {
        System.out.println("\n" + "=".repeat(60));
        System.out.println("OrderService 测试完成");
        System.out.println("=".repeat(60));
    }
}
