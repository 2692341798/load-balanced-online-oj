# 负载均衡式在线评测系统（Load-Balanced Online Judge）

> 一个基于 C++ 的分布式 Online Judge，支持多编译服务器负载均衡、用户认证和现代化的Web界面。

## 🚀 项目概述

这是一个基于C++开发的负载均衡式在线评测系统（Online Judge），专为算法竞赛和编程练习设计。系统采用分布式架构，支持多编译服务器负载均衡，能够高效处理代码编译、运行和评测任务。

### ✨ 主要功能
- **在线代码评测**：支持C/C++代码的编译、运行和自动化测试
- **负载均衡**：多编译服务器分布式处理，提高系统吞吐量
- **用户认证系统**：完整的用户注册、登录和会话管理
- **题单系统 (Training Lists)**：支持创建、分享和管理题目集合，提供拖拽排序和难度分级功能
- **题目管理**：支持算法题目的创建、编辑和分类管理
- **现代化Web界面**：响应式设计，支持深色主题
- **实时评测**：快速反馈代码运行结果和测试用例通过情况
- **错误处理**：完善的异常捕获和用户友好的错误提示
- **安全加固**：基于Linux Namespace的网络隔离、权限降级(nobody)和严格的资源熔断机制，有效防御Sandbox Escape和DoS攻击
- **竞赛爬虫 (Contest Crawler)**：基于C++的高性能爬虫，自动获取Codeforces/LeetCode竞赛信息，支持自适应速率限制和MySQL存储
- **社区讨论 (Community & Discussions)**：支持Markdown帖子、全局评论和行内评论功能
- **数学公式支持**：集成 MathJax，完美渲染 LaTeX 数学公式
- **娱乐中心 (Beta)**：集成经典游戏（如超级玛丽重制版），为用户提供放松的休闲角落
- **用户中心增强**：支持头像上传、个人资料修改（昵称/电话）等个性化设置

### 🏗️ 架构设计
- **OJ主服务器（oj_server）**：提供Web接口、题目管理、评测任务调度与结果汇总
- **编译服务器池（compile_server）**：对提交代码进行编译和运行，主服务器按负载将任务分发到不同实例
- **数据库（MySQL）**：存储题目、用户、提交记录、竞赛信息等数据
- **前端界面**：现代化的HTML/CSS/JS界面，支持响应式设计
- **负载均衡器**：智能选择负载最低的编译服务器处理请求
- **会话管理**：基于Token的用户认证和会话维护

### 🛠️ 核心技术栈
- **后端**：C++11, 多线程编程, Socket网络编程, JSON处理
- **Web框架**：httplib.h（轻量级C++ HTTP服务器）
- **数据库**：MySQL 8.0+
- **模板引擎**：CTemplate
- **网络库**：libcurl (用于爬虫模块)
- **编译器**：GCC/G++
- **构建工具**：GNU Make
- **操作系统**：Linux/macOS
- **测试框架**：Playwright（端到端测试）

## 📚 详细文档

本项目包含完整的技术文档，请参考以下指南：

| 文档类型 | 文件名称 | 描述 |
|---------|---------|------|
| 🏗️ **架构设计** | [架构设计文档](document/architecture.md) | 系统整体架构、核心模块设计与通信机制 |
| 💾 **数据库** | [数据库设计](document/database.md) | 数据库表结构、索引策略与ER关系 |
| 🔌 **接口规范** | [API接口文档](document/api_reference.md) | 详细的后端接口定义与调用示例 |
| 💻 **开发规范** | [开发标准规范](document/development_standards.md) | 代码风格、命名规范与最佳实践 |
| 🤝 **协作规则** | [项目开发规则](document/rule.md) | 团队协作、安全规范与工作流 |
| 🎨 **前端指南** | [前端样式指南](document/frontend_style_guide.md) | 视觉设计系统、组件库与CSS变量 |
| 📝 **变更日志** | [综合开发日志](document/comprehensive_dev_log.md) | 版本迭代记录与技术变更日志 |

## 📋 环境要求

### 开发环境
- **操作系统**：Linux (Ubuntu 18.04+) 或 macOS (10.14+)
- **编译器**：GCC 7.0+ 或 Clang 8.0+
- **C++标准**：C++11或更高版本
- **MySQL**：8.0或更高版本
- **Git**：用于版本控制

