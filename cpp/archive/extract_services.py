import re

# Read the file
with open('emshop_native_impl_oop.cpp', 'r', encoding='utf-8') as f:
    lines = f.readlines()

print(f"Original file: {len(lines)} lines")

# Define services to extract with their approximate line ranges
# We'll find exact boundaries by searching for class definitions
services_to_extract = [
    ('UserService', 1160, 1730),
    ('ProductService', 1731, 2439),
    ('CartService', 2440, 2736),
    ('AddressService', 2737, 3180),
    ('CouponService', 4153, 4505),
    ('ReviewService', 4507, 4755),
    ('OrderService', 1190, 2400)  # 在新的行号位置
]

# Find BaseService definition end (before service includes)
include_insert_line = None
for i, line in enumerate(lines):
    if 'std::unordered_map<std::string, bool>> BaseService::column_exists_cache_' in line:
        # Insert includes after this line
        include_insert_line = i + 1
        break

if include_insert_line is None:
    print("Error: Could not find BaseService definition end")
    exit(1)

print(f"Will insert includes at line {include_insert_line + 1}")

# Remove services from the file (in reverse order to maintain line numbers)
for service_name, start, end in reversed(services_to_extract):
    # Find exact start (class definition)
    actual_start = None
    for i in range(max(0, start - 50), min(len(lines), start + 50)):
        if f'class {service_name} : public BaseService' in lines[i]:
            actual_start = i
            break
    
    if actual_start is None:
        print(f"Warning: Could not find {service_name} class definition near line {start}")
        continue
    
    # Find exact end (closing brace + semicolon)
    actual_end = None
    for i in range(actual_start, min(len(lines), end + 100)):
        if lines[i].strip() == '};' and i > actual_start + 10:
            # Check if this is likely the class closing brace
            # (not a small struct/enum inside)
            # Look for next non-empty line
            next_line_idx = i + 1
            while next_line_idx < len(lines) and lines[next_line_idx].strip() == '':
                next_line_idx += 1
            
            if next_line_idx < len(lines):
                next_line = lines[next_line_idx].strip()
                # If next line starts with //, class, or is empty section separator,
                # this is likely the class end
                if (next_line.startswith('//') or 
                    next_line.startswith('class ') or
                    '=====' in next_line):
                    actual_end = i + 1  # Include the }; line
                    break
    
    if actual_end is None:
        print(f"Warning: Could not find {service_name} class end")
        continue
    
    print(f"Removing {service_name}: lines {actual_start + 1}-{actual_end} ({actual_end - actual_start} lines)")
    
    # Remove the lines
    del lines[actual_start:actual_end]
    
    # Adjust include_insert_line if needed
    if include_insert_line > actual_start:
        include_insert_line -= (actual_end - actual_start)

# Insert include statements
include_lines = [
    '\n',
    '// ====================================================================\n',
    '// 服务模块Include区域\n',
    '// BaseService定义完成后,才能include服务类的实现\n',
    '// ====================================================================\n',
    '#include "services/UserService.h"\n',
    '#include "services/UserService.cpp"\n',
    '#include "services/ProductService.h"\n',
    '#include "services/ProductService.cpp"\n',
    '#include "services/CartService.h"\n',
    '#include "services/CartService.cpp"\n',
    '#include "services/AddressService.h"\n',
    '#include "services/AddressService.cpp"\n',
    '#include "services/CouponService.h"\n',
    '#include "services/CouponService.cpp"\n',
    '#include "services/ReviewService.h"\n',
    '#include "services/ReviewService.cpp"\n',
    '#include "services/OrderService.h"\n',
    '#include "services/OrderService.cpp"\n',
]

lines[include_insert_line:include_insert_line] = include_lines

print(f"Inserted {len(include_lines)} include lines at line {include_insert_line + 1}")
print(f"New file: {len(lines)} lines")
print(f"Reduction: {9629 - len(lines)} lines")

# Write the file
with open('emshop_native_impl_oop.cpp', 'w', encoding='utf-8') as f:
    f.writelines(lines)

print("Done!")
