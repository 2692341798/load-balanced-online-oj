# 前端样式指南

## 1. 设计系统概述

本样式指南定义了冻梨OJ（在线评测系统）的视觉设计规范，采用现代化的深色主题设计，提供一致的用户体验。

### 1.1 设计理念
- **现代化**: 采用当前流行的设计趋势
- **深色主题**: 减少视觉疲劳，适合长时间编程
- **响应式**: 适配不同屏幕尺寸和设备
- **可访问性**: 确保良好的对比度和可读性
- **一致性**: 统一的视觉语言和交互模式

## 2. 颜色系统 (Color System)

基于CSS变量的颜色管理系统，便于主题切换和维护。

### 2.1 主要颜色

| 变量名 | 色值 | 描述 | 使用场景 |
|--------|------|------|----------|
| `--bg-color` | `#1a1a1a` | 主背景色 | 页面背景、主要容器 |
| `--text-main` | `#ffffff` | 主要文字 | 标题、重要内容 |
| `--text-secondary` | `#8c8c8c` | 次要文字 | 描述、辅助信息 |
| `--accent-color` | `#ffa116` | 强调色 | 品牌色、重要按钮 |
| `--accent-hover` | `#e69114` | 强调色悬停 | 鼠标悬停状态 |
| `--nav-bg` | `#282828` | 导航栏背景 | 顶部导航栏 |
| `--card-bg` | `#282828` | 卡片背景 | 内容卡片、面板 |
| `--success` | `#2cbb5d` | 成功色 | 成功状态、主要按钮 |
| `--border-color` | `rgba(255, 255, 255, 0.1)` | 边框色 | 分割线、边框 |

### 2.2 状态颜色

| 状态 | 颜色 | 描述 |
|------|------|------|
| 成功 | `#2cbb5d` | 操作成功、通过状态 |
| 警告 | `#ffa116` | 警告信息、需要注意 |
| 错误 | `#f44336` | 错误状态、失败信息 |
| 信息 | `#2196f3` | 一般信息、提示 |

### 2.3 渐变效果

```css
/* 成功按钮渐变 */
background: linear-gradient(90deg, #2cbb5d 0%, #1e8e3e 100%);

/* 悬停时的阴影效果 */
box-shadow: 0 4px 15px rgba(44, 187, 93, 0.3);
```

## 3. 字体系统 (Typography)

### 3.1 字体栈
```css
font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji";
```

### 3.2 字体大小

| 级别 | 大小 | 用途 |
|------|------|------|
| 标题1 | `2.5rem` | 主标题 |
| 标题2 | `2rem` | 副标题 |
| 标题3 | `1.5rem` | 小标题 |
| 正文 | `1rem` | 主要内容 |
| 小字 | `0.875rem` | 辅助信息 |
| 极小 | `0.75rem` | 标签、提示 |

### 3.3 行高和字重

```css
line-height: 1.5;      /* 标准行高 */
font-weight: 400;      /* 正常字重 */
font-weight: 500;      /* 中等字重 */
font-weight: 600;      /* 粗体 */
```

## 4. 布局系统 (Layout System)

### 4.1 网格系统
- **最大宽度**: `1200px`
- **边距**: `50px` (桌面端), `20px` (移动端)
- **间距**: 基于`8px`的倍数系统

### 4.2 断点设置

```css
/* 移动端 */
@media (max-width: 768px) { ... }

/* 平板端 */
@media (max-width: 1024px) { ... }

/* 桌面端 */
@media (min-width: 1025px) { ... }
```

### 4.3 容器规范

```css
/* 主容器 */
.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 0 50px;
}

/* 卡片容器 */
.card {
    background: var(--card-bg);
    border-radius: 8px;
    padding: 24px;
    border: 1px solid var(--border-color);
}
```

## 5. 组件设计 (Components)

### 5.1 导航栏 (Navbar)

#### 5.1.1 结构规范
```html
<nav class="navbar">
    <div class="navbar-brand">
        <a href="/">
            <span>&lt;/&gt;</span> 冻梨OJ
        </a>
    </div>
    <div class="navbar-links">
        <a href="/">首页</a>
        <a href="/all_questions">题库</a>
    </div>
    <div class="navbar-auth">
        <!-- 用户认证区域 -->
    </div>
</nav>
```

