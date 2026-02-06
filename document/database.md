# 在线评测系统数据库设计文档

## 1. 概述

本文档描述了负载均衡式在线评测系统的数据库设计方案。系统采用MySQL 8.0+作为主要数据库，支持题目管理、用户认证、代码评测等核心功能。同时，系统使用JSON文件（或可选Redis）存储部分非关系型数据（如竞赛列表）。

## 2. 数据库环境配置

### 2.1 数据库连接信息
- **数据库类型**: MySQL 8.0+
- **主机地址**: 127.0.0.1
- **端口号**: 3306
- **数据库名称**: oj
- **用户名**: oj_client
- **密码**: <db_password>
- **字符集**: utf8mb4
- **排序规则**: utf8mb4_unicode_ci

### 2.2 数据库初始化

使用项目提供的SQL脚本进行数据库初始化：

```sql
-- 创建数据库
CREATE DATABASE IF NOT EXISTS oj CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建用户并授权
CREATE USER IF NOT EXISTS 'oj_client'@'localhost' IDENTIFIED BY '<db_password>';
GRANT ALL PRIVILEGES ON oj.* TO 'oj_client'@'localhost';
FLUSH PRIVILEGES;
```

## 3. 数据表设计

### 3.1 题目表 (oj_questions)

**表描述**: 存储在线评测系统的题目信息，包括题目描述、测试用例、资源限制等

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| number | INT | PRIMARY KEY, AUTO_INCREMENT | - | 题目编号，唯一标识 |
| title | VARCHAR(255) | NOT NULL | - | 题目标题 |
| star | VARCHAR(50) | NOT NULL | - | 题目难度等级（简单/中等/困难） |
| cpu_limit | INT | NOT NULL | 1 | CPU时间限制（秒） |
| mem_limit | INT | NOT NULL | 30000 | 内存限制（KB） |
| description | TEXT | NOT NULL | - | 题目描述，支持Markdown格式 |
| header | TEXT | DEFAULT NULL | - | [已废弃] 题目预设代码头 |
| tail_code | TEXT | NOT NULL | - | JSON格式的测试用例，存储测试用例的输入输出 |
| status | INT | DEFAULT 1 | 1 | 题目状态 (0:Hidden, 1:Visible) |
| created_at | TIMESTAMP | - | CURRENT_TIMESTAMP | 创建时间 |
| updated_at | TIMESTAMP | - | CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP | 更新时间 |

**索引设计**:
- `PRIMARY KEY (number)`: 主键索引
- `idx_star`: 基于难度等级的索引，用于按难度筛选题目
- `idx_title`: 基于标题的索引，用于题目搜索

**示例数据**:
```sql
INSERT INTO oj_questions (number, title, star, cpu_limit, mem_limit, description, header, tail_code, status) VALUES
(1, '两数之和', '简单', 1, 30000,
 '给定一个整数数组 nums 和一个整数目标值 target...',
 '',
 '[{\"stdin\":\"2 7 11 15\\n9\\n\",\"expected\":\"0 1\\n\"}]',
 1);
```

### 3.2 用户表 (users)

**表描述**: 存储系统用户信息，支持用户注册和登录认证

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 用户ID，唯一标识 |
| username | VARCHAR(50) | NOT NULL, UNIQUE | - | 用户名，唯一 |
| password | VARCHAR(128) | NOT NULL | - | 密码（SHA256哈希） |
| email | VARCHAR(100) | DEFAULT NULL | NULL | 用户邮箱地址 |
| nickname | VARCHAR(100) | DEFAULT NULL | NULL | 用户昵称 |
| phone | VARCHAR(20) | DEFAULT NULL | NULL | 用户手机号 |
| role | INT | DEFAULT 0 | 0 | 用户角色 (0:User, 1:Admin) |
| created_at | TIMESTAMP | - | CURRENT_TIMESTAMP | 创建时间 |
| updated_at | TIMESTAMP | - | CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP | 更新时间 |

