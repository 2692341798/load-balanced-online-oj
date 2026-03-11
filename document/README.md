# 📚 冻梨 OJ 文档中心

欢迎查阅冻梨 OJ (Load Balanced Online OJ) 的官方文档。本文档中心旨在为开发者、运维人员和贡献者提供全面、清晰的指导。

## 📂 文档结构

为了方便查阅，我们将文档按功能进行了分类整理：

### 🏗️ 架构与设计 (`architecture/`)
- [**系统概览**](architecture/overview.md): 整体架构设计、模块划分及交互流程。
- [**数据库设计**](architecture/database.md): 数据库表结构、ER 图及字段说明。

### 🔌 API 与接口 (`api/`)
- [**API 参考手册**](api/reference.md): 后端 RESTful API 的详细说明、请求参数与响应格式。

### 🚀 部署与运维 (`deployment/`)
- [**阿里云 Docker 部署**](deployment/aliyun.md): 基于 Docker 的全栈部署指南。
- [**竞赛模块部署**](deployment/contest_module.md): 竞赛爬虫及相关模块的特定部署说明。

### 📏 开发规范 (`standards/`)
- [**后端开发规范**](standards/development.md): C++ 代码风格、提交规范及最佳实践。
- [**前端样式指南**](standards/frontend.md): 前端 UI 设计原则、CSS 变量及组件使用指南。

### 📖 操作手册 (`manuals/`)
- [**维护手册**](manuals/maintenance.md): 系统日常维护、故障排查及常见问题解答。

### 📝 日志与报告 (`logs/`, `reports/`)
- [**开发日志**](logs/dev_log.md): 项目开发过程的详细记录与里程碑。
- [**Bug 修复记录**](logs/bug_fixes/): 针对特定 Bug 的分析与修复报告。
- [**清理报告**](reports/): 项目维护过程中的文件清理与重构记录。

## 🤝 如何贡献文档

1.  **Fork** 本仓库。
2.  在 `document/` 目录下找到对应分类，进行修改或添加新文档。
3.  遵循 [Markdown 编写规范](standards/development.md#文档规范)。
4.  提交 Pull Request。

---
**最后更新**: 2026-03-11
**维护团队**: 在线评测系统开发团队
