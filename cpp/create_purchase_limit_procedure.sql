-- 创建限购检查存储过程
USE emshop;

DELIMITER //

DROP PROCEDURE IF EXISTS check_user_purchase_limit//
CREATE PROCEDURE check_user_purchase_limit(
    IN p_user_id BIGINT,
    IN p_product_id BIGINT,
    IN p_quantity INT,
    OUT can_purchase BOOLEAN,
    OUT purchased_count INT,
    OUT limit_count INT,
    OUT limit_period VARCHAR(20)
)
BEGIN
    DECLARE v_purchase_limit INT;
    DECLARE v_purchase_limit_period VARCHAR(20);
    DECLARE v_purchased_count INT DEFAULT 0;
    DECLARE v_start_time TIMESTAMP;

    -- 获取商品限购配置
    SELECT purchase_limit, purchase_limit_period
    INTO v_purchase_limit, v_purchase_limit_period
    FROM products
    WHERE product_id = p_product_id;

    -- 如果不限购，直接返回可以购买
    IF v_purchase_limit = 0 OR v_purchase_limit IS NULL THEN
        SET can_purchase = TRUE;
        SET purchased_count = 0;
        SET limit_count = 0;
        SET limit_period = 'unlimited';
    ELSE

    -- 根据限购周期计算起始时间
    CASE v_purchase_limit_period
        WHEN 'daily' THEN
            SET v_start_time = DATE_FORMAT(NOW(), '%Y-%m-%d 00:00:00');
        WHEN 'weekly' THEN
            SET v_start_time = DATE_SUB(NOW(), INTERVAL WEEKDAY(NOW()) DAY);
            SET v_start_time = DATE_FORMAT(v_start_time, '%Y-%m-%d 00:00:00');
        WHEN 'monthly' THEN
            SET v_start_time = DATE_FORMAT(NOW(), '%Y-%m-01 00:00:00');
        ELSE -- 'total'
            SET v_start_time = '1970-01-01 00:00:00';
    END CASE;

    -- 统计用户在指定周期内的购买数量
    SELECT COALESCE(SUM(quantity), 0)
    INTO v_purchased_count
    FROM user_purchase_records
    WHERE user_id = p_user_id
        AND product_id = p_product_id
        AND purchase_time >= v_start_time
        AND status = 'valid';

    -- 判断是否可以购买
    IF v_purchased_count + p_quantity <= v_purchase_limit THEN
        SET can_purchase = TRUE;
    ELSE
        SET can_purchase = FALSE;
    END IF;

    -- 返回信息
    SET purchased_count = v_purchased_count;
    SET limit_count = v_purchase_limit;
    SET limit_period = v_purchase_limit_period;

    END IF;
END//

DELIMITER ;

SELECT '存储过程 check_user_purchase_limit 创建成功' AS message;