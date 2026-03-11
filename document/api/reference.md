# 在线评测系统API接口文档

## 1. 概述

本文档描述了冻梨OJ（在线评测系统）的所有API接口，包括用户认证、题目管理、代码评测、题单训练、社区讨论等核心功能。

### 1.1 接口规范
- **协议**: HTTP/1.1
- **数据格式**: JSON
- **字符编码**: UTF-8
- **认证方式**: Cookie-based Session

### 1.2 基础信息
- **基础URL**: `http://localhost:8094`
- **API版本**: v1
- **状态码**: 遵循HTTP标准状态码
 
 ### 1.3 前端调用约定
 - **BaseURL**: 前端通过 `/api` 代理请求，无需硬编码完整 URL。
 - **Auth**: 浏览器自动携带 Cookie，前端无需手动处理 Session ID。
 - **Error Handling**: 全局拦截器 (`src/lib/axios.ts`) 统一处理 401 未登录跳转。
 
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

### 2.3 获取用户信息

**接口描述**: 获取当前登录用户的信息

**请求信息**:
```http
GET /api/user
```

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "username": "testuser",
    "email": "test@example.com",
    "avatar": "/uploads/avatars/avatar_123.jpg"
}
```

### 2.4 获取个人中心数据

**接口描述**: 获取当前登录用户的详细个人信息和做题统计数据（需要登录）

**请求信息**:
```http
GET /api/profile
```

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
    "avatar": "/uploads/avatars/avatar_123.jpg",
    "role": 0,
    "created_at": "2026-01-08 12:00:00",
    "stats": {
        "简单": 10,
        "中等": 5,
        "困难": 2
    }
}
```

### 2.5 获取提交记录

**接口描述**: 查询用户的提交历史记录（需要登录）

**请求信息**:
```http
GET /api/submissions?user_id=1&page=1&page_size=20
```

**请求参数**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| user_id | string | 否 | 用户ID筛选 |
| question_id | string | 否 | 题目ID筛选 |
| status | string | 否 | 状态筛选 (如 "0" 代表成功) |
| start_time | string | 否 | 开始时间 |
| end_time | string | 否 | 结束时间 |
| keyword | string | 否 | 关键词搜索 (内容) |
| page | int | 否 | 页码 (默认1) |
| page_size | int | 否 | 每页数量 (默认20, 最大100) |

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
            "question_title": "两数之和",
            "result": "0",
            "cpu_time": 10,
            "mem_usage": 1024,
            "created_at": "2026-01-10 10:00:00",
            "content": "#include...",
            "language": "cpp"
        }
    ]
}
```

### 2.6 用户登出

**接口描述**: 退出登录并清除会话

**请求信息**:
```http
GET /api/logout
```

**响应信息**:
```json
{
    "status": 0,
    "reason": "success"
}
```

### 2.7 更新用户信息

**接口描述**: 更新用户的昵称、邮箱或手机号（需要登录）

**请求信息**:
```http
POST /api/user/update
Content-Type: application/json
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| nickname | string | 否 | 新昵称 |
| email | string | 否 | 新邮箱 |
| phone | string | 否 | 新手机号 |

### 2.8 上传用户头像

**接口描述**: 上传用户头像（需要登录）

**请求信息**:
```http
POST /api/upload_avatar
Content-Type: multipart/form-data
```

**表单参数**:
- `avatar`: 头像图片文件 (jpg/png/gif, max 2MB)

**响应信息**:
```json
{
    "status": 0,
    "url": "/uploads/avatars/avatar_1_123456789.jpg"
}
```

## 3. 题目接口

### 3.1 获取题目列表 (JSON)

**接口描述**: 获取题目列表数据的JSON格式

**请求信息**:
```http
GET /api/problems?page=1&page_size=20
```

**请求参数**:
- `page`: 页码
- `page_size`: 每页数量

**响应信息**:
```json
{
    "status": 0,
    "total": 50,
    "page": 1,
    "data": [
        {
            "number": "1",
            "title": "两数之和",
            "star": "简单"
        }
    ]
}
```

### 3.2 获取题目详情 (JSON)

**接口描述**: 获取指定题目的详细信息JSON（不含完整HTML）

**请求信息**:
```http
GET /api/question/{number}
```

**响应信息**:
```json
{
    "status": 0,
    "data": {
        "number": "1",
        "title": "两数之和",
        "star": "简单",
        "cpu_limit": 1,
        "mem_limit": 30000
    }
}
```

## 4. 评测接口

### 4.1 代码评测

**接口描述**: 提交代码进行评测（需要登录）

**请求信息**:
```http
POST /judge/{number}
Content-Type: application/json
```

**请求体**:

| 参数名 | 类型 | 必需 | 描述 |
|--------|------|------|------|
| code | string | 是 | 用户提交的代码 |
| language | string | 是 | 编程语言 (支持别名: cpp/c++/cc -> C++, java -> Java, python/py -> Python) |
| input | string | 否 | 自定义输入 (调试用) |

**响应信息**:

**成功响应** (200 OK):
```json
{
    "status": 0,
    "reason": "",
    "stdout": "{\"cases\":[{\"name\":\"Case 1\",\"pass\":true,\"input\":\"...\",\"output\":\"...\",\"expected\":\"...\"}],\"summary\":{\"total\":1,\"passed\":1,\"overall\":\"All Passed\"}}",
    "stderr": ""
}
```

## 5. 题单/训练计划接口

### 5.1 获取题单列表

