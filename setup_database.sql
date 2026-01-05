-- 创建数据库
CREATE DATABASE IF NOT EXISTS oj CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建用户并授权
CREATE USER IF NOT EXISTS 'oj_client'@'localhost' IDENTIFIED BY '123456';
GRANT ALL PRIVILEGES ON oj.* TO 'oj_client'@'localhost';
FLUSH PRIVILEGES;

-- 使用数据库
USE oj;

-- 创建题目表
CREATE TABLE IF NOT EXISTS oj_questions (
    number INT PRIMARY KEY AUTO_INCREMENT,
    title VARCHAR(255) NOT NULL,
    star VARCHAR(50) NOT NULL,
    cpu_limit INT NOT NULL DEFAULT 1,
    mem_limit INT NOT NULL DEFAULT 30000,
    description TEXT NOT NULL,
    header_code TEXT NOT NULL,
    tail_code TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_star (star),
    INDEX idx_title (title)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 插入一些测试题目
INSERT INTO oj_questions (number, title, star, cpu_limit, mem_limit, description, header_code, tail_code) VALUES
(1, '两数之和', '简单', 1, 30000, '给定一个整数数组 nums 和一个整数目标值 target，请你在该数组中找出和为目标值 target 的那两个整数，并返回它们的数组下标。\n\n你可以假设每种输入只会对应一个答案。但是，数组中同一个元素在答案里不能重复出现。\n\n你可以按任意顺序返回答案。', '#include <iostream>\n#include <vector>\nusing namespace std;\n\nclass Solution {\npublic:\n    vector<int> twoSum(vector<int>& nums, int target) {\n        // 在这里编写你的代码\n        \n    }\n};\n\nint main() {\n    Solution solution;\n    int n, target;\n    cin >> n >> target;\n    vector<int> nums(n);\n    for (int i = 0; i < n; i++) {\n        cin >> nums[i];\n    }\n    vector<int> result = solution.twoSum(nums, target);\n    cout << result[0] << " " << result[1] << endl;\n    return 0;\n}', 'int main() {\n    // 测试用例1\n    vector<int> nums1 = {2, 7, 11, 15};\n    int target1 = 9;\n    vector<int> result1 = solution.twoSum(nums1, target1);\n    assert(result1.size() == 2);\n    assert((result1[0] == 0 && result1[1] == 1) || (result1[0] == 1 && result1[1] == 0));\n    \n    // 测试用例2\n    vector<int> nums2 = {3, 2, 4};\n    int target2 = 6;\n    vector<int> result2 = solution.twoSum(nums2, target2);\n    assert(result2.size() == 2);\n    assert((result2[0] == 1 && result2[1] == 2) || (result2[0] == 2 && result2[1] == 1));\n    \n    cout << "所有测试用例通过!" << endl;\n    return 0;\n}'),
(2, '回文数', '简单', 1, 30000, '给你一个整数 x ，如果 x 是一个回文整数，返回 true ；否则，返回 false 。\n\n回文数是指正序（从左向右）和倒序（从右向左）读都是一样的整数。\n\n例如，121 是回文，而 123 不是。', '#include <iostream>\nusing namespace std;\n\nclass Solution {\npublic:\n    bool isPalindrome(int x) {\n        // 在这里编写你的代码\n        \n    }\n};\n\nint main() {\n    Solution solution;\n    int x;\n    cin >> x;\n    bool result = solution.isPalindrome(x);\n    cout << (result ? "true" : "false") << endl;\n    return 0;\n}', 'int main() {\n    // 测试用例1\n    assert(solution.isPalindrome(121) == true);\n    \n    // 测试用例2\n    assert(solution.isPalindrome(-121) == false);\n    \n    // 测试用例3\n    assert(solution.isPalindrome(10) == false);\n    \n    cout << "所有测试用例通过!" << endl;\n    return 0;\n}'),
(3, '有效的括号', '简单', 1, 30000, '给定一个只包括 ''('', '')'', ''{'', ''}'', ''['', '']'' 的字符串 s ，判断字符串是否有效。\n\n有效字符串需满足：\n1. 左括号必须用相同类型的右括号闭合。\n2. 左括号必须以正确的顺序闭合。\n3. 每个右括号都有一个对应的相同类型的左括号。', '#include <iostream>\n#include <string>\n#include <stack>\nusing namespace std;\n\nclass Solution {\npublic:\n    bool isValid(string s) {\n        // 在这里编写你的代码\n        \n    }\n};\n\nint main() {\n    Solution solution;\n    string s;\n    cin >> s;\n    bool result = solution.isValid(s);\n    cout << (result ? "true" : "false") << endl;\n    return 0;\n}', 'int main() {\n    // 测试用例1\n    assert(solution.isValid("()") == true);\n    \n    // 测试用例2\n    assert(solution.isValid("()[]{}") == true);\n    \n    // 测试用例3\n    assert(solution.isValid("(]") == false);\n    \n    cout << "所有测试用例通过!" << endl;\n    return 0;\n}');