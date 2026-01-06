# 负载均衡式在线评测系统（Load-Balanced Online Judge）

> 一个基于 C++ 的分布式 Online Judge，支持多编译服务器负载均衡，高效完成代码编译、运行与评测任务。

## 项目概述

这是一个基于C++开发的负载均衡式在线评测系统（Online Judge），专为算法竞赛和编程练习设计。系统采用分布式架构，支持多编译服务器负载均衡，能够高效处理代码编译、运行和评测任务。

### 主要功能
- **在线代码评测**：支持C/C++代码的编译、运行和自动化测试
- **负载均衡**：多编译服务器分布式处理，提高系统吞吐量
- **题目管理**：支持算法题目的创建、编辑和分类管理
- **Web界面**：提供用户友好的Web界面进行题目浏览和代码提交
- **实时评测**：快速反馈代码运行结果和测试用例通过情况

### 架构设计
- **OJ主服务器（oj_server）**：提供 Web 接口、题目管理、评测任务调度与结果汇总
- **编译服务器池（compile_server）**：对提交代码进行编译和运行，主服务器按负载将任务分发到不同实例
- **数据库（MySQL）**：存储题目、提交记录等数据
- **模板与静态资源**：使用 CTemplate 渲染页面，wwwroot 提供静态文件
- **配置文件**：通过 `oj_server/conf/service_machine.conf` 管理编译服务器列表（IP:PORT）

### 核心技术栈
- **后端**：C++11, 多线程编程, Socket网络编程
- **Web框架**：httplib.h（轻量级C++ HTTP服务器）
- **数据库**：MySQL 8.0+
- **模板引擎**：CTemplate
- **编译器**：GCC/G++
- **构建工具**：GNU Make
- **操作系统**：Linux/macOS

## 环境要求

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

### 安装依赖（Ubuntu/Debian）
```bash
sudo apt-get update
sudo apt-get install -y build-essential libjsoncpp-dev libctemplate-dev libmysqlclient-dev mysql-server
```

### 安装依赖（macOS）
```bash
brew install jsoncpp ctemplate mysql
```

## 安装指南

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

### 5. 输出部署文件
```bash
make output
```

## 使用说明

### 启动服务

#### 方式1：分别启动（推荐开发环境）
```bash
# 启动第一个编译服务器（端口8081）
cd compile_server && ./compile_server 8081

# 启动第二个编译服务器（端口8082）
cd compile_server && ./compile_server 8082

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

### 访问系统
- **主页**：http://localhost:8080
- **题目列表**：http://localhost:8080/all_questions
- **题目详情**：http://localhost:8080/question/<题号>

### 基本操作
1. **浏览题目**：访问题目列表页面查看所有可用题目
2. **提交代码**：在题目详情页面编写或粘贴代码
3. **查看结果**：系统会自动编译运行代码并显示评测结果

### 评测结果说明
- **AC（Accepted）**：通过所有测试用例
- **WA（Wrong Answer）**：输出结果错误
- **CE（Compile Error）**：编译失败
- **RE（Runtime Error）**：运行时错误（如越界、段错误）
- **TLE（Time Limit Exceeded）**：超时
- **MLE（Memory Limit Exceeded）**：内存超限

## 项目结构

```
load-balanced-online-oj/
├── comm/                    # 公共组件
│   ├── httplib.h           # HTTP服务器库
│   ├── log.hpp             # 日志系统
│   └── util.hpp            # 工具函数
├── compile_server/          # 编译服务器
│   ├── compile_server.cc   # 主程序
│   ├── compile_run.hpp     # 编译运行核心
│   ├── compiler.hpp        # 编译器封装
│   ├── runner.hpp          # 程序运行器
│   └── makefile            # 编译配置
├── oj_server/              # OJ主服务器
│   ├── oj_server.cc        # 主程序
│   ├── oj_control.hpp      # 业务逻辑控制
│   ├── oj_model2.hpp       # 数据模型（MySQL版）
│   ├── oj_view.hpp         # 视图渲染
│   ├── conf/               # 配置文件
│   │   └── service_machine.conf  # 编译服务器列表
│   ├── questions/          # 题目数据（文件版）
│   ├── template_html/      # HTML模板
│   └── wwwroot/            # 静态资源
├── output/                  # 编译输出（自动生成）
├── document/               # 项目文档
├── makefile                # 主编译文件
└── setup_database.sql      # 数据库初始化脚本
```

## 常见问题
- **端口占用**：若 8080/8081/8082 被占用，请调整启动端口并同步更新配置文件
- **数据库连接失败**：确认 MySQL 已启动，账号密码正确，且初始化脚本执行成功
- **编译服务器不可达**：检查 `service_machine.conf` 地址是否正确、实例是否已启动、网络是否可达
- **权限问题**：macOS 可能需要为可执行文件添加权限：`chmod +x oj_server/oj_server compile_server/compile_server`

## 贡献指南

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

### 提交规范
- **feat**: 新功能
- **fix**: 修复bug
- **docs**: 文档更新
- **style**: 代码格式调整
- **refactor**: 代码重构
- **test**: 测试相关
- **chore**: 构建过程或辅助工具的变动

## 许可证信息

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系方式

如有问题或建议，欢迎通过以下方式联系：
- **Issues**：在项目GitHub页面提交Issue
- **Pull Request**：欢迎提交改进和修复

## 致谢

感谢所有为项目做出贡献的开发者和使用者。