**索引设计**:
- `PRIMARY KEY (id)`: 主键索引
- `UNIQUE KEY username (username)`: 用户名唯一索引
- `idx_email`: 邮箱索引，用于邮箱相关查询

**示例数据**:
```sql
INSERT INTO users (username, password, email, nickname, phone) VALUES
('testuser', 'a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3', 'test@example.com', '测试用户', '13800138000');
```

### 3.3 提交记录表 (submissions)

**表描述**: 存储用户的代码提交记录和评测结果，用于统计用户做题数据

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 提交ID，唯一标识 |
| user_id | INT | NOT NULL | - | 用户ID，关联users表 |
| question_id | INT | NOT NULL | - | 题目ID，关联oj_questions表 |
| language | VARCHAR(20) | NOT NULL | 'cpp' | 编程语言 (cpp, java, python) |
| result | VARCHAR(10) | NOT NULL | - | 评测结果状态码（0为通过） |
| cpu_time | INT | DEFAULT 0 | 0 | 运行耗时（毫秒） |
| mem_usage | INT | DEFAULT 0 | 0 | 内存使用（KB） |
| created_at | TIMESTAMP | - | CURRENT_TIMESTAMP | 提交时间 |
| content | TEXT | - | - | 提交的代码内容 |

**索引设计**:
- `PRIMARY KEY (id)`: 主键索引
- `INDEX idx_user_id (user_id)`: 用户ID索引，用于查询用户提交记录
- `INDEX idx_question_id (question_id)`: 题目ID索引，用于查询题目提交统计

**示例数据**:
```sql
INSERT INTO submissions (user_id, question_id, language, result, cpu_time, mem_usage, content) VALUES
(1, 1, 'cpp', '0', 5, 128, '#include <iostream>...');
```

### 3.4 讨论表 (discussions)

**表描述**: 存储社区讨论文章信息

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 讨论ID |
| title | VARCHAR(255) | NOT NULL | - | 标题 |
| content | TEXT | NOT NULL | - | 内容 (Markdown) |
| author_id | INT | NOT NULL | - | 作者ID |
| created_at | TIMESTAMP | - | CURRENT_TIMESTAMP | 创建时间 |
| likes | INT | DEFAULT 0 | 0 | 点赞数 |
| views | INT | DEFAULT 0 | 0 | 浏览数 |
| is_official | TINYINT | DEFAULT 0 | 0 | 是否官方/置顶 (0:否, 1:是) |

### 3.5 内联评论表 (inline_comments)

**表描述**: 存储文章的具体段落/选区的评论

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 评论ID |
| user_id | INT | NOT NULL | - | 用户ID |
| post_id | INT | NOT NULL | - | 文章ID |
| content | TEXT | NOT NULL | - | 评论内容 |
| selected_text | TEXT | - | - | 被选中的原文文本 |
| parent_id | INT | DEFAULT NULL | NULL | 父评论ID (用于回复) |
| created_at | TIMESTAMP | - | CURRENT_TIMESTAMP | 创建时间 |

### 3.6 文章评论表 (article_comments)

**表描述**: 存储文章的全局评论

**表结构**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 评论ID |
| post_id | INT | NOT NULL | - | 文章ID |
| user_id | INT | NOT NULL | - | 用户ID |
| content | TEXT | NOT NULL | - | 评论内容 |
| created_at | TIMESTAMP | - | CURRENT_TIMESTAMP | 创建时间 |
| likes | INT | DEFAULT 0 | 0 | 点赞数 |

### 3.7 竞赛数据 (MySQL Table)

**存储方式**: MySQL 数据库表 `contests`

**数据描述**: 存储从 Codeforces 和 LeetCode 爬取的近期竞赛信息。

**表结构 (contests)**:

