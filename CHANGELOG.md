# 修复日志 (Fix Log) - 2026-01-08

## 1. 样式重构 (Style Refactoring)
- **目标**: 统一前端页面风格，匹配 `index.css` 和 `index.html` 的设计规范。
- **修改文件**:
    - `oj_server/css/all_questions.css`: 移除冗余 reset 样式，引入 CSS 变量 (`--bg-color`, `--card-bg` 等)，适配深色主题，优化题目列表表格样式。
    - `oj_server/css/one_question.css`: 适配深色主题，优化题目详情页布局，统一代码编辑器区域样式。
    - `oj_server/css/login.css`: 适配深色主题，统一登录/注册卡片风格，优化按钮交互样式。

## 2. 模板结构统一 (Template Structure Unification)
- **目标**: 确保所有 HTML 页面拥有通过的 DOM 结构和组件。
- **修改文件**:
    - `oj_server/template_html/all_questions.html`: 引入 `index.css`，重写 Navbar 结构以保持一致，添加 XSS 防护函数 `escapeHtml`。
    - `oj_server/template_html/one_question.html`: 引入 `index.css`，重写 Navbar，集成 ACE 编辑器并适配样式。
    - `oj_server/template_html/login.html`: 引入 `index.css`，重写 Navbar，优化表单结构。

## 3. 服务器配置修复 (Server Configuration Fix)
- **问题**: 静态资源 (CSS) 加载失败，导致页面样式丢失。
- **修复**: 修改 `oj_server/oj_server.cc`，移除手动实现的 CSS 路由，使用 `svr.set_mount_point("/css", "./css")` 正确挂载 CSS 目录。
- **结果**: 样式文件现在可以被浏览器正确加载。

## 4. 文档 (Documentation)
- 新增 `document/style_guide.md`: 记录项目配色方案、字体规范及核心组件样式。
