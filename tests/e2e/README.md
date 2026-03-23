# 端到端(E2E)测试环境部署与执行指南

本文档介绍如何部署前端和后端的测试环境，并执行基于 Playwright 和 Allure 的端到端测试。

## 1. 环境准备

### 1.1 前置要求
- **Node.js**: v18 或以上版本 (建议 v20)
- **Docker & Docker Compose**: 用于快速启动后端服务、数据库和缓存环境
- **Git**: 用于代码版本控制

### 1.2 安装依赖
进入 E2E 测试目录并安装相关依赖项：

```bash
cd tests/e2e
npm ci
```

安装 Playwright 所需的浏览器及其系统依赖：

```bash
npx playwright install --with-deps
```

## 2. 启动被测系统

在执行测试之前，请确保目标系统的后端 API 服务和前端页面都已经正常运行。

### 2.1 启动后端及相关中间件
在项目根目录使用 Docker Compose 启动依赖的组件（MySQL、Redis等）及后端服务：

```bash
docker-compose up -d
```
请确保相关后端接口能正常响应（默认端口通常为 `8080` 或在 `docker-compose.yml` 中配置的端口）。

### 2.2 启动前端
进入前端项目目录，启动开发服务器：

```bash
cd frontend
npm ci
npm run dev
```
记录下前端服务器的运行地址（例如 `http://localhost:5173`），并在 `tests/e2e/playwright.config.ts` 中配置对应的 `baseURL`。

## 3. 执行测试

### 3.1 运行 Playwright 测试

执行所有 E2E 测试用例：
```bash
npx playwright test
```

如果需要以有头模式（可见浏览器）运行，可加上 `--headed` 参数：
```bash
npx playwright test --headed
```

如果需要执行特定的测试文件：
```bash
npx playwright test tests/login.spec.ts
```

### 3.2 调试测试
Playwright 提供了非常方便的 UI 调试模式：
```bash
npx playwright test --ui
```

## 4. 测试报告

### 4.1 Playwright 默认报告
测试执行完毕后，默认会在 `playwright-report` 目录下生成 HTML 报告。
可以通过以下命令查看：
```bash
npx playwright show-report
```

### 4.2 Allure 测试报告
本系统集成了 Allure 报告生成工具，测试运行结束后，原始结果会存放在 `allure-results` 目录下。

生成并启动 Allure HTML 报告：
```bash
npx allure generate ./allure-results --clean -o ./allure-report
npx allure open ./allure-report
```

## 5. 持续集成 (CI)
本项目已在 `.github/workflows/playwright.yml` 中配置了 GitHub Actions 工作流。每次推送代码到 `main` 或 `master` 分支，或提交 Pull Request 时，将自动构建测试环境并运行所有的 E2E 测试，测试生成的报告将作为 Artifacts 上传保存，保留期为 30 天。
