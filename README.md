# 负载均衡式在线评测系统（Load-Balanced Online Judge）

> 一个基于 C++ 的分布式 Online Judge，支持多编译服务器负载均衡、用户认证和现代化的Web界面。

## 🚀 项目概述

这是一个基于C++开发的负载均衡式在线评测系统（Online Judge），专为算法竞赛和编程练习设计。系统采用分布式架构，支持多编译服务器负载均衡，能够高效处理代码编译、运行和评测任务。

### ✨ 主要功能
- **在线代码评测**：支持C/C++代码的编译、运行和自动化测试
- **负载均衡**：多编译服务器分布式处理，提高系统吞吐量
- **用户认证系统**：完整的用户注册、登录和会话管理
- **题目管理**：支持算法题目的创建、编辑和分类管理
- **现代化Web界面**：响应式设计，支持深色主题
- **实时评测**：快速反馈代码运行结果和测试用例通过情况
- **错误处理**：完善的异常捕获和用户友好的错误提示
- **竞赛爬虫 (Contest Crawler)**：自动获取Codeforces竞赛信息，支持自适应速率限制和robots.txt合规
- **社区讨论 (Community & Discussions)**：支持Markdown帖子、全局评论和行内评论功能

### 🏗️ 架构设计
- **OJ主服务器（oj_server）**：提供Web接口、题目管理、评测任务调度与结果汇总
- **编译服务器池（compile_server）**：对提交代码进行编译和运行，主服务器按负载将任务分发到不同实例
- **数据库（MySQL）**：存储题目、用户、提交记录等数据
- **前端界面**：现代化的HTML/CSS/JS界面，支持响应式设计
- **负载均衡器**：智能选择负载最低的编译服务器处理请求
- **会话管理**：基于Token的用户认证和会话维护

### 🛠️ 核心技术栈
- **后端**：C++11, 多线程编程, Socket网络编程, JSON处理
- **Web框架**：httplib.h（轻量级C++ HTTP服务器）
- **数据库**：MySQL 8.0+
- **模板引擎**：CTemplate
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
- **httplib**：HTTP服务器库（已包含在项目中）
- **openssl**：加密库（用于密码哈希）

### 安装依赖（Ubuntu/Debian）
```bash
sudo apt-get update
sudo apt-get install -y build-essential libjsoncpp-dev libctemplate-dev libmysqlclient-dev mysql-server libssl-dev
```

### 安装依赖（macOS）
```bash
brew install jsoncpp ctemplate mysql openssl
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

# 启动OJ主服务器（默认端口8080）
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

### 6. 访问系统
- **主页**：http://localhost:8080
- **题目列表**：http://localhost:8080/all_questions
- **题目详情**：http://localhost:8080/question/<题号>
- **登录页面**：http://localhost:8080/login

### 7. 基本操作
1. **用户注册**：在登录页面点击注册，填写用户名、密码和邮箱
2. **用户登录**：使用注册的用户名和密码登录
3. **浏览题目**：访问题目列表页面查看所有可用题目
4. **提交代码**：在题目详情页面编写或粘贴代码
5. **查看结果**：系统会自动编译运行代码并显示评测结果

## 📊 评测结果说明
- **AC（Accepted）**：通过所有测试用例
- **WA（Wrong Answer）**：输出结果错误
- **CE（Compile Error）**：编译失败
- **RE（Runtime Error）**：运行时错误（如越界、段错误）
- **TLE（Time Limit Exceeded）**：超时
- **MLE（Memory Limit Exceeded）**：内存超限

## 🏗️ 项目结构

```
load-balanced-online-oj/
├── comm/                           # 公共组件
│   ├── httplib.h                  # HTTP服务器库
│   ├── log.hpp                    # 日志系统
│   └── util.hpp                   # 工具函数
├── crawler/                       # 竞赛爬虫
│   ├── contest_crawler.cc        # 爬虫主程序
│   ├── crawler_common.hpp        # 爬虫公共头文件
│   └── makefile                  # 编译配置
├── compile_server/                # 编译服务器
│   ├── compile_server.cc         # 主程序
│   ├── compile_run.hpp           # 编译运行核心
│   ├── compiler.hpp              # 编译器封装
│   ├── runner.hpp                # 程序运行器
│   └── makefile                  # 编译配置
├── oj_server/                     # OJ主服务器
│   ├── oj_server.cc              # 主程序
│   ├── oj_control.hpp            # 业务逻辑控制
│   ├── oj_model2.hpp             # 数据模型（MySQL版）
│   ├── oj_view.hpp               # 视图渲染
│   ├── conf/                     # 配置文件
│   │   └── service_machine.conf  # 编译服务器列表
│   ├── css/                      # 样式文件
│   │   ├── index.css             # 主页样式
│   │   ├── all_questions.css     # 题目列表样式
│   │   ├── one_question.css      # 题目详情样式
│   │   ├── login.css             # 登录页面样式
│   │   └── profile.css           # 个人中心样式
│   ├── template_html/            # HTML模板
│   │   ├── all_questions.html    # 题目列表模板
│   │   ├── one_question.html     # 题目详情模板
│   │   ├── login.html            # 登录页面模板
│   │   └── profile.html          # 个人中心模板
│   ├── wwwroot/                  # 静态资源
│   │   └── index.html            # 主页
│   └── makefile                  # 编译配置
├── makefile                       # 主编译文件
├── setup_database.sql            # 数据库初始化脚本
├── .gitignore                    # Git忽略规则
├── LICENSE                       # 许可证文件
└── README.md                     # 项目说明
```

## 🔧 配置说明

### 编译服务器配置
编辑 `oj_server/conf/service_machine.conf` 文件，每行配置一个编译服务器：
```
IP地址:端口号
```

### 数据库配置
数据库连接信息在 `oj_server/oj_model2.hpp` 中配置：
```cpp
const std::string host = "127.0.0.1";
const std::string user = "oj_client";
const std::string passwd = "123456";
const std::string db = "oj";
const int port = 3306;
```

## 🧪 测试

### 端到端测试
项目使用Playwright进行端到端测试：
```bash
# 安装Playwright依赖
npm install @playwright/test

