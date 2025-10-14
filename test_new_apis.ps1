# API 测试脚本
# 用于测试新增的业务逻辑改进API

$baseUrl = "http://localhost:8080"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "业务逻辑改进 API 测试" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 测试1: 获取优惠券模板
Write-Host "[测试 1] 获取优惠券模板列表" -ForegroundColor Yellow
try {
    $response = Invoke-RestMethod -Uri "$baseUrl/api/coupon/templates" -Method Get
    Write-Host "✅ 成功" -ForegroundColor Green
    Write-Host ($response | ConvertTo-Json -Depth 3)
} catch {
    Write-Host "❌ 失败: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# 测试2: 计算优惠券折扣
Write-Host "[测试 2] 计算优惠券折扣" -ForegroundColor Yellow
try {
    $body = @{
        coupon_code = "NEWUSER"
        order_amount = 100.0
    } | ConvertTo-Json
    
    $response = Invoke-RestMethod -Uri "$baseUrl/api/coupon/calculate-discount" `
        -Method Post `
        -Body $body `
        -ContentType "application/json"
    
    Write-Host "✅ 成功" -ForegroundColor Green
    Write-Host ($response | ConvertTo-Json -Depth 3)
} catch {
    Write-Host "❌ 失败: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# 测试3: 获取订单可用优惠券
Write-Host "[测试 3] 获取订单可用优惠券" -ForegroundColor Yellow
try {
    $response = Invoke-RestMethod -Uri "$baseUrl/api/coupon/available-for-order?user_id=1&order_amount=200" -Method Get
    Write-Host "✅ 成功" -ForegroundColor Green
    Write-Host ($response | ConvertTo-Json -Depth 3)
} catch {
    Write-Host "❌ 失败: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# 测试4: 获取用户通知
Write-Host "[测试 4] 获取用户通知" -ForegroundColor Yellow
try {
    $response = Invoke-RestMethod -Uri "$baseUrl/api/notifications?user_id=1&unread_only=false" -Method Get
    Write-Host "✅ 成功" -ForegroundColor Green
    Write-Host ($response | ConvertTo-Json -Depth 3)
} catch {
    Write-Host "❌ 失败: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# 测试5: 获取退款申请列表
Write-Host "[测试 5] 获取退款申请列表(管理员)" -ForegroundColor Yellow
try {
    $response = Invoke-RestMethod -Uri "$baseUrl/api/refund/requests?status=all&page=1&page_size=10" -Method Get
    Write-Host "✅ 成功" -ForegroundColor Green
    Write-Host ($response | ConvertTo-Json -Depth 3)
} catch {
    Write-Host "❌ 失败: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# 测试6: 获取用户退款申请
Write-Host "[测试 6] 获取用户退款申请" -ForegroundColor Yellow
try {
    $response = Invoke-RestMethod -Uri "$baseUrl/api/refund/user/1" -Method Get
    Write-Host "✅ 成功" -ForegroundColor Green
    Write-Host ($response | ConvertTo-Json -Depth 3)
} catch {
    Write-Host "❌ 失败: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "测试完成" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
