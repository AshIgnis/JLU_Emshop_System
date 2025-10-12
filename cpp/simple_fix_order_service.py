#!/usr/bin/env python3
"""
简单修复OrderService.cpp:删除private成员,添加OrderService::前缀
"""

with open('services/OrderService.cpp', 'r', encoding='utf-8') as f:
    lines = f.readlines()

new_lines = []
i = 0
while i < len(lines):
    line = lines[i]
    
    # 跳过mutex成员变量
    if 'std::mutex order_mutex_' in line:
        i += 1
        continue
    
    # 添加OrderService::前缀
    # 匹配: const std::string& 方法名() const {
    if line.strip().startswith('const std::string&') and '() const {' in line:
        # 提取方法名
        import re
        match = re.search(r'const std::string&\s+(\w+)\s*\(', line)
        if match:
            method_name = match.group(1)
            line = line.replace(method_name + ' (', 'OrderService::' + method_name + ' (')
    
    # 匹配: std::string 方法名(...) {
    elif line.strip().startswith('std::string ') and ' {' in line and '::' not in line:
        import re
        match = re.search(r'std::string\s+(\w+)\s*\(', line)
        if match:
            method_name = match.group(1)
            line = line.replace(method_name + ' (', 'OrderService::' + method_name + ' (')
    
    # 匹配: json 方法名(...) {
    elif line.strip().startswith('json ') and ' {' in line and '::' not in line:
        import re
        match = re.search(r'json\s+(\w+)\s*\(', line)
        if match:
            method_name = match.group(1)
            line = line.replace(method_name + ' (', 'OrderService::' + method_name + ' (')
    
    # 匹配: bool 方法名(...) const {
    elif line.strip().startswith('bool ') and '() const {' in line and '::' not in line:
        import re
        match = re.search(r'bool\s+(\w+)\s*\(', line)
        if match:
            method_name = match.group(1)
            line = line.replace(method_name + ' (', 'OrderService::' + method_name + ' (')
    
    # 匹配: OrderService() : BaseService() 构造函数
    elif 'OrderService() : BaseService()' in line:
        line = line.replace('OrderService()', 'OrderService::OrderService()')
    
    # 匹配: std::string getServiceName() const override
    elif 'std::string getServiceName() const override' in line:
        line = line.replace('std::string getServiceName()', 'std::string OrderService::getServiceName()')
    
    new_lines.append(line)
    i += 1

# 写回文件
with open('services/OrderService.cpp', 'w', encoding='utf-8') as f:
    f.writelines(new_lines)

print(f"Fixed OrderService.cpp ({len(new_lines)} lines)")
print("Added OrderService:: prefix to methods")
print("Removed mutex member variable")
