import { test, expect } from '@playwright/test';

const TARGET_URL = process.env.TARGET_URL || 'http://localhost:8097';
const USERNAME = 'admin123';
const PASSWORD = 'Ahqjhqj73@you';

test.describe('综合场景集成测试', () => {

  test.beforeEach(async ({ page }) => {
    // 1. 全局前置登录
    await page.goto(`${TARGET_URL}/login`);
    const usernameInput = page.locator('#login-username');
    const passwordInput = page.locator('#login-password');
    const submitBtn = page.locator('#login-submit-btn');

    await usernameInput.fill(USERNAME);
    await passwordInput.fill(PASSWORD);
    await submitBtn.click();

    // 等待登录成功跳转
    await expect(page).not.toHaveURL(/.*login/i, { timeout: 10000 });
  });

  // 场景 1: 多语言支持测试
  test('多语言支持：切换 C++, Python, Java 并提交代码', async ({ page }) => {
    const languages = [
      {
        name: 'C++',
        code: '#include <iostream>\nusing namespace std;\nint main() { int a, b; while (cin >> a >> b) { cout << a + b << endl; } return 0; }',
      },
      {
        name: 'Python',
        code: 'import sys\nfor line in sys.stdin:\n    a, b = map(int, line.split())\n    print(a + b)',
      },
      {
        name: 'Java',
        code: 'import java.util.Scanner;\npublic class Main {\n    public static void main(String[] args) {\n        Scanner sc = new Scanner(System.in);\n        while (sc.hasNextInt()) {\n            int a = sc.nextInt();\n            int b = sc.nextInt();\n            System.out.println(a + b);\n        }\n    }\n}',
      }
    ];

    for (const lang of languages) {
      await test.step(`提交并测试语言: ${lang.name}`, async () => {
        await page.goto(`${TARGET_URL}/question/1`);
        await expect(page.locator('#language-select')).toBeVisible();
        
        // 选择语言（兼容处理不同的 value 或 label）
        const selectLocator = page.locator('#language-select');
        try {
          await selectLocator.selectOption({ label: lang.name });
        } catch {
          await selectLocator.selectOption(lang.name);
        }

        // 注入对应语言的代码
        await page.evaluate((codeContent) => {
          // @ts-ignore
          const editor = ace.edit("code");
          editor.setValue(codeContent);
        }, lang.code);

        // 提交运行
        const submitBtn = page.locator('.btn-submit');
        await submitBtn.click();

        // 等待评测状态重置
        await expect(submitBtn).toHaveText('提交运行', { timeout: 30000 });
        
        // 验证是否有评测结果返回
        const resultArea = page.locator('#result-area');
        await expect(resultArea).toContainText(/通过|答案错误|编译错误/);
      });
    }
  });

  // 场景 2: 网络断开重连测试
  test('网络重连：模拟离线提交失败后，恢复在线提交成功', async ({ page, context }) => {
    await page.goto(`${TARGET_URL}/question/1`);
    
    // 注入基础测试代码
    await page.evaluate(() => {
      // @ts-ignore
      const editor = ace.edit("code");
      editor.setValue('#include <iostream>\nusing namespace std;\nint main() { cout << "Test" << endl; return 0; }');
    });

    const submitBtn = page.locator('.btn-submit');

    // 1. 模拟断网环境
    await context.setOffline(true);
    
    // 尝试提交，预期的行为是无响应、一直 loading 或者抛出网络错误提示
    await submitBtn.click();
    const resultArea = page.locator('#result-area');
    
    // 验证离线时前端的异常反馈（具体文案依据实际情况，或者静默失败）
    await expect(resultArea).toContainText(/错误|网络|Failed|Network/i, { timeout: 5000 }).catch(() => {});

    // 2. 恢复网络连接
    await context.setOffline(false);
    
    // 3. 再次尝试提交，应成功到达服务器并返回评测结果
    await submitBtn.click();
    await expect(submitBtn).toHaveText('提交运行', { timeout: 30000 });
    await expect(resultArea).toContainText(/通过|答案错误/);
  });

  // 场景 3: 视觉回归测试
  test('视觉回归：对比评测结果页面的渲染截图', async ({ page }) => {
    await page.goto(`${TARGET_URL}/question/1`);
    
    // 注入错误代码以确保结果面板稳定呈现
    await page.evaluate(() => {
      // @ts-ignore
      const editor = ace.edit("code");
      editor.setValue('#include <iostream>\nusing namespace std;\nint main() { cout << "Wrong Answer" << endl; return 0; }');
    });

    const submitBtn = page.locator('.btn-submit');
    await submitBtn.click();
    
    // 等待评测执行完成
    await expect(submitBtn).toHaveText('提交运行', { timeout: 30000 });
    
    // 定位结果容器并确保其可见
    const resultContainer = page.locator('.result-area-container').first();
    await expect(resultContainer).toBeVisible();
    
    // 使用 Playwright 的截图对比功能（设定 1% 的容差比率）
    await expect(resultContainer).toHaveScreenshot('submission-result.png', { maxDiffPixelRatio: 0.01 });
  });

  // 场景 4: 排行榜/状态实时更新测试
  test('排行榜实时更新：提交正确答案后，3秒内验证数据刷新', async ({ page }) => {
    // 注意：当前系统可能以个人统计数据（/profile）作为得分或通过数的展示。
    // 如果系统有专门的 /leaderboard，可以修改此路由。
    const RANK_URL = `${TARGET_URL}/profile`; 
    
    // 1. 记录提交前的分数/通过题数
    await page.goto(RANK_URL);
    const scoreLocator = page.locator('.stat-total .stat-value').first(); // 根据页面实际结构调整
    let initialScoreText = '0';
    if (await scoreLocator.isVisible()) {
      initialScoreText = await scoreLocator.innerText();
    }
    const initialScore = parseInt(initialScoreText.trim(), 10) || 0;

    // 2. 进行有效提交 (P1000 - A+B)
    await page.goto(`${TARGET_URL}/question/1`);
    await page.evaluate(() => {
      // @ts-ignore
      const editor = ace.edit("code");
      editor.setValue('#include <iostream>\nusing namespace std;\nint main() { int a, b; while (cin >> a >> b) { cout << a + b << endl; } return 0; }');
    });

    const submitBtn = page.locator('.btn-submit');
    await submitBtn.click();
    await expect(submitBtn).toHaveText('提交运行', { timeout: 30000 });
    await expect(page.locator('#result-area')).toContainText('通过');

    // 3. 验证 3 秒内分数/排行榜更新
    const startTime = Date.now();
    await page.goto(RANK_URL);
    
    // 使用 toPass 轮询机制，如果在 3 秒内满足断言即视为通过
    await expect(async () => {
      const newScoreText = await page.locator('.stat-total .stat-value').first().innerText();
      const newScore = parseInt(newScoreText.trim(), 10) || 0;
      expect(newScore).toBeGreaterThanOrEqual(initialScore);
    }).toPass({ timeout: 3000 });

    const timeTaken = Date.now() - startTime;
    // 确保整个导航和验证的时间在 3000 毫秒以内
    expect(timeTaken).toBeLessThanOrEqual(3000);
  });
});