| 字段名 | 数据类型 | 约束 | 默认值 | 描述 |
|--------|----------|------|--------|------|
| id | INT | PRIMARY KEY, AUTO_INCREMENT | - | 自增主键 |
| contest_id | VARCHAR(50) | NOT NULL | - | 平台竞赛ID |
| name | VARCHAR(255) | NOT NULL | - | 竞赛名称 |
| start_time | DATETIME | NOT NULL | - | 开始时间 |
| end_time | DATETIME | NOT NULL | - | 结束时间 |
| link | VARCHAR(255) | NOT NULL | - | 竞赛链接 |
| source | VARCHAR(50) | DEFAULT 'Codeforces' | Codeforces | 来源 (Codeforces/LeetCode) |
| status | VARCHAR(20) | DEFAULT 'upcoming' | upcoming | 状态 (upcoming/running/ended) |
| last_crawl_time | DATETIME | DEFAULT NULL | NULL | 最后爬取时间 |
| created_at | TIMESTAMP | - | CURRENT_TIMESTAMP | 创建时间 |
| updated_at | TIMESTAMP | - | CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP | 更新时间 |

**索引设计**:
- `PRIMARY KEY (id)`: 主键索引
- `UNIQUE KEY idx_source_id (source, contest_id)`: 竞赛唯一性约束（按来源+平台ID）
- `INDEX idx_status_time (status, start_time)`: 按状态与时间查询优化

## 4. 数据访问层设计

### 4.1 核心类结构

系统采用C++实现数据访问层，主要类包括：

#### Question结构体
```cpp
struct Question {
    std::string number;     // 题目编号
    std::string title;      // 题目标题
    std::string star;       // 难度等级
    std::string desc;       // 题目描述
    std::string tail;       // 测试用例(JSON)
    int cpu_limit;          // CPU时间限制
    int mem_limit;          // 内存限制
};
```

#### User结构体
```cpp
struct User {
    std::string id;         // 用户ID
    std::string username;   // 用户名
    std::string password;   // 密码哈希
    std::string email;      // 邮箱地址
    std::string nickname;   // 昵称
    std::string phone;      // 手机号
};
```

#### Model类核心方法
```cpp
class Model {
public:
    // 题目相关操作
    bool GetAllQuestions(vector<Question> *out);           // 获取所有题目
    bool GetOneQuestion(const string &number, Question *q); // 获取单个题目
    
    // 用户相关操作
    bool RegisterUser(const string &username, const string &password, 
                     const string &email, const string &nickname, 
                     const string &phone); // 用户注册
    bool LoginUser(const string &username, const string &password, User *user); // 用户登录
    bool CheckUserExists(const string &username); // 检查用户是否存在
    
    // 通用数据库操作
    bool ExecuteSql(const string &sql);                    // 执行SQL语句
    bool QueryMySql(const string &sql, vector<Question> *out);  // 查询题目
    bool QueryUserMySql(const string &sql, vector<User> *out);  // 查询用户
    
private:
    void InitUserTable(); // 初始化用户表
    std::string SHA256Hash(const std::string &str); // SHA256哈希
};
```

### 4.2 安全设计

#### 4.2.1 密码安全
- 使用SHA256算法对用户密码进行哈希处理
- 不存储明文密码，只存储哈希值
- 密码哈希生成示例：

```cpp
std::string Model::SHA256Hash(const std::string &str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    
    // 转换为十六进制字符串
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}
```

#### 4.2.2 SQL注入防护
- 使用参数化查询（建议后续改进）
- 对用户输入进行严格的验证和过滤
- 使用MySQL C API的安全函数
- 输入验证示例：

```cpp
bool Model::RegisterUser(const string &username, const string &password,
                        const string &email, const string &nickname,
                        const string &phone) {
    // 参数验证
    if(username.empty() || password.empty() || email.empty()) {
        return false;
    }
    if(username.length() < 3 || username.length() > 20) {
        return false;
    }
    if(password.length() < 6 || password.length() > 30) {
        return false;
    }
    
    // 检查用户名是否已存在
    if(CheckUserExists(username)) {
        return false;
    }
    
    // 密码哈希
    std::string hashed_password = SHA256Hash(password);
    
    // 构建SQL语句
    std::string sql = "INSERT INTO users (username, password, email, nickname, phone) VALUES ('";
    sql += username + "', '" + hashed_password + "', '" + email + "', '";
    sql += nickname + "', '" + phone + "')";
    
    return ExecuteSql(sql);
}
```