#### 5.1.2 样式规范
```css
.navbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0 50px;
    height: 60px;
    background-color: var(--bg-color);
    position: fixed;
    width: 100%;
    top: 0;
    z-index: 1000;
    border-bottom: 1px solid var(--border-color);
}

.navbar-brand {
    display: flex;
    align-items: center;
    font-size: 1.5rem;
    font-weight: bold;
    color: var(--text-main);
}

.navbar-brand span {
    color: var(--accent-color);
    margin-right: 5px;
}

.navbar-links {
    display: flex;
    gap: 30px;
    align-items: center;
}

.navbar-links a {
    color: var(--text-secondary);
    font-size: 0.95rem;
    transition: color 0.3s;
}

.navbar-links a:hover,
.navbar-links a.active {
    color: var(--text-main);
}
```

### 5.2 按钮系统 (Buttons)

#### 5.2.1 主要按钮 (Primary Button)
```css
.btn-primary {
    background: linear-gradient(90deg, #2cbb5d 0%, #1e8e3e 100%);
    color: white;
    border: none;
    padding: 12px 24px;
    border-radius: 25px;
    font-size: 1rem;
    font-weight: 500;
    cursor: pointer;
    transition: all 0.3s ease;
    box-shadow: 0 4px 15px rgba(44, 187, 93, 0.3);
}

.btn-primary:hover {
    transform: translateY(-2px);
    box-shadow: 0 6px 20px rgba(44, 187, 93, 0.4);
}
```

#### 5.2.2 文字按钮 (Text Button)
```css
.btn-text {
    color: var(--text-secondary);
    background: none;
    border: none;
    padding: 8px 16px;
    font-size: 0.95rem;
    cursor: pointer;
    transition: color 0.3s;
}

.btn-text:hover {
    color: var(--text-main);
}
```

#### 5.2.3 按钮状态

| 状态 | 样式变化 |
|------|----------|
| 默认 | 基础样式 |
| 悬停 | 颜色变亮，轻微上浮 |
| 激活 | 颜色加深，缩小效果 |
| 禁用 | 透明度降低，鼠标禁用 |
| 加载 | 旋转动画，禁用点击 |

### 5.3 表单组件 (Forms)

#### 5.3.1 输入框 (Input)
```css
.form-group {
    margin-bottom: 20px;
}

.form-group label {
    display: block;
    margin-bottom: 8px;
    color: var(--text-main);
    font-weight: 500;
}

.form-group input {
    width: 100%;
    padding: 12px 16px;
    background: var(--card-bg);
    border: 1px solid var(--border-color);
    border-radius: 6px;
    color: var(--text-main);
    font-size: 1rem;
    transition: border-color 0.3s, box-shadow 0.3s;
}

.form-group input:focus {
    outline: none;
    border-color: var(--accent-color);
    box-shadow: 0 0 0 3px rgba(255, 161, 22, 0.1);
}

.form-group input::placeholder {
    color: var(--text-secondary);
}
```

#### 5.3.2 选择框和复选框
```css
.checkbox-group {
    display: flex;
    align-items: center;
    gap: 8px;
    color: var(--text-secondary);
}

.checkbox-group input[type="checkbox"] {
    width: auto;
    margin: 0;
}
```

### 5.4 卡片组件 (Cards)

#### 5.4.1 基础卡片
```css
.card {
    background: var(--card-bg);
    border-radius: 8px;
    padding: 24px;
    border: 1px solid var(--border-color);
    transition: transform 0.3s, box-shadow 0.3s;
}

.card:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.2);
}
```

#### 5.4.2 题目卡片
```css
.question-card {
    background: var(--card-bg);
    border-radius: 8px;
    padding: 20px;
    margin-bottom: 16px;
    border: 1px solid var(--border-color);
    transition: all 0.3s ease;
    cursor: pointer;
}

.question-card:hover {
    border-color: var(--accent-color);
    box-shadow: 0 4px 12px rgba(255, 161, 22, 0.1);
}

.question-title {
    font-size: 1.1rem;
    font-weight: 600;
    color: var(--text-main);
    margin-bottom: 8px;
}

.question-difficulty {
    display: inline-block;
    padding: 4px 8px;
    border-radius: 4px;
    font-size: 0.75rem;
    font-weight: 500;
}

.difficulty-easy {
    background: rgba(44, 187, 93, 0.2);
    color: #2cbb5d;
}

.difficulty-medium {
    background: rgba(255, 161, 22, 0.2);
    color: #ffa116;
}

.difficulty-hard {
    background: rgba(244, 67, 54, 0.2);
    color: #f44336;
}
```