# 运行测试
npx playwright test
```

### 单元测试
可以通过访问API接口进行功能测试：
```bash
# 测试用户注册
curl -X POST http://localhost:8080/api/register \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"testpass123","email":"test@example.com"}'

# 测试用户登录
curl -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"testpass123"}'

# 测试代码评测
curl -X POST http://localhost:8080/judge/1 \
  -H "Content-Type: application/json" \
  -d '{"code":"#include<iostream>\nint main(){std::cout<<\"Hello World\"<<std::endl;return 0;}"}'
```

## 🐛 常见问题

### 端口占用
若8080/8081/8082/8083端口被占用，请：
1. 调整启动端口参数
2. 同步更新配置文件 `service_machine.conf`
3. 重启服务

### 数据库连接失败
1. 确认MySQL已启动：`sudo service mysql start`
2. 检查账号密码是否正确
3. 确认初始化脚本已执行：`mysql -u root -p < setup_database.sql`
4. 检查数据库用户权限

### 编译服务器不可达
1. 检查 `service_machine.conf` 地址是否正确
2. 确认编译服务器实例已启动
3. 检查网络连接和防火墙设置
4. 查看服务器日志排查问题

### 权限问题（macOS）
```bash
chmod +x oj_server/oj_server compile_server/compile_server
```

### 编译错误
1. 确认所有依赖已安装
2. 检查C++编译器版本
3. 查看详细的编译错误信息
4. 确保MySQL开发库已安装

## 🤝 贡献指南

### 开发流程
1. **Fork项目**：创建个人分支进行开发
2. **创建功能分支**：`git checkout -b feature/your-feature-name`
3. **编写代码**：遵循项目编码规范
4. **测试验证**：确保功能正常且不影响现有功能
5. **提交代码**：`git commit -m "feat: 添加新功能描述"`
6. **推送分支**：`git push origin feature/your-feature-name`
7. **创建Pull Request**：描述清楚修改内容和原因

### 代码规范
- **命名规范**：使用有意义的英文全拼，循环变量可用i/j/k
- **代码结构**：保持函数单一职责，相关代码就近组织
- **注释规范**：注释说明"为什么"而不是"做什么"
- **错误处理**：完善的错误检查和异常处理
- **日志记录**：关键操作需要添加日志记录
- **安全规范**：不暴露敏感信息，参数验证完整

### 提交规范
- **feat**: 新功能
- **fix**: 修复bug
- **docs**: 文档更新
- **style**: 代码格式调整
- **refactor**: 代码重构
- **test**: 测试相关
- **chore**: 构建过程或辅助工具的变动

### 测试要求
- 新功能必须包含相应的测试用例
- 修复bug需要添加回归测试
- 所有测试用例必须通过
- 代码覆盖率应保持或提高

## 📄 许可证信息

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

感谢所有为项目做出贡献的开发者和使用者。

---

**最后更新时间**: 2026-01-31  
**文档版本**: v0.4.0  
**维护团队**: 在线评测系统开发团队