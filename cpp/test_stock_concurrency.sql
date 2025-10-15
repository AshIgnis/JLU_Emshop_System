-- ============================================
-- 库存并发控制测试脚本
-- 测试多用户同时购买最后一件商品的情况
-- ============================================

USE emshop;

-- 1. 创建一个测试商品（库存仅1件）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, is_featured, created_at)
VALUES ('限量版测试商品', '用于测试并发购买，库存仅1件', 1, '测试品牌', 9999.00, 1, '/images/test.jpg', 'active', 1, NOW());

SET @test_product_id = LAST_INSERT_ID();

-- 2. 查看该商品信息
SELECT '=== 测试商品信息 ===' as info;
SELECT product_id, name, stock_quantity, version, status FROM products WHERE product_id = @test_product_id;

-- 3. 测试存储过程：尝试扣减库存
SELECT '=== 测试1：正常扣减库存（应该成功） ===' as info;
CALL sp_decrease_stock_optimistic(@test_product_id, 1, 1, 0, @result, @message);
SELECT @result as result, @message as message;

-- 查看扣减后的库存
SELECT product_id, name, stock_quantity, version, status FROM products WHERE product_id = @test_product_id;

-- 4. 测试存储过程：再次尝试扣减（应该失败 - 库存为0）
SELECT '=== 测试2：尝试再次扣减（应该失败：库存为0） ===' as info;
CALL sp_decrease_stock_optimistic(@test_product_id, 1, 2, 0, @result2, @message2);
SELECT @result2 as result, @message2 as message;

-- 5. 测试恢复库存
SELECT '=== 测试3：恢复库存（退款） ===' as info;
CALL sp_restore_stock(@test_product_id, 1, 1, 0, '测试退款', @result3, @message3);
SELECT @result3 as result, @message3 as message;

-- 查看恢复后的库存
SELECT product_id, name, stock_quantity, version, status FROM products WHERE product_id = @test_product_id;

-- 6. 查看库存变动日志
SELECT '=== 库存变动日志 ===' as info;
SELECT 
    log_id,
    product_id,
    user_id,
    change_type,
    quantity_before,
    quantity_change,
    quantity_after,
    reason,
    created_at
FROM stock_change_logs
WHERE product_id = @test_product_id
ORDER BY created_at DESC;

-- 7. 测试并发场景模拟（使用不同的版本号）
SELECT '=== 测试4：模拟并发冲突（两个用户同时扣减） ===' as info;

-- 首先确保库存为1
UPDATE products SET stock_quantity = 1, version = 0 WHERE product_id = @test_product_id;

-- 用户A扣减（应该成功）
CALL sp_decrease_stock_optimistic(@test_product_id, 1, 10, 1001, @resultA, @messageA);
SELECT @resultA as '用户A结果', @messageA as '用户A消息';

-- 用户B尝试扣减（应该失败 - 库存已为0）
CALL sp_decrease_stock_optimistic(@test_product_id, 1, 11, 1002, @resultB, @messageB);
SELECT @resultB as '用户B结果', @messageB as '用户B消息';

-- 8. 查看最终状态
SELECT '=== 最终商品状态 ===' as info;
SELECT product_id, name, stock_quantity, version, status, sold_quantity FROM products WHERE product_id = @test_product_id;

-- 9. 查看所有库存变动记录
SELECT '=== 所有库存变动记录 ===' as info;
SELECT * FROM stock_change_logs WHERE product_id = @test_product_id ORDER BY created_at;

-- 10. 清理测试数据（可选）
-- DELETE FROM stock_change_logs WHERE product_id = @test_product_id;
-- DELETE FROM products WHERE product_id = @test_product_id;

SELECT '✅ 并发控制测试完成！' as result;
