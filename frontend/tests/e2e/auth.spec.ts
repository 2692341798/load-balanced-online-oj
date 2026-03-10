import { test, expect } from '@playwright/test';

test.describe('Authentication Flow', () => {
  // Generate a shorter unique username (max 20 chars allowed by backend)
  // Date.now() is 13 digits. "u" + timestamp = 14 chars. Safe.
  const username = `u${Date.now()}`; 
  const password = 'testpassword123';
  const email = `${username}@example.com`;

  test('should register a new user', async ({ page }) => {
    await page.goto('/register');
    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', password);
    await page.fill('input[name="confirmPassword"]', password);
    await page.fill('input[name="email"]', email);
    
    // Assuming the register button has type="submit"
    await page.click('button[type="submit"]');
    
    // Check for potential error message if redirection fails
    try {
      await expect(page).toHaveURL(/\/login/, { timeout: 5000 });
    } catch (e) {
      // If timeout, check if there is an error toast
      const toast = page.locator('[role="status"]'); // Radix UI toast usually has role="status" or similar
      if (await toast.isVisible()) {
        const errorText = await toast.innerText();
        console.log('Registration failed with toast:', errorText);
        throw new Error(`Registration failed: ${errorText}`);
      }
      throw e;
    }
  });

  test('should login with the registered user', async ({ page }) => {
    await page.goto('/login');
    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', password);
    await page.click('button[type="submit"]');
    
    // Expect redirect to home and user avatar/menu presence
    await expect(page).toHaveURL('/');
    await expect(page.getByRole('button', { name: username.charAt(0).toUpperCase() })).toBeVisible();
  });
});