**接口描述**: 获取题单列表

**请求信息**:
```http
GET /api/training/list?page=1&limit=20&visibility=public&author_id=
```

**请求参数**:
- `page`: 页码
- `limit`: 每页数量
- `visibility`: 可见性 (public/private)，留空为全部
- `author_id`: 作者ID，若为 "me" 则获取当前用户创建的题单

**响应信息**:
```json
{
    "status": 0,
    "total": 10,
    "data": [
        {
            "id": "1",
            "title": "DP基础",
            "difficulty": "中等",
            "author_name": "admin",
            "problem_count": 5,
            "likes": 10,
            "collections": 2
        }
    ]
}
```

### 5.2 获取题单详情

**接口描述**: 获取指定题单的详细信息及包含的题目

**请求信息**:
```http
GET /api/training/{id}
```

**响应信息**:
```json
{
    "status": 0,
    "data": {
        "id": "1",
        "title": "DP基础",
        "description": "动态规划入门",
        "difficulty": "中等",
        "tags": "[\"dp\",\"array\"]",
        "author_id": "1",
        "visibility": "public",
        "problems": [
            {
                "id": "1",
                "title": "两数之和",
                "difficulty": "简单",
                "status": "Solved"
            }
        ]
    }
}
```

### 5.3 创建题单

**接口描述**: 创建新的题单（需要登录）

**请求信息**:
```http
POST /api/training/create
Content-Type: application/json
```

**请求体**:
- `title`: 标题 (必填)
- `description`: 描述
- `difficulty`: 难度
- `tags`: 标签 (JSON字符串)
- `visibility`: 可见性 (public/private)

### 5.4 编辑题单

**接口描述**: 编辑现有题单（需要是作者）

**请求信息**:
```http
POST /api/training/edit
```

**请求体**:
- `id`: 题单ID (必填)
- 其他字段同创建接口

### 5.5 删除题单

**接口描述**: 删除题单（需要是作者）

**请求信息**:
```http
POST /api/training/delete
```

**请求体**:
- `id`: 题单ID (必填)

### 5.6 题单题目管理

**接口描述**: 向题单添加、移除或重新排序题目

**请求信息**:
- `POST /api/training/add_problem`: 添加单题
- `POST /api/training/add_problems`: 批量添加
- `POST /api/training/remove_problem`: 移除题目
- `POST /api/training/reorder`: 重新排序

**参数示例**:
- 单题添加: `{ "training_list_id": "1", "question_id": "100" }`
- 批量添加: `{ "training_list_id": "1", "question_ids": ["101", "102"] }`
- 重排序: `{ "training_list_id": "1", "problem_ids": ["102", "101"] }`

## 6. 社区讨论接口

### 6.1 获取讨论列表

**接口描述**: 获取讨论区文章列表

**请求信息**:
```http
GET /api/discussions
```

### 6.2 获取讨论详情

**接口描述**: 获取单篇讨论详情

**请求信息**:
```http
GET /api/discussion/{id}
```

### 6.3 发布讨论

**接口描述**: 发布新的讨论文章（需要登录）

**请求信息**:
```http
POST /api/discussion
Content-Type: application/json
```

**请求体**:
- `title`: 标题
- `content`: 内容 (Markdown)
- `question_id`: 关联题目ID (可选)

### 6.4 获取题目相关讨论

**接口描述**: 获取特定题目的讨论列表

**请求信息**:
```http
GET /api/discussions/question/{qid}
```

### 6.5 评论系统

**接口描述**: 管理内联评论和文章评论

**API列表**:
- `GET /api/inline_comments/{post_id}`: 获取内联评论
- `POST /api/inline_comment/add`: 添加内联评论 (参数: post_id, content, selected_text, parent_id)
- `POST /api/inline_comment/delete`: 删除内联评论 (参数: comment_id)
- `GET /api/article_comments/{post_id}`: 获取文章评论
- `POST /api/article_comment/add`: 添加文章评论 (参数: post_id, content)

### 6.6 上传图片

**接口描述**: 上传图片到服务器（用于讨论区）

**请求信息**:
```http
POST /api/upload_image
Content-Type: multipart/form-data
```

**表单参数**:
- `image`: 图片文件

## 7. 管理员接口

### 7.1 题目管理

**接口描述**: 管理员专用的题目管理接口

**API列表**:
- `GET /api/admin/questions`: 获取所有题目列表（含隐藏）
- `POST /api/admin/question`: 创建题目
  - 参数: title, star, description, tail (JSON用例), cpu_limit, mem_limit, status
- `POST /api/admin/question/update/{id}`: 更新题目
- `POST /api/admin/question/delete/{id}`: 删除题目

## 8. 竞赛接口

### 8.1 获取竞赛列表

**接口描述**: 获取近期竞赛列表数据的JSON格式

**请求信息**:
```http
GET /api/contests?page=1&size=5&status=upcoming
```

**请求参数**:
- `page`: 页码
- `size`: 每页数量
- `status`: 状态筛选 (upcoming/running/ended)

**响应信息**:
```json
{
    "status": 0,
    "total": 10,
    "data": [
        {
            "name": "Codeforces Round #900",
            "start_time": "2026-02-20 22:35:00",
            "end_time": "2026-02-21 00:35:00",
            "status": "upcoming",
            "link": "...",
            "source": "Codeforces"
        }
    ]
}
```

---

**文档版本**: v1.2.2  
**最后更新时间**: 2026-03-10  
**维护团队**: 在线评测系统开发团队