### 依赖项
- **jsoncpp**：JSON解析库
- **pthread**：POSIX线程库
- **ctemplate**：HTML模板引擎
- **mysqlclient**：MySQL客户端库
- **http_server**：基于 cpp-httplib 的高性能 Web 服务器
- **libcurl**：高性能网络请求库 (用于C++爬虫)

### 安装依赖（Ubuntu/Debian）
```bash
sudo apt-get update
sudo apt-get install -y build-essential libjsoncpp-dev libctemplate-dev libmysqlclient-dev mysql-server libssl-dev libcurl4-openssl-dev
```

### 安装依赖（macOS）
```bash
brew install jsoncpp ctemplate mysql openssl curl
```

## 🚀 快速开始

### 1. 克隆项目
```bash
git clone https://github.com/2692341798/load-balanced-online-oj.git
cd load-balanced-online-oj
```

### 2. 数据库设置
```bash
# 登录MySQL
mysql -u root -p

# 执行数据库初始化脚本
source setup_database.sql
# 如果需要竞赛数据支持，请确保执行了最新的迁移脚本
```

### 3. 配置编译服务器
编辑 `oj_server/conf/service_machine.conf`，按行写入可用编译服务器地址：
```
127.0.0.1:8081
127.0.0.1:8082
127.0.0.1:8083
```
端口与数量可根据实际机器资源调整。

### 4. 编译项目
```bash
# 编译所有组件
make

# 或者分别编译
make -C compile_server
make -C oj_server
make -C crawler
```

### 5. 启动服务

#### 方式1：分别启动（推荐开发环境）
```bash
# 启动第一个编译服务器（端口8081）
cd compile_server && ./compile_server 8081

# 启动第二个编译服务器（端口8082）
cd compile_server && ./compile_server 8082

# 启动第三个编译服务器（端口8083）
cd compile_server && ./compile_server 8083

# 启动OJ主服务器（默认端口8088）
cd oj_server && ./oj_server
```

#### 方式2：使用输出目录
```bash
# 进入输出目录
cd output

# 启动编译服务器
./compile_server/compile_server 8081
./compile_server/compile_server 8082

# 启动OJ服务器
cd oj_server && ./oj_server
```

### 6. 爬虫服务 (可选)
如果需要获取竞赛数据或题目，可以运行爬虫服务（确保使用C++二进制文件）：
```bash
# 运行竞赛爬虫 (Codeforces/LeetCode)
./crawler/contest_crawler

# 运行洛谷题目爬虫 (C++实现)
./crawler/luogu_crawler
```

### 7. 访问系统
- **主页**：http://localhost:8088
- **题目列表**：http://localhost:8088/all_questions
- **题目详情**：http://localhost:8088/question/<题号>
- **登录页面**：http://localhost:8088/login
- **娱乐中心**：http://localhost:8088/entertainment (或侧边栏入口)

## 🏗️ 项目结构

```
load-balanced-online-oj/
├── comm/                           # 公共组件
│   ├── httplib.h                  # HTTP服务器库
│   ├── log.hpp                    # 日志系统
│   └── util.hpp                   # 工具函数
├── compile_server/                # 编译服务器
│   ├── compile_server.cc         # 主程序
│   ├── compile_run.hpp           # 编译运行核心
│   ├── compiler.hpp              # 编译器封装
│   ├── runner.hpp                # 程序运行器
│   └── makefile                  # 编译配置
├── crawler/                       # 竞赛爬虫
│   ├── contest_crawler.cc        # 竞赛爬虫主程序 (核心)
│   ├── luogu_crawler.cc          # 洛谷题目爬虫
│   ├── crawler_common.hpp        # 爬虫公共头文件
│   └── makefile                  # 编译配置
├── oj_server/                     # OJ主服务器
│   ├── oj_server.cc              # 主程序
│   ├── oj_control.hpp            # 业务逻辑控制
│   ├── oj_model.hpp              # 数据模型（MySQL版）
│   ├── oj_view.hpp               # 视图渲染
│   ├── contest_utils.hpp         # 竞赛工具类
│   ├── conf/                     # 配置文件
│   │   └── service_machine.conf  # 编译服务器列表
│   ├── resources/                # 静态资源与模板
│   │   ├── css/                  # 样式文件
│   │   ├── template_html/        # HTML模板
│   │   └── wwwroot/              # 静态资源(JS/Images)
│   │       └── games/            # 娱乐中心游戏资源
│   └── makefile                  # 编译配置
├── document/                      # 项目文档
│   ├── architecture.md           # 架构设计
│   ├── database.md               # 数据库设计
│   └── ...                       # 其他文档
├── tests/                         # 测试代码
│   ├── oj_server/                # OJ服务测试
│   └── crawler/                  # 爬虫测试
├── docker/                        # Docker部署配置
│   ├── Dockerfile.oj
│   ├── Dockerfile.compile
│   └── docker-compose.yml
├── scripts/                       # 运维脚本
│   ├── start.sh                  # 一键启动
│   ├── stop.sh                   # 一键停止
│   └── deploy_docker.sh          # Docker部署脚本
├── sql/                           # 数据库脚本
│   └── setup_database.sql        # 数据库初始化
├── data/                          # 数据文件
│   └── contests.json             # 竞赛数据缓存
├── makefile                       # 主编译文件
├── .gitignore                    # Git忽略规则
├── LICENSE                       # 许可证文件
└── README.md                     # 项目说明
```

