import re

with open('oj_model.hpp', 'r') as f:
    content = f.read()

# Replace CheckAndUpgradeTable specific pattern
content = re.sub(
    r'MYSQL \*my = mysql_init\(nullptr\);\s+if\(nullptr == mysql_real_connect\(my, host\.c_str\(\), user\.c_str\(\), passwd\.c_str\(\), db\.c_str\(\), port, nullptr, 0\)\)\{\s+LOG\(ERROR\) << "Upgrade Check: Connect failed" << "\\n";\s+mysql_close\(my\);\s+return;\s+\}',
    r'ConnectionGuard guard;\n            MYSQL *my = guard.get();\n            if(!my){\n                LOG(ERROR) << "Upgrade Check: Connect failed" << "\\n";\n                return;\n            }',
    content
)

# Standard pattern with character set warning
pattern_1 = r'MYSQL \*my = mysql_init\(nullptr\);\s+if\(nullptr == mysql_real_connect\(my, host\.c_str\(\), user\.c_str\(\), passwd\.c_str\(\),db\.c_str\(\),port, nullptr, 0\)\)\{(.*?)\s+mysql_close\(my\);\s+return false;\s+\}\s+if\(0 != mysql_set_character_set\(my, "utf8mb4"\)\) \{ LOG\(WARNING\) << "mysql_set_character_set error: " << mysql_error\(my\) << "\\n"; \}'

def repl_1(m):
    err_log = m.group(1).strip()
    res = 'ConnectionGuard guard;\n            MYSQL *my = guard.get();\n            if(!my){\n                '
    if err_log:
        res += err_log + '\n                '
    res += 'return false;\n            }'
    return res

content = re.sub(pattern_1, repl_1, content, flags=re.DOTALL)

# Standard pattern without character set warning (e.g. GetTotalUserCount)
pattern_2 = r'MYSQL \*my = mysql_init\(nullptr\);\s+if\(nullptr == mysql_real_connect\(my, host\.c_str\(\), user\.c_str\(\), passwd\.c_str\(\),db\.c_str\(\),port, nullptr, 0\)\)\{(.*?)\s+mysql_close\(my\);\s+return false;\s+\}'

content = re.sub(pattern_2, repl_1, content, flags=re.DOTALL)

# Remove `mysql_close(my);` from everywhere EXCEPT `MySQLConnectionPool` methods
# We will just split the file by class MySQLConnectionPool, and only remove `mysql_close(my);` from `class Model`
parts = content.split('class Model')
if len(parts) == 2:
    model_part = parts[1]
    model_part = re.sub(r'^\s*mysql_close\(my\);\s*$', '', model_part, flags=re.MULTILINE)
    content = parts[0] + 'class Model' + model_part

with open('oj_model.hpp', 'w') as f:
    f.write(content)

print("Done")
