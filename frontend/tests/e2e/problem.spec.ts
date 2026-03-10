import { test, expect } from '@playwright/test';

test.describe('Problem Browsing and Detail', () => {
  test('should list problems and navigate to detail', async ({ page }) => {
    await page.goto('/problems');
    
    // Check if problems are listed
    const problemLink = page.locator('a[href^="/problem/"]').first();
    await expect(problemLink).toBeVisible();
    
    const problemTitle = await problemLink.innerText();
    
    // Navigate to detail
    await problemLink.click();
    
    // Check detail page
    await expect(page).toHaveURL(/\/problem\/\d+/);
    await expect(page.locator('h1')).toContainText(problemTitle);
    
    // Check editor existence (Monaco editor usually has class 'monaco-editor')
    await expect(page.locator('.monaco-editor')).toBeVisible();
  });
});