### 5.5 布局组件 (Layout Components)

#### 5.5.1 可调整面板 (Resizable Panes)
```css
.split-pane {
    display: flex;
    height: 100%;
}

.gutter {
    background-color: var(--bg-color);
    background-repeat: no-repeat;
    background-position: 50%;
    transition: background-color 0.3s;
}

.gutter:hover {
    background-color: var(--accent-color);
}

.gutter.gutter-horizontal {
    cursor: col-resize;
    width: 10px;
}

.gutter.gutter-vertical {
    cursor: row-resize;
    height: 10px;
}
```

#### 5.5.2 标签页界面 (Tabbed Interface)
用于测试用例和运行结果的展示。

```css
.tabs-container {
    display: flex;
    flex-direction: column;
    height: 100%;
}

.tabs-header {
    display: flex;
    background: #2d2d2d;
    border-bottom: 1px solid var(--border-color);
}

.tab-item {
    padding: 10px 20px;
    color: var(--text-secondary);
    cursor: pointer;
    border-right: 1px solid var(--border-color);
    transition: all 0.3s;
    font-size: 0.9rem;
}

.tab-item:hover {
    color: var(--text-main);
    background: rgba(255, 255, 255, 0.05);
}

.tab-item.active {
    color: var(--text-main);
    background: #1e1e1e;
    border-top: 2px solid var(--accent-color);
}

.tab-content {
    flex: 1;
    overflow: auto;
    background: #1e1e1e;
    padding: 16px;
}
```

## 6. 代码编辑器样式 (Code Editor)

### 6.1 主题标准
- **标准主题**: VS Code Dark Theme
- **支持语言**: C++, Java, Python
- **字体**: Consolas, 'Courier New', monospace

### 6.2 编辑器容器
```css
.code-editor {
    background: #1e1e1e;
    border: 1px solid var(--border-color);
    border-radius: 6px;
    overflow: hidden;
}

.editor-header {
    background: #2d2d2d;
    padding: 12px 16px;
    border-bottom: 1px solid var(--border-color);
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.editor-title {
    color: var(--text-main);
    font-size: 0.9rem;
    font-weight: 500;
}
```

### 6.3 代码区域
```css
.code-area {
    background: #1e1e1e;
    color: #d4d4d4;
    font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
    font-size: 14px;
    line-height: 1.5;
    padding: 16px;
    min-height: 300px;
    white-space: pre-wrap;
    overflow-x: auto;
    tab-size: 4;
}

.code-area:focus {
    outline: none;
}
```

### 6.4 语法高亮（基础）
```css
/* 关键字 */
.keyword { color: #569cd6; }

/* 字符串 */
.string { color: #ce9178; }

/* 注释 */
.comment { color: #6a9955; }

/* 数字 */
.number { color: #b5cea8; }

/* 函数 */
.function { color: #dcdcaa; }
```

## 7. 结果展示组件 (Result Display)

### 7.1 结果容器
```css
.result-container {
    background: var(--card-bg);
    border-radius: 8px;
    padding: 20px;
    border: 1px solid var(--border-color);
    margin-top: 20px;
}

.result-header {
    display: flex;
    align-items: center;
    gap: 12px;
    margin-bottom: 16px;
}

.result-status {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    padding: 8px 16px;
    border-radius: 20px;
    font-size: 0.9rem;
    font-weight: 500;
}

.status-success {
    background: rgba(44, 187, 93, 0.2);
    color: #2cbb5d;
}

.status-error {
    background: rgba(244, 67, 54, 0.2);
    color: #f44336;
}

.status-warning {
    background: rgba(255, 161, 22, 0.2);
    color: #ffa116;
}
```

### 7.2 输出区域
```css
.output-area {
    background: #1e1e1e;
    border: 1px solid var(--border-color);
    border-radius: 4px;
    padding: 12px 16px;
    font-family: 'Consolas', 'Monaco', monospace;
    font-size: 14px;
    line-height: 1.5;
    color: #d4d4d4;
    white-space: pre-wrap;
    max-height: 200px;
    overflow-y: auto;
}

.output-label {
    color: var(--text-secondary);
    font-size: 0.85rem;
    margin-bottom: 8px;
    font-weight: 500;
}
```

## 8. 动画和过渡 (Animations)

