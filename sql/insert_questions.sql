USE oj;

INSERT INTO oj_questions (number, title, star, cpu_limit, mem_limit, description, header, tail_code, status) VALUES
(1, '两数之和', '简单', 1, 30000,
 '给定一个整数数组 nums 和一个整数目标值 target，请你在该数组中找出和为目标值 target  的那 两个 整数，并返回它们的数组下标。\n\n你可以假设每种输入只会对应一个答案。但是，数组中同一个元素在答案里不能重复出现。\n\n你可以按任意顺序返回答案。\n\n示例 1：\n输入：nums = [2,7,11,15], target = 9\n输出：[0,1]\n解释：因为 nums[0] + nums[1] == 9 ，返回 [0, 1] 。',
 '',
 '[{\"stdin\":\"2 7 11 15\\n9\\n\",\"expected\":\"0 1\\n\"}]',
 1);

INSERT INTO users (username, password, email, nickname, phone) VALUES
('testuser', 'a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3', 'test@example.com', '测试用户', '13800138000');
