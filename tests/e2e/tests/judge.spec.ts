import { test, expect } from '@playwright/test';

// 目标环境地址
const TARGET_URL = process.env.TARGET_URL || 'http://localhost:8097';
const USERNAME = 'admin123';
const PASSWORD = 'Ahqjhqj73@you';

test.describe('P1000 题目评测核心流程测试', () => {
  test.beforeEach(async ({ page }) => {
    // 1. 登录
    await page.goto(`${TARGET_URL}/login`);
    const usernameInput = page.locator('#login-username');
    const passwordInput = page.locator('#login-password');
    const submitBtn = page.locator('#login-submit-btn');

    await usernameInput.fill(USERNAME);
    await passwordInput.fill(PASSWORD);
    await submitBtn.click();

    // 等待登录成功跳转
    await expect(page).not.toHaveURL(/.*login/i, { timeout: 10000 });

    // 2. 导航到 P1000 题目详情页
    await page.goto(`${TARGET_URL}/question/1`);

    // 3. 确保页面加载完成，语言选择器可见
    await expect(page.locator('#language-select')).toBeVisible();

    // 4. 选择语言为 C++ (大多数系统 C/C++ 互通，由于下拉菜单选项是 C++)
    await page.locator('#language-select').selectOption('C++');
  });

  const testCases = [
    // 1. Accepted: Correct code
    {
      scenario: '1. Accepted (通过)',
      data: [
        { desc: '标准 A+B', code: '#include <stdio.h>\nint main() { int a, b; if (scanf("%d %d", &a, &b) == 2) { printf("%d\\n", a + b); } return 0; }', expected: /通过/ },
        { desc: '多组输入 A+B', code: '#include <stdio.h>\nint main() { int a, b; while (scanf("%d %d", &a, &b) != EOF) { printf("%d\\n", a + b); } return 0; }', expected: /通过/ },
        { desc: '使用 iostream 的 A+B', code: '#include <iostream>\nusing namespace std;\nint main() { int a, b; while (cin >> a >> b) { cout << a + b << endl; } return 0; }', expected: /通过/ }
      ]
    },
    // 2. Wrong Answer: Logic error
    {
      scenario: '2. Wrong Answer (未通过/逻辑错误)',
      data: [
        { desc: '输出固定错误值', code: '#include <stdio.h>\nint main() { int a, b; scanf("%d %d", &a, &b); printf("0\\n"); return 0; }', expected: /未通过|答案错误/ },
        { desc: '执行减法', code: '#include <stdio.h>\nint main() { int a, b; while(scanf("%d %d", &a, &b) != EOF) { printf("%d\\n", a - b); } return 0; }', expected: /未通过|答案错误/ },
        { desc: '输出无关字符串', code: '#include <stdio.h>\nint main() { printf("Hello World\\n"); return 0; }', expected: /未通过|答案错误/ }
      ]
    },
    // 3. Time Limit Exceeded: Infinite loop
    {
      scenario: '3. Time Limit Exceeded (时间超限)',
      data: [
        { desc: 'while(1) 死循环', code: '#include <stdio.h>\nint main() { while(1); return 0; }', expected: /时间超限/ },
        { desc: 'for(;;) 死循环', code: '#include <stdio.h>\nint main() { for(;;); return 0; }', expected: /时间超限/ },
        { desc: 'goto 死循环', code: '#include <stdio.h>\nint main() { loop: goto loop; return 0; }', expected: /时间超限/ }
      ]
    },
    // 4. Memory Limit Exceeded
    {
      scenario: '4. Memory Limit Exceeded (内存超限)',
      data: [
        { desc: '超大 malloc (按要求)', code: '#include <stdlib.h>\nint main() { void *p = malloc(9999ULL * 1024 * 1024 * 1024); if(!p) abort(); return 0; }', expected: /内存超限|运行时错误|未通过/ },
        { desc: '循环分配并写入', code: '#include <stdlib.h>\n#include <string.h>\nint main() { while(1) { void *p = malloc(1024 * 1024); if(p) memset(p, 1, 1024 * 1024); } return 0; }', expected: /内存超限|运行时错误|时间超限/ },
        { desc: '超大静态数组', code: '#include <stdio.h>\nint a[1024 * 1024 * 100];\nint main() { a[0] = 1; return 0; }', expected: /内存超限|编译错误|运行时错误|未通过|答案错误/ }
      ]
    },
    // 5. Compilation Error: Syntax error
    {
      scenario: '5. Compilation Error (编译错误)',
      data: [
        { desc: '缺少分号', code: '#include <stdio.h>\nint main() { int a, b\n return 0; }', expected: /编译错误/ },
        { desc: '未定义的函数', code: '#include <stdio.h>\nint main() { not_exist_func(); return 0; }', expected: /编译错误/ },
        { desc: '完全不合法的语法', code: 'this is completely wrong syntax', expected: /编译错误/ }
      ]
    },
    // 6. Empty Code: Empty or whitespace code
    {
      scenario: '6. Empty Code (空代码拦截)',
      data: [
        { desc: '完全为空', code: '', expected: /编译错误|提交错误|未知错误|不能为空/ },
        { desc: '仅包含空格', code: '    ', expected: /编译错误|提交错误|未知错误|不能为空/ },
        { desc: '仅包含换行符', code: '\n\n\n', expected: /编译错误|提交错误|未知错误|不能为空/ }
      ]
    }
  ];

  for (const group of testCases) {
    test.describe(group.scenario, () => {
      for (let i = 0; i < group.data.length; i++) {
        const testData = group.data[i];

        test(`Case ${i + 1}: ${testData.desc}`, async ({ page }) => {
          // 通过 Ace Editor 的 API 注入代码
          await page.evaluate((codeContent) => {
            // @ts-ignore
            const editor = ace.edit("code");
            editor.setValue(codeContent);
          }, testData.code);

          // 点击提交运行按钮
          const submitBtn = page.locator('.btn-submit');
          await submitBtn.click();

          // 等待按钮文本从“评测中...”变回“提交运行”
          await expect(submitBtn).toHaveText('提交运行', { timeout: 30000 });

          // 获取并验证评测结果
          const resultArea = page.locator('#result-area');
          await expect(resultArea).toContainText(testData.expected);
        });
      }
    });
  }
});