### 8.1 过渡效果
```css
/* 基础过渡 */
.transition {
    transition: all 0.3s ease;
}

/* 快速过渡 */
.transition-fast {
    transition: all 0.15s ease;
}

/* 慢速过渡 */
.transition-slow {
    transition: all 0.5s ease;
}
```

### 8.2 加载动画
```css
@keyframes spin {
    0% { transform: rotate(0deg); }
    100% { transform: rotate(360deg); }
}

.loading {
    display: inline-block;
    width: 20px;
    height: 20px;
    border: 2px solid var(--border-color);
    border-top: 2px solid var(--accent-color);
    border-radius: 50%;
    animation: spin 1s linear infinite;
}
```

### 8.3 淡入淡出
```css
@keyframes fadeIn {
    from { opacity: 0; transform: translateY(10px); }
    to { opacity: 1; transform: translateY(0); }
}

.fade-in {
    animation: fadeIn 0.3s ease-out;
}
```

## 9. 响应式设计 (Responsive Design)

### 9.1 移动端适配
```css
/* 移动端导航栏 */
@media (max-width: 768px) {
    .navbar {
        padding: 0 20px;
    }
    
    .navbar-links {
        gap: 20px;
    }
    
    .container {
        padding: 0 20px;
    }
    
    .card {
        padding: 16px;
    }
    
    .btn-primary {
        padding: 10px 20px;
        font-size: 0.95rem;
    }
}
```

### 9.2 平板适配
```css
@media (max-width: 1024px) {
    .hero {
        padding: 80px 20px;
    }
    
    .grid-2 {
        grid-template-columns: 1fr;
        gap: 20px;
    }
}
```

## 10. 可访问性 (Accessibility)

### 10.1 颜色对比
- 确保所有文本与背景的颜色对比度达到WCAG 2.1 AA标准
- 重要信息的对比度达到AAA标准

### 10.2 焦点指示
```css
/* 键盘导航焦点样式 */
:focus-visible {
    outline: 2px solid var(--accent-color);
    outline-offset: 2px;
}

/* 移除默认焦点样式 */
:focus:not(:focus-visible) {
    outline: none;
}
```

### 10.3 屏幕阅读器支持
```css
/* 隐藏但对屏幕阅读器可见 */
.visually-hidden {
    position: absolute;
    width: 1px;
    height: 1px;
    padding: 0;
    margin: -1px;
    overflow: hidden;
    clip: rect(0, 0, 0, 0);
    white-space: nowrap;
    border: 0;
}
```

## 11. 主题扩展 (Theme Extensions)

### 11.1 深色主题变量
```css
:root {
    /* 深色主题 */
    --bg-color: #1a1a1a;
    --text-main: #ffffff;
    --text-secondary: #8c8c8c;
    --card-bg: #282828;
    --border-color: rgba(255, 255, 255, 0.1);
}

/* 浅色主题（未来扩展） */
[data-theme="light"] {
    --bg-color: #ffffff;
    --text-main: #1a1a1a;
    --text-secondary: #666666;
    --card-bg: #f5f5f5;
    --border-color: rgba(0, 0, 0, 0.1);
}
```

### 11.2 高对比度模式
```css
@media (prefers-contrast: high) {
    :root {
        --text-main: #ffffff;
        --text-secondary: #cccccc;
        --border-color: rgba(255, 255, 255, 0.3);
    }
}
```

## 12. 文件组织规范

### 12.1 CSS文件结构
```
oj_server/resources/css/
├── index.css           # 基础样式和变量
├── login.css          # 登录页面样式
├── all_questions.css  # 题目列表样式
├── one_question.css   # 题目详情样式
├── admin.css          # 管理员后台样式
└── components.css     # 通用组件样式（可选）
```

### 12.2 命名规范
- **BEM命名法**: `block__element--modifier`
- **CSS变量**: `--category-property`
- **类名**: 小写字母，连字符分隔
- **ID**: 避免使用ID选择器，优先使用类选择器

### 12.3 代码规范
```css
/* 良好的代码格式 */
.navbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0 50px;
    height: 60px;
    background-color: var(--bg-color);
}

/* 属性顺序 */
/* 1. 布局属性 */
/* 2. 盒模型属性 */
/* 3. 视觉属性 */
/* 4. 文字属性 */
/* 5. 动画属性 */
```

## 14. Markdown 内容样式 (Markdown Content)

用于题目描述、题解等富文本内容的展示。

