# 在线评测系统API接口文档

## 1. 概述

本文档描述了冻梨OJ（在线评测系统）的所有API接口，包括用户认证、题目管理、代码评测等核心功能。

### 1.1 接口规范
- **协议**: HTTP/1.1
- **数据格式**: JSON
- **字符编码**: UTF-8
- **认证方式**: Cookie-based Session

### 1.2 基础信息
- **基础URL**: `http://localhost:8080`
- **API版本**: v1
- **状态码**: 遵循HTTP标准状态码

## 2. 认证接口

### 2.1 用户注册

**接口描述**: 注册新用户账号

**请求信息**:
```http
POST /api/register
Content-Type: application/json
```

**请求参数**:

| 参数名 | 类型 | 必需 | 描述 | 限制 |
|--------|------|------|------|------|
| username | string | 是 | 用户名 | 3-20个字符，只能包含字母、数字、下划线 |
| password | string | 是 | 密码 | 6-30个字符 |
| email | string | 是 | 邮箱地址 | 有效的邮箱格式 |
| nickname | string | 否 | 昵称 | 最大50个字符 |
| phone | string | 否 | 手机号 | 有效的手机号格式 |

**请求示例**:
```json
{
    "username": "testuser",
    "password": "testpass123",
    "email": "test@example.com",
    "nickname": "测试用户",
    "phone": "13800138000"
}
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success"
}
```

**错误响应** (200 OK):
```json
{
    "status": 1,
    "reason": "注册失败：用户名已存在"
}
```

**错误码说明**:
- `status: 1` - 注册失败，具体原因在reason字段中

### 2.2 用户登录

**接口描述**: 用户登录认证

**请求信息**:
```http
POST /api/login
Content-Type: application/json
```

**请求参数**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| username | string | 是 | 用户名 |
| password | string | 是 | 密码 |

**请求示例**:
```json
{
    "username": "testuser",
    "password": "testpass123"
}
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success"
}
```
**响应头**:
```http
Set-Cookie: session_id=abc123def456; Path=/; Max-Age=86400; HttpOnly
```

**错误响应** (200 OK):
```json
{
    "status": 1,
    "reason": "用户名或密码错误"
}
```

### 2.3 获取用户信息

**接口描述**: 获取当前登录用户的信息

**请求信息**:
```http
GET /api/user
Cookie: session_id=abc123def456
```

**请求参数**: 无

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "username": "testuser",
    "email": "test@example.com",
    "nickname": "测试用户"
}
```

**错误响应** (200 OK):
```json
{
    "status": 1,
    "reason": "未登录"
}
```

### 2.4 获取个人中心数据

**接口描述**: 获取当前登录用户的详细个人信息和做题统计数据（需要登录）

**请求信息**:
```http
GET /api/profile
Cookie: session_id=abc123def456
```

**请求参数**: 无

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success",
    "username": "testuser",
    "email": "test@example.com",
    "nickname": "测试用户",
    "phone": "13800138000",
    "created_at": "2026-01-08 12:00:00",
    "stats": {
        "简单": 10,
        "中等": 5,
        "困难": 2
    }
}
```

**错误响应** (200 OK):
```json
{
    "status": 1,
    "reason": "未登录"
}
```

### 2.5 获取提交记录

**接口描述**: 查询用户的提交历史记录（需要登录）

**请求信息**:
```http
GET /api/submissions?user_id=1&page=1&page_size=20
Cookie: session_id=abc123def456
```