### 4.3 数据库连接管理

#### 4.3.1 连接配置常量
```cpp
const std::string host = "127.0.0.1";
const std::string user = "oj_client";
const std::string passwd = "YOUR_PASSWORD";
const std::string db = "oj";
const int port = 3306;
```

#### 4.3.2 连接池设计
当前实现采用简单的连接管理方式：
- 每次操作都创建新的数据库连接
- 操作完成后立即关闭连接
- 设置字符集为utf8避免中文乱码

#### 4.3.3 连接示例代码
```cpp
bool Model::ExecuteSql(const std::string &sql) {
    MYSQL *my = mysql_init(nullptr);
    if(nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), 
                                   passwd.c_str(), db.c_str(), port, nullptr, 0)){
        LOG(FATAL) << "连接数据库失败!" << "\n";
        return false;
    }
    mysql_set_character_set(my, "utf8");
    
    if(0 != mysql_query(my, sql.c_str())) {
        LOG(WARNING) << sql << " execute error: " << mysql_error(my) << "\n";
        mysql_close(my);
        return false;
    }
    
    mysql_close(my);
    return true;
}
```

## 5. 索引设计策略

### 5.1 题目表索引
```sql
-- 主键索引（自动创建）
PRIMARY KEY (number)

-- 难度等级索引，用于按难度筛选题目
INDEX idx_star (star)

-- 标题索引，用于题目搜索
INDEX idx_title (title)
```

### 5.2 用户表索引
```sql
-- 主键索引（自动创建）
PRIMARY KEY (id)

-- 用户名唯一索引
UNIQUE KEY username (username)

-- 邮箱索引，用于邮箱相关查询
INDEX idx_email (email)
```

## 6. 数据完整性约束

### 6.1 实体完整性
- 所有表都有主键，确保记录的唯一性
- 用户表的用户名字段有唯一约束

### 6.2 参照完整性
- 题目编号和用户ID都是自增的，确保连续性
- 外键关系（未来扩展）

### 6.3 域完整性
- 非空约束：关键字段如用户名、密码、题目标题等不能为空
- 默认值：创建时间和更新时间有默认值
- 数据类型约束：确保数据类型的正确性

## 7. 性能优化建议

### 7.1 索引优化
- 为经常查询的字段添加索引（已添加star、title索引）
- 避免在WHERE子句中使用函数，以免索引失效
- 定期分析和优化表结构
- 使用复合索引优化多条件查询

### 7.2 查询优化
- 使用LIMIT限制返回结果数量
- 避免SELECT *，只查询需要的字段
- 使用JOIN代替子查询（如需要复杂查询）
- 合理使用WHERE条件，减少数据扫描范围

### 7.3 连接优化
- 考虑实现连接池，减少连接创建开销
- 设置合理的连接超时时间
- 使用持久连接（长连接）
- 监控连接数，避免连接泄漏

### 7.4 表结构优化
- 选择合适的数据类型，避免过度设计
- 合理使用TEXT类型，大字段单独存储
- 定期清理过期数据
- 使用分区表处理大数据量（未来扩展）

## 8. 扩展性设计

### 8.1 未来可能的扩展
- **提交记录表**: 记录用户代码提交历史
- **评测结果表**: 存储详细的评测结果信息
- **题目分类表**: 支持题目按算法类型分类
- **用户统计表**: 记录用户解题统计信息
- **竞赛表**: 支持在线竞赛功能
- **评论表**: 支持题目讨论和评论

### 8.2 数据库版本管理
- 使用数据库迁移脚本管理表结构变更
- 保持向后兼容性
- 记录数据库版本信息
- 支持回滚操作

### 8.3 分库分表策略（大数据量时）
- 按用户ID分片存储提交记录
- 按时间分区存储日志数据
- 使用读写分离提高查询性能

## 9. 备份与恢复

### 9.1 备份策略
- **定期备份**: 建议每日全量备份
- **增量备份**: 每小时增量备份binlog
- **备份验证**: 定期验证备份文件的完整性
- **多地备份**: 本地+异地双备份

