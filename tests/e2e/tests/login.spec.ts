import { test, expect } from '@playwright/test';

// 从环境变量读取目标 URL，或者使用默认的开发环境地址
const TARGET_URL = process.env.TARGET_URL || 'http://localhost:8097';
const USERNAME = 'admin123';
const CORRECT_PASSWORD = 'Ahqjhqj73@you';

test.describe('登录与认证测试 (Login and Authentication)', () => {
  test.beforeEach(async ({ page }) => {
    // 每次测试前访问目标 URL 的登录页
    const loginUrl = TARGET_URL.endsWith('/') ? TARGET_URL + 'login' : TARGET_URL + '/login';
    await page.goto(loginUrl);
  });

  test('1. 正确凭证登录 (Correct credentials login)', async ({ page }) => {
    // 定位用户名、密码输入框及提交按钮，兼容不同常见的选择器
    const usernameInput = page.locator('#login-username');
    const passwordInput = page.locator('#login-password');
    const submitBtn = page.locator('#login-submit-btn');

    // 填入正确的账号和密码
    await usernameInput.fill(USERNAME);
    await passwordInput.fill(CORRECT_PASSWORD);
    await submitBtn.click();

    // 断言：登录成功后 URL 发生变化（不再包含 login）
    await expect(page).not.toHaveURL(/.*login/i, { timeout: 10000 });
    
    // 或者可以补充断言：检查是否出现了特定的成功提示、头像、退出按钮等
    // await expect(page.locator('text="登录成功"')).toBeVisible();
  });

  test('2. 错误密码 (Wrong password)', async ({ page }) => {
    const usernameInput = page.locator('#login-username');
    const passwordInput = page.locator('#login-password');
    const submitBtn = page.locator('#login-submit-btn');

    // 填入错误的密码
    await usernameInput.fill(USERNAME);
    await passwordInput.fill('WrongPassword123!');
    await submitBtn.click();

    // 断言：页面显示错误提示，并且依然停留在当前页面（可根据实际的 UI 框架调整 class 或文本）
    const errorMessage = page.locator('.error-message, .toast, .ant-message, :text("密码错误"), :text("用户名或密码错误"), :text("Invalid")').first();
    await expect(errorMessage).toBeVisible({ timeout: 5000 });
  });

  test('3. 空用户名或密码 (Empty username/password)', async ({ page }) => {
    const submitBtn = page.locator('#login-submit-btn');
    
    // 直接点击提交
    await submitBtn.click();

    // 断言：未发生跳转，依然停留在登录状态，密码框依旧可见
    const passwordInput = page.locator('#login-password');
    await expect(passwordInput).toBeVisible();

    // 可选：检查是否有必填提示（HTML5 原生验证或自定义前端 UI 提示）
    // const requiredMessage = page.locator('text="请输入用户名", text="不能为空"').first();
    // await expect(requiredMessage).toBeVisible();
  });

  test('4. SQL注入尝试 (SQL injection attempt)', async ({ page }) => {
    const usernameInput = page.locator('#login-username');
    const passwordInput = page.locator('#login-password');
    const submitBtn = page.locator('#login-submit-btn');

    // 尝试输入常见的 SQL 注入 payload
    const injectionPayload = "' OR '1'='1";
    await usernameInput.fill(injectionPayload);
    await passwordInput.fill(injectionPayload);
    await submitBtn.click();

    // 断言：系统应能正确拦截/处理注入，拒绝登录并给出通用的错误提示
    const errorMessage = page.locator('.error-message, .toast, .ant-message, :text("密码错误"), :text("用户名或密码错误"), :text("用户不存在")').first();
    await expect(errorMessage).toBeVisible({ timeout: 5000 });
    
    // 确保没有发生意外的登录跳转
    await expect(passwordInput).toBeVisible();
  });

  test('5. 暴力破解尝试 (Brute force login attempt)', async ({ page }) => {
    const usernameInput = page.locator('#login-username');
    const passwordInput = page.locator('#login-password');
    const submitBtn = page.locator('#login-submit-btn');

    // 模拟多次失败的登录尝试
    const MAX_ATTEMPTS = 6;
    for (let i = 0; i < MAX_ATTEMPTS; i++) {
      await usernameInput.fill(USERNAME);
      await passwordInput.fill(`WrongPwd_Attempt_${i}`);
      await submitBtn.click();
      
      // 等待请求响应，模拟真实用户操作间隔，也可等待网络空闲
      await page.waitForTimeout(500); 
    }

    // 断言：多次失败后应出现通用错误，或者触发系统的速率限制/锁定提示
    const errorMessage = page.locator('.error-message, .toast, .ant-message, :text("频繁"), :text("锁定"), :text("过多"), :text("密码错误"), :text("用户名或密码错误")').first();
    await expect(errorMessage).toBeVisible();
  });
});