**请求参数**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| user_id | string | 否 | 用户ID筛选 |
| question_id | string | 否 | 题目ID筛选 |
| status | string | 否 | 状态筛选 |
| start_time | string | 否 | 开始时间 |
| end_time | string | 否 | 结束时间 |
| keyword | string | 否 | 关键词搜索 |
| page | int | 否 | 页码 (默认1) |
| page_size | int | 否 | 每页数量 (默认20) |

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "total": 100,
    "page": 1,
    "page_size": 20,
    "data": [
        {
            "id": "1",
            "user_id": "1",
            "question_id": "1",
            "language": "cpp",
            "result": "0",
            "cpu_time": 10,
            "mem_usage": 1024,
            "created_at": "2026-01-10 10:00:00",
            "content": "#include..."
        }
    ]
}
```

**错误响应** (200 OK):
```json
{
    "status": 1,
    "reason": "Database Error"
}
```

## 3. 题目接口

### 3.1 获取题目列表

**接口描述**: 获取所有题目列表（需要登录）

**请求信息**:
```http
GET /all_questions
Cookie: session_id=abc123def456
```

**请求参数**: 无

**响应信息**:

**成功响应** (200 OK):
返回HTML页面，包含题目列表

**错误响应** (302 Found):
重定向到登录页面 `/login`

### 3.2 获取题目详情

**接口描述**: 获取指定题目的详细信息（需要登录）

**请求信息**:
```http
GET /question/{number}
Cookie: session_id=abc123def456
```

**路径参数**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| number | string | 是 | 题目编号 |

**请求示例**:
```http
GET /question/1
Cookie: session_id=abc123def456
```

**响应信息**:

**成功响应** (200 OK):
返回HTML页面，包含题目详情和代码编辑器

**错误响应** (302 Found):
重定向到登录页面 `/login`

## 4. 评测接口

### 4.1 代码评测

**接口描述**: 提交代码进行评测（需要登录）

**请求信息**:
```http
POST /judge/{number}
Content-Type: application/json
Cookie: session_id=abc123def456
```

**路径参数**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| number | string | 是 | 题目编号 |

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| code | string | 是 | 用户提交的代码 |
| language | string | 是 | 编程语言 (枚举: cpp, java, python) |

**请求示例**:
```http
POST /judge/1
Content-Type: application/json
Cookie: session_id=abc123def456

{
    "code": "#include <iostream>\nusing namespace std;\nint main() {\n    cout << \"Hello World\" << endl;\n    return 0;\n}",
    "language": "cpp"
}
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "编译运行成功",
    "stdout": "Hello World\n",
    "stderr": ""
}
```

**错误响应** (200 OK):
```json
{
    "status": -1,
    "reason": "请先登录"
}
```

**评测状态码**:

| 状态码 | 含义 |
|--------|------|
| 0 | 编译运行成功 |
| -1 | 提交的代码为空 |
| -2 | 未知错误 |
| -3 | 编译错误 |
| 6 | 内存超过限制 (SIGABRT) |
| 24 | CPU使用超时 (SIGXCPU) |
| 8 | 浮点数溢出 (SIGFPE) |
| 其他 | 运行时错误 |

## 5. 页面接口

### 5.1 首页

**接口描述**: 访问系统首页

**请求信息**:
```http
GET /
```

**响应信息**:

**成功响应** (302 Found):
重定向到题目列表页面 `/all_questions`

### 5.2 登录页面

**接口描述**: 获取登录页面

**请求信息**:
```http
GET /login
```

**响应信息**:

**成功响应** (200 OK):
返回HTML登录页面

### 5.3 个人中心页面

**接口描述**: 访问用户个人中心页面（需要登录）

**请求信息**:
```http
GET /profile
Cookie: session_id=abc123def456
```

**响应信息**:

**成功响应** (200 OK):
返回HTML页面，包含用户信息和做题统计

**错误响应** (302 Found):
重定向到登录页面 `/login`

## 6. 管理员接口

### 6.1 获取题目列表 (管理员)

**接口描述**: 获取所有题目列表，包含未发布题目（需要管理员权限）

**请求信息**:
```http
GET /api/admin/questions
Cookie: session_id=abc123def456
```

**请求参数**: 无

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success",
    "data": [
        {
            "number": "1",
            "title": "两数之和",
            "star": "简单",
            "status": 1
        },
        {
            "number": "2",
            "title": "未发布题目",
            "star": "困难",
            "status": 0
        }
    ]
}
```

