# 在线评测系统数据库设计文档

## 1. 概述

本文档描述了负载均衡式在线评测系统的数据库设计方案。系统采用MySQL 8.0+作为主要数据库，支持题目管理、用户认证、代码评测、题单训练、社区讨论等核心功能。

## 2. 数据库环境配置

### 2.1 数据库连接信息
- **数据库类型**: MySQL 8.0+
- **主机地址**: 127.0.0.1 (Docker内部为 `db`)
- **端口号**: 3306
- **数据库名称**: oj
- **用户名**: oj_client
- **密码**: <db_password>
- **字符集**: utf8mb4
- **排序规则**: utf8mb4_unicode_ci

### 2.2 数据库初始化

```sql
-- 创建数据库
CREATE DATABASE IF NOT EXISTS oj CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建用户并授权
CREATE USER IF NOT EXISTS 'oj_client'@'%' IDENTIFIED BY '<db_password>';
GRANT ALL PRIVILEGES ON oj.* TO 'oj_client'@'%';
FLUSH PRIVILEGES;
```

## 3. 数据表设计

### 3.1 题目表 (oj_questions)

**表描述**: 存储在线评测系统的题目信息。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| number | INT | PRIMARY KEY, AUTO_INCREMENT | - | 题目编号 |
| title | VARCHAR(255) | NOT NULL | - | 题目标题 |
| star | VARCHAR(50) | NOT NULL | - | 难度等级 |
| cpu_limit | INT | NOT NULL | 1 | CPU时间限制(秒) |
| mem_limit | INT | NOT NULL | 30000 | 内存限制(KB) |
| description | TEXT | NOT NULL | - | 题目描述(Markdown) |
| header | TEXT | DEFAULT NULL | - | [已废弃] 预设代码 |
| tail | TEXT | NOT NULL | - | JSON格式的测试用例 |
| status | INT | DEFAULT 1 | 1 | 状态 (0:Hidden, 1:Visible) |

### 3.2 用户表 (users)

**表描述**: 存储系统用户信息。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 用户ID |
| username | VARCHAR(50) | NOT NULL, UNIQUE | - | 用户名 |
| password | VARCHAR(128) | NOT NULL | - | 密码(SHA256) |
| email | VARCHAR(100) | DEFAULT NULL | - | 邮箱 |
| nickname | VARCHAR(100) | DEFAULT NULL | - | 昵称 |
| phone | VARCHAR(20) | DEFAULT NULL | - | 手机号 |
| avatar | VARCHAR(255) | DEFAULT NULL | - | 头像URL |
| role | INT | DEFAULT 0 | 0 | 角色 (0:User, 1:Admin) |
| status | INT | DEFAULT 0 | 0 | 状态 (0:Normal, 1:Banned) |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |
| updated_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP | - | 更新时间 |

### 3.3 提交记录表 (submissions)

**表描述**: 存储用户的代码提交记录。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 提交ID |
| user_id | INT | NOT NULL | - | 用户ID |
| question_id | INT | NOT NULL | - | 题目ID |
| result | VARCHAR(10) | NOT NULL | - | 结果状态码 |
| cpu_time | INT | DEFAULT 0 | 0 | 运行耗时(ms) |
| mem_usage | INT | DEFAULT 0 | 0 | 内存使用(KB) |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 提交时间 |
| content | TEXT | - | - | 提交代码 |
| language | VARCHAR(20) | DEFAULT 'cpp' | 'cpp' | 编程语言 |

### 3.4 讨论表 (discussions)

**表描述**: 存储社区讨论文章。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 讨论ID |
| title | VARCHAR(255) | NOT NULL | - | 标题 |
| content | TEXT | NOT NULL | - | 内容(Markdown) |
| author_id | INT | NOT NULL | - | 作者ID |
| question_id | INT | DEFAULT 0 | 0 | 关联题目ID |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |
| likes | INT | DEFAULT 0 | 0 | 点赞数 |
| views | INT | DEFAULT 0 | 0 | 浏览数 |
| is_official | TINYINT(1) | DEFAULT 0 | 0 | 是否官方置顶 |

**索引**:
- `INDEX idx_author_id (author_id)`
- `INDEX idx_question_id (question_id)`

### 3.5 内联评论表 (inline_comments)