### 9.2 备份命令
```bash
# 全量备份
mysqldump -u root -p --single-transaction --routines --triggers oj > oj_backup_$(date +%Y%m%d).sql

# 增量备份（需要开启binlog）
mysqlbinlog --start-datetime="$(date -d '1 hour ago' +'%Y-%m-%d %H:%M:%S')" mysql-bin.000001 > increment_backup.sql
```

### 9.3 恢复流程
1. **停止应用服务**: 防止数据写入
2. **恢复数据库备份**: 使用备份文件恢复
3. **应用增量备份**: 如果有增量备份
4. **验证数据完整性**: 检查关键数据
5. **重启应用服务**: 恢复服务

## 10. 监控与维护

### 10.1 性能监控
- **连接数监控**: 监控数据库连接使用情况
- **查询性能监控**: 监控慢查询和查询执行时间
- **表空间监控**: 监控数据文件大小增长
- **锁等待监控**: 监控锁等待和死锁情况

### 10.2 定期维护
- **表优化**: 定期执行OPTIMIZE TABLE
- **统计信息更新**: 定期更新表统计信息
- **索引重建**: 定期重建碎片化的索引
- **数据清理**: 清理过期和无用的数据

### 10.3 监控指标
```sql
-- 查看数据库大小
SELECT table_schema AS 'Database', 
       SUM(data_length + index_length) / 1024 / 1024 AS 'Size (MB)'
FROM information_schema.tables 
GROUP BY table_schema;

-- 查看慢查询
SHOW PROCESSLIST;
SELECT * FROM information_schema.processlist WHERE TIME > 1;

-- 查看表状态
SHOW TABLE STATUS LIKE 'oj_questions';
```

## 11. 安全建议

### 11.1 数据库安全
- **强密码策略**: 使用复杂密码，定期更换
- **最小权限原则**: 只授予必要的权限
- **网络安全**: 使用SSL连接，限制访问IP
- **版本更新**: 及时更新数据库版本，修复安全漏洞

### 11.2 应用安全
- **输入验证**: 实现完善的输入验证
- **SQL注入防护**: 使用参数化查询
- **敏感数据加密**: 对敏感数据进行加密存储
- **访问控制**: 实现基于角色的访问控制
- **审计日志**: 记录数据库操作日志

### 11.3 安全配置示例
```sql
-- 创建专用用户，限制权限
CREATE USER 'oj_app'@'localhost' IDENTIFIED BY 'StrongPassword123!';
GRANT SELECT, INSERT, UPDATE ON oj.* TO 'oj_app'@'localhost';
FLUSH PRIVILEGES;

-- 启用SSL（需要配置SSL证书）
-- GRANT USAGE ON *.* TO 'oj_client'@'localhost' REQUIRE SSL;
```

## 12. 故障处理

### 12.1 常见故障
- **连接超时**: 检查网络连接和防火墙设置
- **权限错误**: 检查用户权限和密码
- **表损坏**: 使用REPAIR TABLE修复
- **死锁**: 优化事务和查询顺序

### 12.2 故障排查步骤
1. **查看错误日志**: 分析MySQL错误日志
2. **检查系统资源**: CPU、内存、磁盘空间
3. **验证配置**: 检查配置文件参数
4. **测试连接**: 使用命令行工具测试
5. **查看状态**: 使用SHOW STATUS命令

## 13. 版本历史

- **v0.2.9 (2026-01-10)**: 
  - 更新 `oj_questions` 表结构：
    - 修正字段拼写错误 (`oj_quesoions` -> `header`, `tj_questions` -> `tail`)
    - `header` 字段标记为废弃
    - `tail` 字段改为存储 JSON 格式的测试用例
    - 移除冗余的 `tail_code` 字段
  - 更新 `submissions` 表说明，明确支持多语言 (cpp, java, python)

---

**最后更新时间**: 2026-02-06  
**文档版本**: v1.0.0  
**维护团队**: 在线评测系统开发团队
