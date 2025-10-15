#!/usr/bin/env python3
"""
将Qt C++文件中的原始字符串字面量 R"(...)" 转换为传统字符串拼接
"""

import re
import sys
from pathlib import Path

def convert_raw_string(content):
    """
    转换 R"(...)" 为传统字符串格式
    """
    # 匹配 R"(...)" 模式（支持多行）
    pattern = r'R"\((.*?)\)"'
    
    def replacer(match):
        raw_content = match.group(1)
        lines = raw_content.split('\n')
        
        # 清理每行并添加引号
        formatted_lines = []
        for line in lines:
            stripped = line.strip()
            if stripped:
                # 转义双引号
                escaped = stripped.replace('"', '\\"')
                formatted_lines.append(f'"{escaped}"')
        
        # 用换行符连接
        if len(formatted_lines) > 1:
            return '\n        ' + '\n        '.join(formatted_lines)
        elif formatted_lines:
            return formatted_lines[0]
        else:
            return '""'
    
    # 执行替换
    result = re.sub(pattern, replacer, content, flags=re.DOTALL)
    return result

def process_file(filepath):
    """处理单个文件"""
    print(f"处理文件: {filepath}")
    
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    converted = convert_raw_string(content)
    
    if converted != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(converted)
        print(f"  ✅ 已转换")
        return True
    else:
        print(f"  ⏭️  无需转换")
        return False

def main():
    # 要处理的文件列表
    files = [
        "src/ui/LoginDialog.cpp",
        "src/ui/MainWindow.cpp",
        "src/ui/tabs/DashboardTab.cpp",
        "src/ui/tabs/ProductsTab.cpp",
    ]
    
    qtclient_dir = Path(__file__).parent.parent
    converted_count = 0
    
    for file_path in files:
        full_path = qtclient_dir / file_path
        if full_path.exists():
            if process_file(full_path):
                converted_count += 1
        else:
            print(f"⚠️  文件不存在: {full_path}")
    
    print(f"\n总结: 转换了 {converted_count} 个文件")

if __name__ == "__main__":
    main()