**表描述**: 存储文章的具体选区评论。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 评论ID |
| user_id | INT | NOT NULL | - | 用户ID |
| post_id | INT | NOT NULL | - | 文章ID |
| content | TEXT | NOT NULL | - | 内容 |
| selected_text | TEXT | - | - | 选中文本 |
| parent_id | INT | DEFAULT 0 | 0 | 父评论ID |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |

**索引**:
- `INDEX idx_post_id (post_id)`

### 3.6 文章评论表 (article_comments)

**表描述**: 存储文章的全局评论。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 评论ID |
| post_id | INT | NOT NULL | - | 文章ID |
| user_id | INT | NOT NULL | - | 用户ID |
| content | TEXT | NOT NULL | - | 内容 |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |
| likes | INT | DEFAULT 0 | 0 | 点赞数 |

**索引**:
- `INDEX idx_post_id (post_id)`
- `INDEX idx_user_id (user_id)`

### 3.7 竞赛表 (contests)

**表描述**: 存储爬取的竞赛信息。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | ID |
| contest_id | VARCHAR(50) | NOT NULL | - | 平台竞赛ID |
| name | VARCHAR(255) | NOT NULL | - | 名称 |
| start_time | DATETIME | NOT NULL | - | 开始时间 |
| end_time | DATETIME | NOT NULL | - | 结束时间 |
| link | VARCHAR(255) | NOT NULL | - | 链接 |
| source | VARCHAR(50) | DEFAULT 'Codeforces' | - | 来源 |
| status | VARCHAR(20) | DEFAULT 'upcoming' | - | 状态 |
| last_crawl_time | DATETIME | DEFAULT NULL | - | 最后爬取时间 |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |
| updated_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP | - | 更新时间 |

**索引**:
- `UNIQUE KEY idx_source_id (source, contest_id)`
- `INDEX idx_status_time (status, start_time DESC)`

### 3.8 题单表 (training_lists)

**表描述**: 存储训练计划/题单元数据。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | ID |
| title | VARCHAR(255) | NOT NULL | - | 标题 |
| description | TEXT | - | - | 描述 |
| difficulty | VARCHAR(50) | DEFAULT 'Unrated' | - | 难度 |
| tags | TEXT | - | - | 标签 |
| author_id | INT | NOT NULL | - | 作者ID |
| visibility | ENUM | DEFAULT 'public' | - | 可见性 |
| likes | INT | DEFAULT 0 | 0 | 点赞数 |
| collections | INT | DEFAULT 0 | 0 | 收藏数 |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |
| updated_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP | - | 更新时间 |

**索引**:
- `INDEX idx_author_id (author_id)`

### 3.9 题单题目关联表 (training_list_items)

**表描述**: 存储题单中包含的题目。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | ID |
| training_list_id | INT | NOT NULL | - | 题单ID |
| question_id | INT | NOT NULL | - | 题目ID |
| order_index | INT | NOT NULL DEFAULT 0 | 0 | 排序索引 |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |

**索引**:
- `INDEX idx_list_id (training_list_id)`
- `INDEX idx_question_id (question_id)`
- `UNIQUE KEY unique_item (training_list_id, question_id)`

### 3.10 邀请码表 (invitation_codes)

**表描述**: 存储管理员注册邀请码。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | ID |
| code | VARCHAR(50) | NOT NULL, UNIQUE | - | 邀请码 |
| is_used | TINYINT(1) | DEFAULT 0 | 0 | 是否已使用 |
| used_by | INT | DEFAULT 0 | 0 | 使用者ID |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |

### 3.11 操作日志表 (operation_logs)

**表描述**: 存储管理员的关键操作日志。

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | ID |
| user_id | INT | NOT NULL | - | 操作者ID |
| action | VARCHAR(50) | NOT NULL | - | 动作类型 |
| target | VARCHAR(100) | - | - | 操作对象 |
| details | TEXT | - | - | 详细信息 |
| ip | VARCHAR(50) | - | - | 操作IP |
| created_at | TIMESTAMP | DEFAULT CURRENT_TIMESTAMP | - | 创建时间 |

**索引**:
- `INDEX idx_user_id (user_id)`

---

**文档版本**: v1.2.8  
**最后更新时间**: 2026-03-16  
**维护团队**: 在线评测系统开发团队
