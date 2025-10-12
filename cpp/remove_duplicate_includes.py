#!/usr/bin/env python3
"""
删除emshop_native_impl_oop.cpp中的重复include
"""

with open('emshop_native_impl_oop.cpp', 'r', encoding='utf-8') as f:
    lines = f.readlines()

print(f"Original: {len(lines)} lines")

# 找到第二个"服务模块Include区域"的位置
second_include_start = -1
count = 0
for i, line in enumerate(lines):
    if '服务模块Include区域' in line:
        count += 1
        if count == 2:
            second_include_start = i - 1  # 前一行的空行
            break

if second_include_start > 0:
    # 找到重复include的结束位置(到下一个空行或注释)
    second_include_end = second_include_start
    for i in range(second_include_start, min(second_include_start + 30, len(lines))):
        if lines[i].strip() == '' and i > second_include_start + 15:
            second_include_end = i
            break
        second_include_end = i
    
    print(f"Found duplicate includes at lines {second_include_start+1}-{second_include_end+1}")
    
    # 删除重复部分
    new_lines = lines[:second_include_start] + lines[second_include_end+1:]
    
    with open('emshop_native_impl_oop.cpp', 'w', encoding='utf-8') as f:
        f.writelines(new_lines)
    
    print(f"New: {len(new_lines)} lines")
    print(f"Removed: {len(lines) - len(new_lines)} lines")
else:
    print("No duplicate includes found")