## 🎮 娱乐中心 (Beta)

### 1. 经典游戏复刻
- **超级玛丽 (Super Mario Remake)**: 基于 Backbone Game Engine 的 HTML5 完美复刻版。
- **特性**: 
  - 经典的 1-1 关卡体验
  - 还原度极高的物理引擎与音效
  - 适配现代浏览器的响应式显示
  - 键盘操作支持 (方向键移动/跳跃)

### 2. 访问方式
- 点击侧边栏 "娱乐中心" 即可进入。
- 旨在为刷题疲惫的用户提供放松的角落。

## 🕷️ 竞赛爬虫模块详情

本模块负责从竞争性编程平台（目前支持 Codeforces 和 LeetCode）抓取竞赛信息并存储到数据库中。

### ✨ 主要特性
- **多源支持**：抓取 Codeforces 和 LeetCode 的周赛信息。
- **Robots.txt 合规**：严格遵守目标站点的 `robots.txt` 规则。
- **数据持久化**：将竞赛数据存储在 MySQL 中。
- **C++原生实现**：高性能，无Python依赖。

## 🤝 贡献指南

### 开发流程
1. **Fork项目**：创建个人分支进行开发
2. **分支命名规范**：
   - `feat/`: 新功能 (e.g., `feat/avatar-upload`)
   - `fix/`: 修复Bug (e.g., `fix/login-error`)
   - `docs/`: 文档更新
   - `refactor/`: 代码重构
3. **提交代码**：
   - **Commit Message**: `type: description` (e.g., `feat: add avatar upload`)
4. **创建Pull Request**：描述清楚修改内容和原因

### 代码审查清单 (Code Review Checklist)
- [ ] **逻辑正确性**：代码逻辑清晰，无明显Bug
- [ ] **风格规范**：遵循项目代码风格（命名、注释、格式）
- [ ] **安全性检查**：无SQL注入、XSS等安全隐患，不暴露敏感信息
- [ ] **测试覆盖**：包含必要的单元测试或功能验证

## 📅 版本历史

- **v1.1.2** (2026-03-04):
  - 🎨 视觉体验升级：优化题单与讨论区卡片样式，提升对比度与层次感
  - 💄 样式修复：解决讨论区摘要显示问题，统一深色主题设计语言
- **v1.1.1** (2026-03-04):
  - 🐛 修复用户头像同步问题：优化多会话状态下的用户信息更新逻辑
- **v1.1.0** (2026-02-22):
  - ✨ 新增题单/训练计划模块：支持创建、分享、管理题目集合
  - 🔄 题单题目拖拽排序功能
  - 📝 完善数据库与API文档
- **v1.0.5** (2026-02-16):
  - 🔧 项目结构优化：整合爬虫模块至 `crawler/` 目录
- **v1.0.4** (2026-02-15):
  - ✨ 新增娱乐中心 (Beta)，集成超级玛丽复刻版
  - 👤 用户中心增强：支持头像上传、个人资料修改
  - 📄 分页功能优化
  - 🔧 项目结构重构：分离测试与工具代码
    - 测试代码迁移至 `tests/`
    - 爬虫工具迁移至 `tools/crawler/`
    - 清理构建产物与冗余文件
- **v1.0.3**:
  - 🕷️ 引入C++竞赛爬虫 (Codeforces/LeetCode)
  - 🔧 统一服务端口为 8088
- **v0.3.7-fun**:
  - 🎮 娱乐中心初步集成

## 📄 许可证信息

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

感谢所有为项目做出贡献的开发者和使用者。

---

**最后更新时间**: 2026-03-04  
**文档版本**: v1.1.2  
**维护团队**: 在线评测系统开发团队
