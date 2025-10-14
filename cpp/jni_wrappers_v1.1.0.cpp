// 新增 JNI 包装函数 - v1.1.0 业务逻辑改进
// 这些函数应该追加到 emshop_native_impl_oop.cpp 文件末尾

// 1. 审批退款申请
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_approveRefund
  (JNIEnv *env, jclass clazz, jlong refundId, jlong adminId, jboolean approve, jstring adminReply) {
    try {
        const char* reply_chars = env->GetStringUTFChars(adminReply, nullptr);
        std::string reply_str(reply_chars);
        env->ReleaseStringUTFChars(adminReply, reply_chars);
        
        OrderService* orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService->approveRefund(refundId, adminId, approve == JNI_TRUE, reply_str);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "审批退款申请失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 2. 获取退款申请列表
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getRefundRequests
  (JNIEnv *env, jclass clazz, jstring status, jint page, jint pageSize) {
    try {
        const char* status_chars = env->GetStringUTFChars(status, nullptr);
        std::string status_str(status_chars);
        env->ReleaseStringUTFChars(status, status_chars);
        
        OrderService* orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService->getRefundRequests(status_str, page, pageSize);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取退款申请列表失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 3. 获取用户退款申请列表
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getUserRefundRequests
  (JNIEnv *env, jclass clazz, jlong userId) {
    try {
        OrderService* orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService->getUserRefundRequests(userId);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取用户退款申请列表失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 4. 获取用户通知列表
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getNotifications
  (JNIEnv *env, jclass clazz, jlong userId, jboolean unreadOnly) {
    try {
        OrderService* orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService->getNotifications(userId, unreadOnly == JNI_TRUE);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取通知列表失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 5. 标记通知为已读
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_markNotificationRead
  (JNIEnv *env, jclass clazz, jlong notificationId, jlong userId) {
    try {
        OrderService* orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService->markNotificationRead(notificationId, userId);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "标记通知失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 6. 获取订单可用优惠券
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getAvailableCouponsForOrder
  (JNIEnv *env, jclass clazz, jlong userId, jdouble orderAmount) {
    try {
        CouponService* couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService->getAvailableCouponsForOrder(userId, orderAmount);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取可用优惠券失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 7. 计算优惠券折扣
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_calculateCouponDiscount
  (JNIEnv *env, jclass clazz, jstring couponCode, jdouble orderAmount) {
    try {
        const char* code_chars = env->GetStringUTFChars(couponCode, nullptr);
        std::string code_str(code_chars);
        env->ReleaseStringUTFChars(couponCode, code_chars);
        
        CouponService* couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService->calculateCouponDiscount(code_str, orderAmount);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "计算优惠券折扣失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 8. 创建优惠券活动
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_createCouponActivity
  (JNIEnv *env, jclass clazz, jstring name, jstring code, jstring type, 
   jdouble value, jdouble minAmount, jint quantity, jstring startDate, jstring endDate, jlong templateId) {
    try {
        const char* name_chars = env->GetStringUTFChars(name, nullptr);
        std::string name_str(name_chars);
        env->ReleaseStringUTFChars(name, name_chars);
        
        const char* code_chars = env->GetStringUTFChars(code, nullptr);
        std::string code_str(code_chars);
        env->ReleaseStringUTFChars(code, code_chars);
        
        const char* type_chars = env->GetStringUTFChars(type, nullptr);
        std::string type_str(type_chars);
        env->ReleaseStringUTFChars(type, type_chars);
        
        const char* start_chars = env->GetStringUTFChars(startDate, nullptr);
        std::string start_str(start_chars);
        env->ReleaseStringUTFChars(startDate, start_chars);
        
        const char* end_chars = env->GetStringUTFChars(endDate, nullptr);
        std::string end_str(end_chars);
        env->ReleaseStringUTFChars(endDate, end_chars);
        
        CouponService* couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService->createCouponActivity(
            name_str, code_str, type_str, value, minAmount, quantity, start_str, end_str, templateId);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "创建优惠券活动失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 9. 获取优惠券模板列表
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getCouponTemplates
  (JNIEnv *env, jclass clazz) {
    try {
        CouponService* couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService->getCouponTemplates();
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取优惠券模板失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 10. 分发优惠券给用户
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_distributeCouponsToUsers
  (JNIEnv *env, jclass clazz, jstring couponCode, jstring userIdsJson) {
    try {
        const char* code_chars = env->GetStringUTFChars(couponCode, nullptr);
        std::string code_str(code_chars);
        env->ReleaseStringUTFChars(couponCode, code_chars);
        
        const char* users_chars = env->GetStringUTFChars(userIdsJson, nullptr);
        std::string users_str(users_chars);
        env->ReleaseStringUTFChars(userIdsJson, users_chars);
        
        CouponService* couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService->distributeCouponsToUsers(code_str, users_str);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "分发优惠券失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 11. 申请退款 (更新版本 - 包含 user_id)
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_requestRefund__JJLjava_lang_String_2
  (JNIEnv *env, jclass clazz, jlong orderId, jlong userId, jstring reason) {
    try {
        const char* reason_chars = env->GetStringUTFChars(reason, nullptr);
        std::string reason_str(reason_chars);
        env->ReleaseStringUTFChars(reason, reason_chars);
        
        OrderService* orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService->requestRefund(orderId, userId, reason_str);
        
        return JNIStringConverter::jsonToJstring(env, result);
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "申请退款失败: " + std::string(e.what());
        error_response["error_code"] = Constants::ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}
