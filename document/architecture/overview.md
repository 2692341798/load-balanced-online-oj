# 在线评测系统架构设计文档

## 1. 系统概述

### 1.1 项目背景
负载均衡式在线评测系统是一个分布式评测平台，旨在为编程学习者和竞赛选手提供高效、稳定的代码评测服务。系统通过多编译服务器负载均衡架构，解决了传统单体 OJ 在高并发场景下的性能瓶颈问题。

### 1.2 核心功能
- **在线代码评测**: 支持 C++, Java, Python 代码的编译、运行和自动化测试
- **负载均衡**: 多编译服务器分布式处理，智能分发任务，自动故障转移
- **题单系统**: 结构化题目集合，支持拖拽排序与进度追踪
- **社区系统**: Markdown 文章发布、行内评论、图片上传与管理
- **竞赛模块**: 支持实时竞赛排名，同步 Codeforces & LeetCode 竞赛数据
- **现代化 Web 界面**: 响应式设计，深色/浅色主题切换，MathJax 公式完美渲染

### 1.3 技术栈
- **后端**: C++11, 多线程, Socket, JSON (jsoncpp)
- **Web 框架**: httplib.h (轻量级 C++ HTTP 服务器)
- **数据库**: MySQL 8.0+, Redis (缓存层)
- **前端**: React 19, Vite, TypeScript, TailwindCSS, Shadcn UI, Zustand
- **运维**: Docker, Docker Compose, Multi-stage Build, Nginx

### 1.4 技术选型理由
- **C++ (后端)**: 评测核心对性能要求极高，且需要精确控制子进程资源（CPU、内存、系统调用），C++ 提供了最底层的系统访问能力（如 `setrlimit`, `ptrace`）。
- **httplib**: 相比庞大的 Web 框架，`httplib` header-only 且轻量，适合构建高性能的微服务接口，减少运行时开销。
- **React 19 + Vite**: 采用最新的前端技术栈，利用 Virtual DOM 和组件化开发提升用户体验；Vite 提供极速的开发服务器启动和热更新。
- **MySQL**: 关系型数据库保证了用户数据、提交记录和题目数据的强一致性，适合处理复杂的关联查询。
- **Docker**: 容器化部署确保了开发、测试和生产环境的一致性，特别是对于需要特定编译器版本的评测环境至关重要。

## 2. 整体架构

系统采用“前端 SPA + 后端 API”的分离式架构。前端作为单页应用运行在用户浏览器中，通过 RESTful API 与后端进行交互。

```mermaid
graph TD
    User[用户] -->|HTTPS| Nginx[Nginx 反向代理]
    Nginx -->|Static Files| FE[前端静态资源 (React)]
    Nginx -->|API Request| OJ[OJ 主服务器:8094]
    
    subgraph "Backend Services"
        OJ -->|Load Balance| CS1[编译服务器1:8081]
        OJ -->|Load Balance| CS2[编译服务器2:8082]
        OJ -->|Load Balance| CS3[编译服务器3:8083]
        OJ -->|SQL| DB[(MySQL 数据库)]
        OJ -->|Cache| Redis[(Redis 缓存)]
    end
    
    Crawler["C++ 爬虫服务"] -->|SQL| DB
```

### 2.1 分层视图 (Layered View)
系统架构遵循清晰的分层原则，确保各模块职责单一，易于维护和扩展。

1.  **表现层 (Presentation Layer)**
    *   **React Frontend**: 负责 UI 渲染、用户交互、路由管理。
    *   **State Management (Zustand)**: 管理会话状态、题目数据等前端全局状态。
    *   **API Client (Axios)**: 封装 HTTP 请求，处理统一的错误响应和拦截。

2.  **网关/控制层 (Controller Layer)**
    *   **OJ Server (httplib)**: 处理 HTTP 请求路由，执行身份验证 (Session/JWT)，参数校验。
    *   **Middleware**: 负责日志记录、跨域处理 (CORS)、安全过滤。

3.  **业务逻辑层 (Business Logic Layer)**
    *   **OJ Service**: 核心业务逻辑，包括题目管理、竞赛逻辑、排行榜计算。
    *   **Judge Service**: 评测任务调度，负载均衡策略实现。
    *   **Compile Service**: 具体的编译、运行、结果比对逻辑。

4.  **数据访问层 (Data Access Layer)**
    *   **Model**: 封装 SQL 语句，提供面向对象的数据操作接口。
    *   **Connection Pool**: 管理数据库连接，提高并发性能。

5.  **基础设施层 (Infrastructure Layer)**
    *   **OS/Hardware**: Linux Kernel (提供 cgroups, namespaces 支持)。
    *   **Container**: Docker 容器环境，隔离评测沙箱。

## 3. 核心模块设计

### 3.1 前端应用 (frontend)
- **Framework**: React 19 + Vite + TypeScript，采用 SPA (单页应用) 架构。
- **UI Components**: 
    - **Shadcn UI**: 基于 Radix UI 的无头组件库，提供高质量的交互体验。
    - **TailwindCSS**: 原子化 CSS 框架，实现快速响应式布局。
    - **Lucide React**: 统一的图标库。
- **State Management**: **Zustand**，管理用户会话 (`auth.ts`)、题目列表等全局状态，相比 Redux 更轻量高效。
- **Routing**: **React Router v7**，管理页面路由与权限控制 (`ProtectedRoute`)。
- **Network**: **Axios** 封装 (`src/lib/axios.ts`)，统一处理请求拦截、响应错误 (401/500) 和超时。
- **Editor**: **Monaco Editor** (`@monaco-editor/react`)，提供类 VS Code 的代码编辑体验。
- **Build**: 构建产物 (HTML/JS/CSS) 部署在 `backend/oj_server/wwwroot` 目录，由后端静态文件服务提供。

