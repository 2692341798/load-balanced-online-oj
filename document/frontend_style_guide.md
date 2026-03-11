# 前端样式指南 (Frontend Style Guide)

## 1. 设计系统概述

本样式指南定义了冻梨OJ（在线评测系统）的 React 前端视觉设计规范。我们采用 **Tailwind CSS** 结合 **Shadcn UI** 组件库，打造现代化、响应式且支持深色模式的用户界面。

### 1.1 核心原则
- **Utility-First**: 优先使用 Tailwind 工具类，减少手写 CSS。
- **Component-Driven**: 样式与组件绑定，通过 Props 控制变体。
- **Theme-Aware**: 所有颜色使用 CSS 变量，支持动态主题切换。
- **Accessibility**: 遵循 WCAG 标准，确保键盘导航和屏幕阅读器支持。

## 2. 颜色系统 (Colors)

颜色变量定义在 `frontend/src/index.css` (或 `globals.css`) 中，并映射到 Tailwind 配置。

### 2.1 语义化颜色
| Token | Tailwind Class | 用途 |
|-------|----------------|------|
| Background | `bg-background` | 页面主背景 |
| Foreground | `text-foreground` | 主文字颜色 |
| Primary | `bg-primary`, `text-primary-foreground` | 主按钮、强调元素 |
| Secondary | `bg-secondary`, `text-secondary-foreground` | 次级按钮、卡片背景 |
| Muted | `bg-muted`, `text-muted-foreground` | 弱化信息、辅助文字 |
| Accent | `bg-accent`, `text-accent-foreground` | 悬停、选中状态 |
| Destructive | `bg-destructive` | 危险操作、错误提示 |

### 2.2 边框与分割
使用 `border-border` 和 `border-input` 处理边框颜色，确保在深色/浅色模式下自动适配。

## 3. 排版系统 (Typography)

### 3.1 字体栈
默认使用系统字体栈，代码字体使用 `Monaco`, `Consolas` 等等宽字体。

### 3.2 文本样式
- **标题**: `text-3xl font-bold tracking-tight` (H1), `text-2xl font-semibold` (H2)
- **正文**: `text-base leading-7`
- **辅助**: `text-sm text-muted-foreground`
- **代码**: `font-mono text-sm`

## 4. 布局与响应式 (Layout)

### 4.1 断点 (Breakpoints)
遵循 Tailwind 默认断点：
- `sm`: 640px
- `md`: 768px
- `lg`: 1024px
- `xl`: 1280px
- `2xl`: 1536px

### 4.2 容器 (Container)
使用 Shadcn 的 Layout 组件或 Tailwind `container` 类：
```tsx
<div className="container mx-auto py-10">
  {/* Content */}
</div>
```

## 5. 组件使用规范 (Components)

所有基础组件位于 `src/components/ui/`，由 Shadcn CLI 生成。

### 5.1 按钮 (Button)
```tsx
import { Button } from "@/components/ui/button"

// 主按钮
<Button>Click me</Button>

// 次级按钮
<Button variant="secondary">Cancel</Button>

// 危险按钮
<Button variant="destructive">Delete</Button>

// 带图标
<Button>
  <Mail className="mr-2 h-4 w-4" /> Login with Email
</Button>
```

### 5.2 卡片 (Card)
用于展示独立的内容块（如题目、题单）。
```tsx
<Card>
  <CardHeader>
    <CardTitle>Two Sum</CardTitle>
    <CardDescription>Easy • Array</CardDescription>
  </CardHeader>
  <CardContent>
    <p>Problem description...</p>
  </CardContent>
  <CardFooter>
    <Button>Solve</Button>
  </CardFooter>
</Card>
```

### 5.3 表单 (Form)
使用 `react-hook-form` + `zod` + Shadcn Form 组件。
```tsx
<Form {...form}>
  <form onSubmit={form.handleSubmit(onSubmit)} className="space-y-8">
    <FormField
      control={form.control}
      name="username"
      render={({ field }) => (
        <FormItem>
          <FormLabel>Username</FormLabel>
          <FormControl>
            <Input placeholder="shadcn" {...field} />
          </FormControl>
          <FormMessage />
        </FormItem>
      )}
    />
    <Button type="submit">Submit</Button>
  </form>
</Form>
```

## 6. CSS 组织策略

### 6.1 全局样式
仅在 `index.css` 中定义：
- Tailwind Directives (`@tailwind base;` etc.)
- CSS Variables (`:root { ... }`)
- 极少量的全局重置样式

### 6.2 组件样式
- **优先**: Tailwind Utility Classes (`className="..."`)
- **条件样式**: 使用 `cn()` 工具函数合并类名
  ```tsx
  import { cn } from "@/lib/utils"
  
  <div className={cn("bg-red-500", isActive && "bg-green-500")}>
  ```
- **复杂动画**: 可在 `tailwind.config.js` 中扩展 `theme.extend.keyframes`，或使用 `framer-motion`。

## 7. 特殊模块样式

### 7.1 代码编辑器
使用 Monaco Editor，通过配置项定制主题颜色以匹配应用主题。

### 7.2 Markdown 渲染
使用 `react-markdown` 配合 `typography` 插件 (`prose` 类名) 进行排版。
```tsx
<article className="prose dark:prose-invert max-w-none">
  <ReactMarkdown>{content}</ReactMarkdown>
</article>
```

---
**文档版本**: v2.0.0 (React Refactor)
**最后更新**: 2026-03-11