### 14.1 基础排版
```css
.markdown-body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
    font-size: 16px;
    line-height: 1.6;
    color: var(--text-main);
}

.markdown-body h1,
.markdown-body h2,
.markdown-body h3 {
    margin-top: 24px;
    margin-bottom: 16px;
    font-weight: 600;
    line-height: 1.25;
    color: var(--text-main);
}

.markdown-body p {
    margin-bottom: 16px;
    color: #d4d4d4;
}

.markdown-body blockquote {
    padding: 0 1em;
    color: var(--text-secondary);
    border-left: 0.25em solid var(--border-color);
    margin: 0 0 16px 0;
}
```

### 14.2 代码块与列表
```css
.markdown-body pre {
    padding: 16px;
    overflow: auto;
    font-size: 85%;
    line-height: 1.45;
    background-color: #1e1e1e;
    border-radius: 6px;
    border: 1px solid var(--border-color);
    margin-bottom: 16px;
}

.markdown-body code {
    padding: 0.2em 0.4em;
    margin: 0;
    font-size: 85%;
    background-color: rgba(255, 255, 255, 0.1);
    border-radius: 6px;
}

.markdown-body pre code {
    padding: 0;
    background-color: transparent;
}

.markdown-body ul,
.markdown-body ol {
    padding-left: 2em;
    margin-bottom: 16px;
}

.markdown-body li {
    margin-top: 0.25em;
}

/* 图片样式优化 (V0.5.6) */
.markdown-body img {
    max-width: 100%;
    box-sizing: border-box;
    background-color: #fff; /* 适配深色主题 */
    padding: 10px;
    border-radius: 4px;
    display: block;
    margin: 10px auto;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2);
}
```

### 14.3 数学公式支持 (MathJax)
- **引擎**: MathJax v3
- **定界符**:
  - 行内公式: `$ ... $` 或 `\( ... \)`
  - 块级公式: `$$ ... $$` 或 `\[ ... \]`
- **渲染逻辑**: 前端采用 "Protect-Parse-Restore" 策略，防止 Markdown 解析器破坏 LaTeX 语法。

### 14.4 讨论区样式 (V0.3.2)
- **卡片样式**: `.discussion-card` 继承基础卡片风格，增加了悬停浮动效果。
- **标签样式**: `.tag` 使用半透明背景，保持视觉轻量。
- **编辑器集成**: 使用 `EasyMDE` 默认深色适配，自定义了 Toolbar 样式以融入整体主题。
- **内联评论**: 使用绝对定位的 Tooltip (`#inline-comment-tooltip`) 实现上下文交互。
- **Markdown渲染约束**: 使用固定版本的 `marked`（例如 `marked@4.3.0`）并在插入 DOM 前用 `DOMPurify.sanitize` 过滤，避免 CDN 自动升级与 XSS 风险。

### 14.4 竞赛列表样式 (Contest List)

**文件**: `oj_server/resources/css/contest-list.css`

**设计特点**:
- **表格布局**: 使用标准 HTML `<table>` 展示竞赛信息，表头固定。
- **状态标签**:
  - `status-badge`: 圆角标签，根据时间动态显示状态。
  - `status-upcoming`: 蓝色，表示未开始。
  - `status-running`: 绿色，表示进行中。
  - `status-ended`: 灰色，表示已结束。
- **品牌识别**: 标题栏包含来源平台的 Logo 图标 (Codeforces/LeetCode)。
- **响应式表格**: 移动端自动转为卡片式布局 (Card View)。

```css
/* 状态标签示例 */
.status-badge {
    padding: 4px 12px;
    border-radius: 12px;
    font-size: 0.85rem;
    font-weight: 500;
}

.status-upcoming {
    background: rgba(33, 150, 243, 0.2);
    color: #2196f3;
}
```

## 15. 性能优化

### 15.1 CSS优化
- 使用CSS变量减少重复代码
- 合理使用CSS选择器，避免过度嵌套
- 压缩CSS文件，移除未使用的样式
- 使用硬件加速的属性

### 15.2 加载优化
```css
/* 关键CSS内联 */
/* 非关键CSS异步加载 */
/* 使用font-display优化字体加载 */
```

### 15.3 渲染优化
- 避免触发重排和重绘
- 使用transform和opacity进行动画
- 合理使用will-change属性

---

**文档版本**: v1.0.1  
**最后更新时间**: 2026-02-09  
**维护团队**: 在线评测系统开发团队
