-- ====================================================================
-- 创建通知表
-- 日期：2025-10-15
-- ====================================================================

USE emshop;

-- 创建通知表
DROP TABLE IF EXISTS notifications;
CREATE TABLE notifications (
    notification_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '通知ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    type ENUM('system', 'order_status', 'refund', 'promotion', 'message') DEFAULT 'system' COMMENT '通知类型',
    title VARCHAR(200) NOT NULL COMMENT '通知标题',
    content TEXT COMMENT '通知内容',
    related_id BIGINT DEFAULT 0 COMMENT '关联ID（如订单ID、退款ID）',
    status ENUM('unread', 'read', 'deleted') DEFAULT 'unread' COMMENT '通知状态',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    read_at TIMESTAMP NULL COMMENT '阅读时间',
    
    INDEX idx_user_id (user_id),
    INDEX idx_type (type),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at),
    INDEX idx_user_status (user_id, status),
    
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='用户通知表';

SELECT 'Notifications table created successfully!' AS message;