**错误响应** (403 Forbidden):
```json
{
    "status": 1,
    "reason": "权限不足"
}
```

### 6.2 创建题目

**接口描述**: 创建新题目（需要管理员权限）

**请求信息**:
```http
POST /api/admin/question
Content-Type: application/json
Cookie: session_id=abc123def456
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| title | string | 是 | 题目标题 |
| star | string | 是 | 难度 (简单/中等/困难) |
| description | string | 是 | 题目描述 (Markdown) |
| tail | string | 是 | 测试用例 (JSON字符串) |
| cpu_limit | int | 是 | CPU限制 (秒) |
| mem_limit | int | 是 | 内存限制 (KB) |
| status | int | 否 | 状态 (0:Hidden, 1:Visible) |

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success"
}
```

### 6.3 更新题目

**接口描述**: 更新指定题目信息（需要管理员权限）

**请求信息**:
```http
POST /api/admin/question/update/{id}
Content-Type: application/json
Cookie: session_id=abc123def456
```

**路径参数**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| id | string | 是 | 题目编号 |

**请求体**: 同创建题目接口

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success"
}
```

### 6.4 删除题目

**接口描述**: 删除指定题目（需要管理员权限）

**请求信息**:
```http
POST /api/admin/question/delete/{id}
Cookie: session_id=abc123def456
```

**路径参数**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| id | string | 是 | 题目编号 |

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success"
}
```

## 7. 编译服务器接口

### 6.1 编译运行服务

**接口描述**: 编译并运行代码（内部使用）

**请求信息**:
```http
POST /compile_and_run
Content-Type: application/json
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| code | string | 是 | 要编译运行的代码 |
| language | string | 是 | 编程语言 (枚举: cpp, java, python) |
| input | string | 否 | 标准输入数据 |
| cpu_limit | number | 是 | CPU时间限制（秒） |
| mem_limit | number | 是 | 内存限制（KB） |

**请求示例**:
```json
{
    "code": "#include <iostream>\nusing namespace std;\nint main() {\n    int a, b;\n    cin >> a >> b;\n    cout << a + b << endl;\n    return 0;\n}",
    "input": "3 5",
    "cpu_limit": 1,
    "mem_limit": 10240
}
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "编译运行成功",
    "stdout": "8\n",
    "stderr": ""
}
```

**错误响应** (200 OK):
```json
{
    "status": -3,
    "reason": "编译错误",
    "stdout": "",
    "stderr": "error: expected ';' before '}' token"
}
```

## 8. 讨论区接口

### 8.1 获取讨论列表

**接口描述**: 获取讨论区文章列表

**请求信息**:
```http
GET /api/discussions
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success",
    "data": [
        {
            "id": 1,
            "title": "Discussion Title",
            "author_id": 1,
            "likes": 10,
            "views": 100,
            "created_at": "2026-01-27 10:00:00"
        }
    ]
}
```

### 8.2 获取讨论详情

**接口描述**: 获取单篇讨论详情

**请求信息**:
```http
GET /api/discussion/:id
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success",
    "data": {
        "id": 1,
        "title": "Title",
        "content": "Markdown Content",
        "author_id": 1,
        "likes": 10,
        "views": 100
    }
}
```

### 8.3 创建讨论

**接口描述**: 发布新的讨论文章

**请求信息**:
```http
POST /api/discussion
Content-Type: application/json
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| title | string | 是 | 标题 |
| content | string | 是 | 内容 (Markdown) |

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success"
}
```

### 8.4 获取内联评论

**接口描述**: 获取文章的内联评论

**请求信息**:
```http
GET /api/inline_comments/:post_id
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "success",
    "data": [
        {
            "id": 1,
            "content": "Comment",
            "selected_text": "Selected",
            "user_id": 1,
            "created_at": "..."
        }
    ]
}
```

### 8.5 添加内联评论

**接口描述**: 添加内联评论

**请求信息**:
```http
POST /api/inline_comment/add
Content-Type: application/json
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| post_id | int | 是 | 文章ID |
| content | string | 是 | 评论内容 |
| selected_text | string | 是 | 选中文本 |
| parent_id | int | 否 | 父评论ID |