**组件交互流程**:
1. **View (Page/Component)**: 用户触发操作 (如点击登录)。
2. **Store (Zustand)**: 调用 Action (如 `login`)。
3. **Service (API Layer)**: `authService` 调用 `axios` 发送 HTTP 请求。
4. **Backend**: 处理请求并返回 JSON。
5. **Store**: 更新状态 (`user`, `isAuthenticated`)。
6. **View**: 自动重新渲染 UI。

### 3.2 OJ 主服务器 (backend/oj_server)
- **Control**: 核心控制器，处理 API 请求（认证、题目、评测分发、题单、讨论）。
- **Model**: 数据访问层，封装 MySQL 操作。
- **View**: 静态资源服务层，不再负责 HTML 模板渲染，而是返回 JSON 数据或静态文件。
- **LoadBalance**: 负载均衡器，维护编译服务器在线状态，按最小负载算法分发。
- **Session**: 内存会话管理，支持 24 小时过期。

### 3.3 编译服务器 (backend/compile_server)
- **Compiler**: 编译器封装（g++, javac）。
- **Runner**: 运行器，使用 `setrlimit` 进行资源限制（CPU, 内存）。
- **CompileRun**: 核心流程，处理临时文件生成、编译、多测试用例运行、结果收集。

### 3.4 爬虫模块 (backend/crawler)
- **Contest Crawler**: 定期抓取 Codeforces (API) 和 LeetCode (GraphQL) 数据。
- **Luogu Crawler**: 抓取洛谷题目详情。
- **技术**: C++, libcurl, jsoncpp。

## 4. 核心流程

### 4.1 代码提交流程
1. 用户在 React 前端提交代码和语言选择（调用 `/api/judge`）。
2. `Control::Judge` 接收请求，获取题目测试用例 (JSON)。
3. `LoadBalance` 选择最优编译服务器。
4. 主服务器通过 HTTP 将代码、输入和限制发送至编译服务器。
5. 编译服务器针对每个测试用例执行编译运行，对比结果。
6. 返回聚合后的结果 JSON（Accepted, Wrong Answer 等）。
7. 主服务器记录提交历史并返回 JSON 响应，前端实时更新界面。

### 4.2 负载均衡算法
采用“最小负载优先”算法：
- 每次分发前，遍历所有 `online` 服务器，选择 `load` 值最小的一台。
- 若请求失败，自动将服务器移入 `offline` 列表并尝试分发给下一台。
- 离线服务器可通过信号 (`SIGQUIT`) 或健康检查手动/自动恢复。

## 5. 安全设计
- **代码沙箱**: 编译服务器降权运行，严格限制文件系统访问。
- **资源熔断**: 强制限制 CPU 时间和内存空间，防止恶意代码耗尽资源。
- **认证安全**: 密码 SHA256 哈希存储，Cookie 设置 `HttpOnly`。
- **XSS 防护**: 渲染 Markdown 前使用 `DOMPurify` 进行过滤。

## 6. 质量属性 (Quality Attributes)

### 6.1 可用性 (Availability)
- **冗余设计**: 编译服务集群部署，单节点故障不影响整体评测功能。
- **故障恢复**: 负载均衡器具备自动剔除故障节点和重试机制，确保用户请求尽可能成功。

### 6.2 性能 (Performance)
- **并发处理**: 编译服务器支持多线程并发编译，充分利用多核 CPU。
- **缓存优化**: 热点数据（如题目详情）可利用 Redis 缓存（规划中），减少数据库压力。
- **前端优化**: 静态资源 CDN 加速（生产环境），代码分割 (Code Splitting) 减少首屏加载时间。

### 6.3 可维护性 (Maintainability)
- **模块化**: 清晰的前后端分离，模块间通过标准 API 耦合。
- **配置化**: 关键参数（端口、数据库连接、资源限制）均可通过配置文件或环境变量调整。
- **统一规范**: 前端遵循 ESLint/Prettier 规范，后端遵循 Google C++ Style Guide。

### 6.4 安全性 (Security)
- **最小权限原则**: 评测进程以低权限用户 `nobody` 运行。
- **数据隐私**: 用户密码加密存储，敏感操作需身份验证。

## 7. 部署架构
- **Docker 化**: 所有组件（MySQL, OJ, CompileServer, Crawler）均支持容器化。
- **网络**: 使用 Docker 内网隔离，仅暴露 8094 端口。
- **持久化**: 使用 Docker Volumes 存储数据库数据和上传的资源。

## 8. 版本快照 (Version Snapshots)

| 版本号 | 发布日期 | 关键特性 | 架构变更 |
| :--- | :--- | :--- | :--- |
| **v0.1** | 2025-10 | 基础评测功能 | 单体架构，本地编译运行 |
| **v0.5** | 2025-11 | 分离编译服务 | C/S 架构，引入 HTTP 通信 |
| **v0.8** | 2025-12 | 引入数据库 | 增加 MySQL 持久化，用户系统 |
| **v1.0** | 2026-01 | 前端重构 | 引入 React SPA，前后端完全分离 |
| **v1.1** | 2026-02 | 负载均衡 | 多编译服务器集群，自动发现与故障转移 |
| **v1.2** | 2026-03 | 社区与竞赛 | 增加题单、评论、竞赛爬虫模块 |

---

**文档版本**: v1.3.0
**最后更新时间**: 2026-03-12
**维护团队**: 在线评测系统开发团队
