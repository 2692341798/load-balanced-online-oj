import { test, expect } from '@playwright/test';

test.describe('Submission Result UI', () => {
  test.beforeEach(async ({ page }) => {
    // Mock User Auth (Login state)
    await page.route('**/api/user', async route => {
      await route.fulfill({
        status: 200,
        contentType: 'application/json',
        body: JSON.stringify({
          status: 0,
          username: "admin123",
          email: "admin@example.com",
          nickname: "Admin",
          avatar: "/uploads/avatars/default.png",
          role: 1
        })
      });
    });

    // Mock Problem Detail API
    await page.route('**/api/question/1', async route => {
      await route.fulfill({
        status: 200,
        contentType: 'application/json',
        body: JSON.stringify({
          status: 0,
          data: {
            number: "1",
            title: "Two Sum",
            desc: "Given an array of integers nums and an integer target, return indices of the two numbers such that they add up to target.",
            star: "简单",
            cpu_limit: 1,
            mem_limit: 1024,
            header: "class Solution { public: vector<int> twoSum(vector<int>& nums, int target) { } };",
            tail: "[]"
          }
        })
      });
    });
  });

  test('should display compilation result panel on submission', async ({ page }) => {
    // Mock Judge API
    // Note: ProblemDetail.tsx uses baseURL: '/' for judge endpoint, so it hits /judge/1 directly (proxied by Vite)
    await page.route('**/judge/1', async route => {
      await route.fulfill({
        status: 200,
        contentType: 'application/json',
        body: JSON.stringify({
          status: 0,
          stdout: JSON.stringify({
            cases: [
              { name: "Case 1", pass: true, input: "[2,7,11,15]\n9", output: "[0,1]", expected: "[0,1]" },
              { name: "Case 2", pass: false, input: "[3,2,4]\n6", output: "[0,0]", expected: "[1,2]" }
            ],
            summary: { overall: "1/2 Passed", total: 2, passed: 1 }
          }),
          stderr: ""
        })
      });
    });

    // Navigate to problem page
    await page.goto('/problem/1');

    // Verify page loaded
    await expect(page.getByText('Two Sum')).toBeVisible();

    // Click submit
    await page.getByRole('button', { name: /Submit|提交/ }).click();

    // Verify Result Panel appears
    // "Wrong Answer" or "解答错误" based on locale.
    await expect(page.getByText(/Wrong Answer|解答错误/)).toBeVisible();
    
    // Check Case 1 (Pass)
    // Using regex to match "Case 1" or "用例 1"
    await page.getByText(/Case 1|用例 1/).click();
    await expect(page.getByText('[0,1]')).toHaveCount(2); // Output and Expected are same

    // Check Case 2 (Fail)
    await page.getByText(/Case 2|用例 2/).click();
    await expect(page.getByText('[0,0]')).toBeVisible(); // Output
    await expect(page.getByText('[1,2]')).toBeVisible(); // Expected
  });

  test('should display compile error', async ({ page }) => {
    // Mock Judge API for Compile Error
    await page.route('**/judge/1', async route => {
      await route.fulfill({
        status: 200,
        contentType: 'application/json',
        body: JSON.stringify({
          status: 1, // Compile Error status from backend might be non-zero
          reason: "Compile Error",
          stderr: "solution.cpp:10:5: error: expected ';' before 'return'"
        })
      });
    });

    await page.goto('/problem/1');
    await page.getByRole('button', { name: /Submit|提交/ }).click();
    
    // Check status header
    await expect(page.locator('span.text-red-500').filter({ hasText: /Compile Error|编译错误/ })).toBeVisible();
    
    // Check error details
    await expect(page.getByText("solution.cpp:10:5: error: expected ';' before 'return'")).toBeVisible();
  });

  test('should keep left-right main layout and top-bottom workspace layout without overlap', async ({ page }) => {
    await page.route('**/judge/1', async route => {
      await route.fulfill({
        status: 200,
        contentType: 'application/json',
        body: JSON.stringify({
          status: 0,
          stdout: JSON.stringify({
            cases: [
              { name: "Case 1", pass: true, input: "1", output: "1", expected: "1" },
              { name: "Case 2", pass: true, input: "2", output: "2", expected: "2" }
            ],
            summary: { overall: "All Passed", total: 2, passed: 2 }
          }),
          stderr: ""
        })
      });
    });

    for (const viewport of [{ width: 1366, height: 900 }, { width: 1024, height: 900 }, { width: 768, height: 900 }]) {
      await page.setViewportSize(viewport);
      await page.goto('/problem/1');
      await page.getByRole('button', { name: /Submit|提交/ }).click();

      if (viewport.width >= 1024) {
        const preferredHandle = page.getByTestId('main-layout-handle');
        const mainHandle = (await preferredHandle.count()) > 0
          ? preferredHandle.first()
          : page.locator('[role="separator"]').first();
        const handleBox = await mainHandle.boundingBox();
        if (handleBox) {
          const startX = handleBox.x + handleBox.width / 2;
          const y = handleBox.y + handleBox.height / 2;
          await page.mouse.move(startX, y);
          await page.mouse.down();
          await page.mouse.move(viewport.width * 0.22, y);
          await page.mouse.up();
        }
      }

      const descriptionPanel = page.getByTestId('problem-description-panel');
      const editorPanel = page.getByTestId('editor-panel');
      const resultPanel = page.getByTestId('result-panel');

      await expect(descriptionPanel).toBeVisible();
      await expect(editorPanel).toBeVisible();
      await expect(resultPanel).toBeVisible();

      const descriptionBox = await descriptionPanel.boundingBox();
      const editorBox = await editorPanel.boundingBox();
      const resultBox = await resultPanel.boundingBox();

      expect(descriptionBox).not.toBeNull();
      expect(editorBox).not.toBeNull();
      expect(resultBox).not.toBeNull();

      if (!descriptionBox || !editorBox || !resultBox) {
        throw new Error('Panel bounding box is missing');
      }

      const intersectionArea = (boxA: { x: number; y: number; width: number; height: number }, boxB: { x: number; y: number; width: number; height: number }) => {
        const overlapWidth = Math.max(
          0,
          Math.min(boxA.x + boxA.width, boxB.x + boxB.width) - Math.max(boxA.x, boxB.x)
        );
        const overlapHeight = Math.max(
          0,
          Math.min(boxA.y + boxA.height, boxB.y + boxB.height) - Math.max(boxA.y, boxB.y)
        );
        return overlapWidth * overlapHeight;
      };

      expect(intersectionArea(descriptionBox, editorBox)).toBeLessThanOrEqual(1);
      expect(intersectionArea(descriptionBox, resultBox)).toBeLessThanOrEqual(1);
      expect(intersectionArea(editorBox, resultBox)).toBeLessThanOrEqual(1);
      expect(editorBox.y + editorBox.height).toBeLessThanOrEqual(resultBox.y + 1);

      if (viewport.width >= 1024) {
        expect(descriptionBox.x + descriptionBox.width).toBeLessThanOrEqual(editorBox.x + 1);
      }
    }
  });
});
