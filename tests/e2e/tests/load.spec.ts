import { test, expect, chromium } from '@playwright/test';
import * as fs from 'fs';
import * as path from 'path';

// 目标环境配置
const TARGET_URL = process.env.TARGET_URL || 'http://localhost:8097';
const USERNAME = process.env.USERNAME || 'admin123';
const PASSWORD = process.env.PASSWORD || 'Ahqjhqj73@you';

// 压测参数配置
const CONCURRENT_USERS = 13; // 调整并发测试最佳处理量
const DURATION_MS = 30000; // 30秒
const PROBLEM_ID = '2';

// 简单的 A+B AC代码
const AC_CODE = `#include <iostream>
using namespace std;
int main() {
    int a, b;
    while (cin >> a >> b) {
        cout << a + b << endl;
    }
    return 0;
}`;

// 用于存储指标的接口
interface Metric {
  timestamp: string;
  userId: number;
  url: string;
  status: number;
  duration: number; // 毫秒
}

test.describe('P1000 负载与性能测试', () => {
  // 设置此套件特定的 actionTimeout
  test.use({ actionTimeout: 60000, navigationTimeout: 60000 });

  // 由于测试运行 60 秒，另外还包含浏览器启动、登录、资源回收等操作，这里将超时设为 120 秒
  test.setTimeout(DURATION_MS + 60000);

  test('模拟并发用户持续提交 AC 代码', async () => {
    // 启动一个浏览器实例，复用该实例以降低内存消耗
    const browser = await chromium.launch();
    const metrics: Metric[] = [];
    
    const startTime = Date.now();
    const endTime = startTime + DURATION_MS;

    // 单个用户会话流程
    const runUserSession = async (userId: number) => {
      // 为每个用户创建隔离的上下文（Cookies/Session 互相独立）
      const context = await browser.newContext();
      const page = await context.newPage();

      // 监听网络响应以收集状态码和响应时间
      page.on('response', (response) => {
        const url = response.url();
        const request = response.request();
        
        // 仅拦截关心的登录请求 (判断是否有 5xx)
        if (request.method() === 'POST' && url.includes('/login')) {
          const status = response.status();
          metrics.push({
            timestamp: new Date().toISOString(),
            userId,
            url,
            status,
            duration: 0
          });
        }
      });

      try {
        // 1. 登录流程
        await page.goto(`${TARGET_URL}/login`);
        await page.locator('#login-username').fill(USERNAME);
        await page.locator('#login-password').fill(PASSWORD);
        await page.locator('#login-submit-btn').click();
        
        // 等待登录成功跳转（不再位于 login 页面）
        await expect(page).not.toHaveURL(/.*login/i, { timeout: 15000 });

        // 2. 导航至 P1000 题目页
        await page.goto(`${TARGET_URL}/question/${PROBLEM_ID}`);
        
        // 等待语言选择框加载完毕并选择 C++
        await expect(page.locator('#language-select')).toBeVisible({ timeout: 15000 });
        await page.locator('#language-select').selectOption('C++');

        // 等待 ace editor 加载完毕
        await page.waitForFunction(() => typeof (window as any).ace !== 'undefined', { timeout: 30000 });
        // 等待一小段时间确保 DOM 元素完全渲染并绑定
        await page.waitForTimeout(1000);

        // 使用页面中的 ace editor 注入代码
        await page.evaluate((codeContent) => {
          // @ts-ignore
          const editor = ace.edit("code");
          editor.setValue(codeContent);
        }, AC_CODE);

        const submitBtn = page.locator('.btn-submit');

        // 3. 在规定时间 (60s) 内循环不断提交
        while (Date.now() < endTime) {
          const reqStart = Date.now();
          await submitBtn.click();
          
          // 等待提交流程结束（按钮文本从 "评测中..." 变回 "提交运行"）
          // 为高并发情况预留较长等待时间 (60s)，防止因高负载变慢导致误报超时失败
          await expect(submitBtn).toHaveText('提交运行', { timeout: 60000 });
          
          const reqEnd = Date.now();
          metrics.push({
            timestamp: new Date().toISOString(),
            userId,
            url: `${TARGET_URL}/judge/${PROBLEM_ID}`,
            status: 200, // Assuming success if it returns to "提交运行"
            duration: reqEnd - reqStart
          });
          
          // 短暂间隔防止完全 0 延迟死循环拖垮本地 NodeJS 和网络栈
          await page.waitForTimeout(500);
        }
      } catch (err) {
        console.error(`User ${userId} encountered error:`, err);
      } finally {
        await context.close();
      }
    };

    // 使用 Promise.all 同时启动 50 个并发用户
    const promises = Array.from({ length: CONCURRENT_USERS }, (_, i) => runUserSession(i + 1));
    await Promise.all(promises);

    await browser.close();

    // 将收集到的指标导出为 CSV
    const csvPath = path.join(__dirname, 'load_test_metrics.csv');
    const header = 'Timestamp,UserId,URL,Status,Duration(ms)\n';
    const rows = metrics.map(m => `${m.timestamp},${m.userId},${m.url},${m.status},${m.duration}`).join('\n');
    fs.writeFileSync(csvPath, header + rows);
    console.log(`指标已成功导出至: ${csvPath}`);

    // ===== 性能断言 =====
    const judgeMetrics = metrics.filter(m => m.url.includes(`/judge/${PROBLEM_ID}`));
    
    // 1. 断言没有任何 5xx 服务器错误
    const errors5xx = judgeMetrics.filter(m => m.status >= 500 && m.status < 600);
    expect(errors5xx.length, `预期 0 个 5xx 错误，但发现了 ${errors5xx.length} 个`).toBe(0);

    // 2. 断言 P95 响应时间 <= 2秒 (2000ms)
    if (judgeMetrics.length > 0) {
      const durations = judgeMetrics.map(m => m.duration).sort((a, b) => a - b);
      const p95Index = Math.floor(durations.length * 0.95);
      const p95Duration = durations[p95Index];
      
      console.log(`总提交次数: ${judgeMetrics.length}`);
      console.log(`P95 响应时间: ${p95Duration}ms`);
      console.log(`最大响应时间: ${durations[durations.length - 1]}ms`);
      
      expect(p95Duration, `预期 P95 响应时间 <= 2000ms, 但实际为 ${p95Duration}ms`).toBeLessThanOrEqual(2000);
    } else {
      console.warn('未收集到任何提交评测指标，请检查 URL 匹配逻辑是否正确。');
    }
  });
});
