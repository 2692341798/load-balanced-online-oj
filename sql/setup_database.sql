-- 创建数据库
CREATE DATABASE IF NOT EXISTS oj CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE oj;

-- 题目表
CREATE TABLE IF NOT EXISTS oj_questions (
    number INT PRIMARY KEY AUTO_INCREMENT,
    title VARCHAR(255) NOT NULL,
    star VARCHAR(50) NOT NULL,
    cpu_limit INT NOT NULL DEFAULT 1,
    mem_limit INT NOT NULL DEFAULT 30000,
    description TEXT NOT NULL,
    header TEXT DEFAULT NULL,
    tail_code TEXT NOT NULL,
    status INT DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_star (star),
    INDEX idx_title (title)
);

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(128) NOT NULL,
    email VARCHAR(100) DEFAULT NULL,
    nickname VARCHAR(100) DEFAULT NULL,
    phone VARCHAR(20) DEFAULT NULL,
    role INT DEFAULT 0 COMMENT '0:User, 1:Admin',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_email (email)
);

-- 提交记录表
CREATE TABLE IF NOT EXISTS submissions (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    question_id INT NOT NULL,
    language VARCHAR(20) DEFAULT 'cpp',
    result VARCHAR(10) NOT NULL,
    cpu_time INT DEFAULT 0,
    mem_usage INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    content TEXT,
    INDEX idx_user_id (user_id),
    INDEX idx_question_id (question_id)
);

-- 讨论表
CREATE TABLE IF NOT EXISTS discussions (
    id INT PRIMARY KEY AUTO_INCREMENT,
    title VARCHAR(255) NOT NULL,
    content TEXT NOT NULL,
    author_id INT NOT NULL,
    question_id INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    likes INT DEFAULT 0,
    views INT DEFAULT 0,
    is_official TINYINT DEFAULT 0,
    INDEX idx_author_id (author_id),
    INDEX idx_question_id (question_id)
);

-- 内联评论表
CREATE TABLE IF NOT EXISTS inline_comments (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    post_id INT NOT NULL,
    content TEXT NOT NULL,
    selected_text TEXT,
    parent_id INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_post_id (post_id)
);

-- 文章评论表
CREATE TABLE IF NOT EXISTS article_comments (
    id INT PRIMARY KEY AUTO_INCREMENT,
    post_id INT NOT NULL,
    user_id INT NOT NULL,
    content TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    likes INT DEFAULT 0,
    INDEX idx_post_id (post_id),
    INDEX idx_user_id (user_id)
);

-- 竞赛表
CREATE TABLE IF NOT EXISTS contests (
    id INT PRIMARY KEY AUTO_INCREMENT,
    contest_id VARCHAR(50) NOT NULL COMMENT 'Platform ID',
    name VARCHAR(255) NOT NULL,
    start_time DATETIME NOT NULL,
    end_time DATETIME NOT NULL,
    link VARCHAR(255) NOT NULL,
    source VARCHAR(50) DEFAULT 'Codeforces',
    status VARCHAR(20) DEFAULT 'upcoming',
    last_crawl_time DATETIME DEFAULT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY idx_source_id (source, contest_id),
    INDEX idx_status_time (status, start_time DESC)
);
