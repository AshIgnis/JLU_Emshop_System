#!/usr/bin/env python3
import re

with open('services/OrderService.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# 移除mutex成员
content = re.sub(r'\s+std::mutex order_mutex_;', '', content)

# 添加OrderService::前缀到方法定义
# 1. const std::string& getXXX() const {
content = re.sub(r'\n(\s*)const std::string& (\w+)\s*\((.*?)\)\s*const\s*\{', 
                 r'\n\1const std::string& OrderService::\2(\3) const {', content)

# 2. std::string getXXX(...) { 或 std::string generateXXX() {
content = re.sub(r'\n(\s*)std::string (\w+)\s*\((.*?)\)\s*\{', 
                 r'\n\1std::string OrderService::\2(\3) {', content)

# 3. json methodName(...) {
content = re.sub(r'\n(\s*)json (\w+)\s*\((.*?)\)\s*\{', 
                 r'\n\1json OrderService::\2(\3) {', content)

# 4. bool methodName(...) const {
content = re.sub(r'\n(\s*)bool (\w+)\s*\((.*?)\)\s*const\s*\{', 
                 r'\n\1bool OrderService::\2(\3) const {', content)

# 5. 构造函数 OrderService() : BaseService()
content = re.sub(r'(\s+)OrderService\(\)\s*:\s*BaseService\(\)', 
                 r'\1OrderService::OrderService() : BaseService()', content)

# 写回
with open('services/OrderService.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("OrderService.cpp fixed!")
print("- Removed mutex member")
print("- Added OrderService:: prefix to all methods")