### 8.6 删除内联评论

**接口描述**: 删除内联评论

**请求信息**:
```http
POST /api/inline_comment/delete
Content-Type: application/json
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| comment_id | int | 是 | 评论ID |

### 8.7 获取文章评论

**接口描述**: 获取文章的全局评论

**请求信息**:
```http
GET /api/article_comments/:post_id
```

### 8.8 添加文章评论

**接口描述**: 添加文章全局评论

**请求信息**:
```http
POST /api/article_comment/add
Content-Type: application/json
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| post_id | int | 是 | 文章ID |
| content | string | 是 | 评论内容 |

### 8.9 上传图片

**接口描述**: 上传图片到服务器

**请求信息**:
```http
POST /api/upload_image
Content-Type: multipart/form-data
```

## 9. 错误处理

### 9.1 通用错误码

| HTTP状态码 | 描述 |
|-------------|------|
| 200 | 请求成功（业务状态在JSON中） |
| 302 | 重定向（未登录时） |
| 400 | 请求参数错误 |
| 401 | 未认证（理论上不会出现，使用302重定向） |
| 403 | 权限不足 |
| 404 | 资源不存在 |
| 500 | 服务器内部错误 |
| 503 | 服务不可用 |

### 9.2 业务错误码

| 状态码 | 含义 |
|--------|------|
| 0 | 成功 |
| 1 | 业务错误（具体原因在reason字段） |
| -1 | 代码为空 |
| -2 | 未知错误 |
| -3 | 编译错误 |

## 10. 请求示例

### 10.1 完整评测流程

```bash
# 1. 用户注册
curl -X POST http://localhost:8080/api/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "testpass123",
    "email": "test@example.com"
  }'

# 2. 用户登录
curl -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "testpass123"
  }' \
  -c cookies.txt

# 3. 获取题目列表（需要登录）
curl -X GET http://localhost:8080/all_questions \
  -b cookies.txt

# 4. 提交代码评测
curl -X POST http://localhost:8080/judge/1 \
  -H "Content-Type: application/json" \
  -b cookies.txt \
  -d '{
    "code": "#include <iostream>\nusing namespace std;\nint main() {\n    cout << \"Hello World\" << endl;\n    return 0;\n}"
  }'
```

## 11. 安全说明

### 11.1 认证安全
- 使用HTTPS协议传输敏感信息
- Session Cookie设置HttpOnly属性
- Session有效期为24小时
- 密码使用SHA256哈希存储

### 10.2 输入验证
- 所有输入参数都进行长度和格式验证
- SQL注入防护（需要改进为参数化查询）
- 代码执行沙箱环境
- 资源使用限制（CPU、内存）

### 10.3 错误处理
- 不暴露系统内部错误信息
- 用户友好的错误提示
- 详细的日志记录（服务器端）

## 11. 性能说明

### 11.1 响应时间
- 用户认证: < 100ms
- 题目列表: < 100ms
- 题目详情: < 200ms
- 代码评测: < 5s（包含编译运行时间）

### 10.2 并发限制
- 单用户并发请求：无明确限制
- 系统总并发：依赖服务器配置
- 编译任务并发：受编译服务器数量限制

## 13. 版本历史

### v0.3.0 (2026-01-21)
- 新增管理员接口：
  - 获取题目列表 (包含未发布)
  - 创建/更新/删除题目
- 权限控制升级：增加管理员角色校验

### v1.0.0 (2026-01-08)
- 初始版本
- 支持用户注册、登录
- 支持题目列表和详情查看
- 支持代码评测
- 基础的错误处理

---

**文档版本**: v0.3.5  
**最后更新时间**: 2026-01-28  
**维护团队**: 在线评测系统开发团队