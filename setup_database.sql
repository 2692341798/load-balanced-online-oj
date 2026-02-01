-- Database initialization script for Online Judge System

-- Create database
CREATE DATABASE IF NOT EXISTS oj CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE oj;

-- Create user and grant privileges
-- Note: Check if user exists before creating to avoid errors in some MySQL versions if using strict mode, 
-- but CREATE USER IF NOT EXISTS is supported in MySQL 5.7+
CREATE USER IF NOT EXISTS 'oj_client'@'localhost' IDENTIFIED BY '123456';
GRANT ALL PRIVILEGES ON oj.* TO 'oj_client'@'localhost';
FLUSH PRIVILEGES;

-- Table: oj_questions
-- Stores problem information
CREATE TABLE IF NOT EXISTS `oj_questions` (
  `number` INT PRIMARY KEY AUTO_INCREMENT COMMENT '题目编号，唯一标识',
  `title` VARCHAR(255) NOT NULL COMMENT '题目标题',
  `star` VARCHAR(50) NOT NULL COMMENT '题目难度等级（简单/中等/困难）',
  `cpu_limit` INT NOT NULL DEFAULT 1 COMMENT 'CPU时间限制（秒）',
  `mem_limit` INT NOT NULL DEFAULT 30000 COMMENT '内存限制（KB）',
  `description` TEXT NOT NULL COMMENT '题目描述，支持Markdown格式',
  `header` TEXT NOT NULL COMMENT '[已废弃] 题目预设代码头',
  `tail` TEXT NOT NULL COMMENT 'JSON格式的测试用例',
  `status` INT DEFAULT 1 COMMENT '0:Hidden, 1:Visible',
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  INDEX `idx_star` (`star`),
  INDEX `idx_title` (`title`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='题目表';

-- Table: users
-- Stores user information
CREATE TABLE IF NOT EXISTS `users` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '用户ID，唯一标识',
  `username` VARCHAR(50) NOT NULL UNIQUE COMMENT '用户名，唯一',
  `password` VARCHAR(128) NOT NULL COMMENT '密码（SHA256哈希）',
  `email` VARCHAR(100) DEFAULT NULL COMMENT '用户邮箱地址',
  `nickname` VARCHAR(100) DEFAULT NULL COMMENT '用户昵称',
  `phone` VARCHAR(20) DEFAULT NULL COMMENT '用户手机号',
  `role` INT DEFAULT 0 COMMENT '0:User, 1:Admin',
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  INDEX `idx_email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- Table: submissions
-- Stores submission records
CREATE TABLE IF NOT EXISTS `submissions` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '提交ID，唯一标识',
  `user_id` INT NOT NULL COMMENT '用户ID，关联users表',
  `question_id` INT NOT NULL COMMENT '题目ID，关联oj_questions表',
  `language` VARCHAR(20) NOT NULL DEFAULT 'cpp' COMMENT '编程语言 (cpp, java, python)',
  `result` VARCHAR(10) NOT NULL COMMENT '评测结果状态码（0为通过）',
  `cpu_time` INT DEFAULT 0 COMMENT '运行耗时（毫秒）',
  `mem_usage` INT DEFAULT 0 COMMENT '内存使用（KB）',
  `content` TEXT COMMENT '提交的代码内容',
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '提交时间',
  INDEX `idx_user_id` (`user_id`),
  INDEX `idx_question_id` (`question_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='提交记录表';

-- Insert admin user (optional, strictly speaking not part of schema but useful)
-- Password is 'admin' hashed with SHA256 (example, please change in production)
-- echo -n "admin" | shasum -a 256
-- 8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918
-- Table: discussions
-- Stores discussion posts
CREATE TABLE IF NOT EXISTS `discussions` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '帖子ID',
  `title` VARCHAR(255) NOT NULL COMMENT '标题',
  `content` TEXT NOT NULL COMMENT '内容(Markdown)',
  `author_id` INT NOT NULL COMMENT '作者ID',
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `likes` INT DEFAULT 0 COMMENT '点赞数',
  `views` INT DEFAULT 0 COMMENT '浏览量',
  `is_official` TINYINT(1) DEFAULT 0 COMMENT '是否官方帖',
  INDEX `idx_author_id` (`author_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='讨论区帖子表';

-- Table: article_comments
-- Stores global comments for articles/posts
CREATE TABLE IF NOT EXISTS `article_comments` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '评论ID',
  `post_id` INT NOT NULL COMMENT '帖子ID',
  `user_id` INT NOT NULL COMMENT '用户ID',
  `content` TEXT NOT NULL COMMENT '评论内容',
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `likes` INT DEFAULT 0 COMMENT '点赞数',
  INDEX `idx_post_id` (`post_id`),
  INDEX `idx_user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='文章全局评论表';

-- Table: inline_comments
-- Stores inline comments for specific text segments
CREATE TABLE IF NOT EXISTS `inline_comments` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '内联评论ID',
  `user_id` INT NOT NULL COMMENT '用户ID',
  `post_id` INT NOT NULL COMMENT '帖子ID',
  `content` TEXT NOT NULL COMMENT '评论内容',
  `selected_text` TEXT COMMENT '选中的文本片段',
  `parent_id` INT DEFAULT NULL COMMENT '父评论ID (用于回复)',
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  INDEX `idx_post_id` (`post_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='段内评论表';

-- Table: contests
-- Stores contest list data fetched by crawler
CREATE TABLE IF NOT EXISTS `contests` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `contest_id` varchar(50) NOT NULL COMMENT 'Platform ID',
  `name` varchar(255) NOT NULL,
  `start_time` datetime NOT NULL,
  `end_time` datetime NOT NULL,
  `link` varchar(255) NOT NULL,
  `source` varchar(50) DEFAULT 'Codeforces',
  `status` varchar(20) DEFAULT 'upcoming',
  `last_crawl_time` datetime DEFAULT NULL,
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_source_id` (`source`, `contest_id`),
  INDEX `idx_status_time` (`status`, `start_time` DESC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO users (username, password, email, nickname, role) 
SELECT 'admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918', 'admin@example.com', 'Administrator', 1 
WHERE NOT EXISTS (SELECT * FROM users WHERE username = 'admin');
