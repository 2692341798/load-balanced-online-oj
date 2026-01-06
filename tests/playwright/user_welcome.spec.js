const { test, expect } = require('@playwright/test');

test('homepage shows login/register when not logged in', async ({ page }) => {
  await page.goto('http://localhost:8080/');
  const loginLink = page.locator('.navbar .login');
  await expect(loginLink).toHaveText(/登录\/注册|加载中/);
});

test('all_questions redirects to login when not logged in', async ({ page }) => {
  const response = await page.goto('http://localhost:8080/all_questions');
  expect(response.url()).toMatch(/\/login$/);
});

test('question page redirects to login when not logged in', async ({ page }) => {
  const response = await page.goto('http://localhost:8080/question/1');
  expect(response.url()).toMatch(/\/login$/);
});

test('api user returns status=1 when not logged in', async ({ request }) => {
  const res = await request.get('http://localhost:8080/api/user');
  expect(res.ok()).toBeTruthy();
  const data = await res.json();
  expect(data.status).toBe(1);
  expect(typeof data.reason).toBe('string');
});

