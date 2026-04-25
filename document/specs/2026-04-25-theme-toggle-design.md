# Theme Toggle Feature Design Spec

## 1. Overview
Add a theme toggle feature to the Online Judge (OJ) platform to allow users to switch between light and dark modes. The system will default to the user's OS preference and remember manual overrides across sessions.

## 2. Architecture & Components
- **State Initialization Script**: A small inline JavaScript snippet placed in the `<head>` of HTML templates (e.g., `index.html`, `all_questions.html`, etc.) or in a shared head partial if one exists. This script executes synchronously to apply the `data-theme="light"` attribute to `document.documentElement` if the saved preference or OS preference is light. This prevents FOUC (Flash of Unstyled Content).
- **UI Toggle Component**: A button added to `oj_server/resources/template_html/shared/navbar.html`. The button will display a Sun icon when in dark mode and a Moon icon when in light mode.
- **Toggle Logic**: Handled in `oj_server/resources/template_html/js/common.js`. A function `toggleTheme()` will flip the `data-theme` attribute, update the icon, and save the preference to `localStorage`.

## 3. Data Flow & State Management
1. **Initial Load**:
   - Check `localStorage.getItem('theme')`.
   - If present, use it (`'light'` or `'dark'`).
   - If absent, check `window.matchMedia('(prefers-color-scheme: light)').matches`. If true, set to `'light'`, else `'dark'`.
   - Apply `data-theme` attribute to the root `<html>` element.
2. **User Interaction**:
   - User clicks the theme toggle button in the navbar.
   - The system checks the current `data-theme` attribute.
   - If current is `'light'`, switch to `'dark'` (and vice versa).
   - Update `localStorage.setItem('theme', newTheme)`.
   - Update the button icon to match the new theme.

## 4. UI/UX Details
- The toggle button will be placed in the `.navbar-auth` section, right before the user profile/login links.
- The button will use simple SVG icons (Sun/Moon) to visually indicate the current state and the action.
- CSS transitions will be applied to the `background-color` and `color` properties in `common.css` to make the theme switch smooth.

## 5. Error Handling & Edge Cases
- **No `localStorage` Support**: Fallback to OS preference, but toggling will only last for the current page session.
- **FOUC Prevention**: Ensure the initialization script runs *before* the DOM is fully parsed and rendered.

## 6. Testing Plan
- Manually verify that the OS preference is respected on first load (no `localStorage` set).
- Verify that clicking the toggle button switches the theme and updates the icon.
- Verify that refreshing the page retains the manually selected theme without flashing the wrong theme.
- Verify the layout and alignment of the new button in the navbar across different screen sizes.