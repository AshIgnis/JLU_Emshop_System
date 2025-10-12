#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
正确地从emshop_native_impl_oop.cpp提取OrderService类并创建.cpp实现文件
"""

import re

def main():
    # 读取主文件
    with open('emshop_native_impl_oop.cpp', 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    print(f"主文件共 {len(lines)} 行")
    
    # OrderService类从1190行开始(0-indexed: 1189)
    # 找到类结束位置 "class EmshopServiceManager"之前
    order_service_start = 1189  # 0-indexed
    order_service_end = None
    
    for i in range(order_service_start + 100, len(lines)):
        if 'class EmshopServiceManager' in lines[i]:
            order_service_end = i - 1  # EmshopServiceManager前一行
            # 往回找到OrderService的};
            for j in range(i-1, order_service_start, -1):
                if lines[j].strip() == '};':
                    order_service_end = j
                    break
            break
    
    print(f"OrderService: 行 {order_service_start+1} 到 {order_service_end+1}")
    
    order_lines = lines[order_service_start:order_service_end+1]
    print(f"提取了 {len(order_lines)} 行")
    
    # 创建实现文件
    impl = []
    impl.append('/**\n')
    impl.append(' * @file OrderService.cpp\n')
    impl.append(' * @brief 订单服务类实现\n')
    impl.append(' * @date 2025-01-13\n')
    impl.append(' */\n')
    impl.append('\n')
    impl.append('#include "OrderService.h"\n')
    impl.append('\n')
    
    # 解析类内容,转换为实现
    # 跳过第一行 "class OrderService : public BaseService {"
    # 跳过最后一行 "};"
    # 处理private/public标记
    # 给方法加上OrderService::前缀
    
    in_private = False
    in_public = False
    brace_level = 0  # 追踪花括号嵌套层级
    
    for i, line in enumerate(order_lines):
        # 跳过第一行(类声明)
        if i == 0:
            continue
        # 跳过最后一行(};)
        if i == len(order_lines) - 1:
            continue
        
        stripped = line.strip()
        
        # 跳过private:/public:标记
        if stripped in ['private:', 'public:', 'protected:']:
            in_private = (stripped == 'private:')
            in_public = (stripped == 'public:')
            continue
        
        # 处理方法定义
        # 匹配: "    json methodName(" 或 "    std::string methodName("
        # 需要添加 OrderService:: 前缀
        method_match = re.match(r'^(\s+)(json|std::string|bool|void)(\s+)(\w+)(\s*\()', line)
        
        if method_match and brace_level == 0:
            indent, ret_type, space1, method_name, open_paren = method_match.groups()
            rest_of_line = line[method_match.end():]
            
            # 构造函数特殊处理
            if method_name == 'OrderService':
                line = ret_type + ' OrderService::' + method_name + open_paren + rest_of_line
            # const辅助方法需要保留const
            elif 'const' in rest_of_line and '{' in rest_of_line:
                # 简单方法,直接替换
                line = ret_type + ' OrderService::' + method_name + open_paren + rest_of_line
            else:
                # 普通方法
                line = ret_type + ' OrderService::' + method_name + open_paren + rest_of_line
        
        # 跟踪花括号
        brace_level += line.count('{') - line.count('}')
        
        impl.append(line)
    
    # 写入文件
    with open('services/OrderService.cpp', 'w', encoding='utf-8') as f:
        f.writelines(impl)
    
    print(f"已创建 OrderService.cpp,共 {len(impl)} 行")
    print("完成!")

if __name__ == '__main__':
    main()
