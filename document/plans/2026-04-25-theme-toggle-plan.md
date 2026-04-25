# Theme Toggle Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a dark/light mode theme toggle that defaults to OS preference, saves user choice to `localStorage`, and prevents FOUC.

**Architecture:** 
- A synchronous initialization script added to the top of `common.js` (which is loaded in the `<head>` of all pages) applies the correct theme before the DOM renders.
- A toggle button is added to `navbar.html` outside the dynamic `auth-container`.
- The logic updates `localStorage` and the `data-theme` attribute on the `<html>` element.
- CSS transitions ensure smooth switching.

**Tech Stack:** HTML, Vanilla JavaScript, CSS variables

---

### Task 1: Update CSS for Theme Toggle Button

**Files:**
- Modify: `/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/oj_server/resources/template_html/css/nav.css`
- Modify: `/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/oj_server/resources/template_html/css/common.css`

- [ ] **Step 1: Add flex container for right side of navbar**
Modify `nav.css` to add `.navbar-right` class.

```css
.navbar-right {
    display: flex;
    align-items: center;
    gap: 20px;
}
```

- [ ] **Step 2: Add button and icon styles to common.css**
Modify `common.css` to add `.btn-icon` styles and theme-specific icon display rules. Add this to the bottom of the file or near `.btn-text`.

```css
.btn-icon {
    background: transparent;
    border: none;
    color: var(--text-main);
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 8px;
    border-radius: 50%;
    transition: background-color 0.3s, color 0.3s;
}

.btn-icon:hover {
    background-color: rgba(128, 128, 128, 0.2);
}

.btn-icon svg {
    width: 20px;
    height: 20px;
    fill: currentColor;
}

/* Hide/show specific icons based on current theme */
html[data-theme="light"] .icon-moon {
    display: block;
}
html[data-theme="light"] .icon-sun {
    display: none;
}

html:not([data-theme="light"]) .icon-moon {
    display: none;
}
html:not([data-theme="light"]) .icon-sun {
    display: block;
}
```

### Task 2: Add Theme Toggle Script Logic

**Files:**
- Modify: `/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/oj_server/resources/template_html/js/common.js`

- [ ] **Step 1: Add Initialization Script at the very top of the file**
Add this snippet at line 1, before any other logic, to execute synchronously during parsing.

```javascript
// Theme Initialization (Prevents FOUC)
(function() {
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme === 'light' || savedTheme === 'dark') {
        if (savedTheme === 'light') {
            document.documentElement.setAttribute('data-theme', 'light');
        } else {
            document.documentElement.removeAttribute('data-theme');
        }
    } else {
        // Fallback to OS preference
        if (window.matchMedia && window.matchMedia('(prefers-color-scheme: light)').matches) {
            document.documentElement.setAttribute('data-theme', 'light');
        }
    }
})();
```

- [ ] **Step 2: Add Theme Toggle Function and Event Listener**
Add the following functions near the end of `common.js`, inside the existing `window.addEventListener('DOMContentLoaded', () => { ... })` block.

```javascript
    // Theme Toggle Logic
    const themeToggleBtn = document.getElementById('theme-toggle');
    if (themeToggleBtn) {
        themeToggleBtn.addEventListener('click', () => {
            const currentTheme = document.documentElement.getAttribute('data-theme');
            let newTheme = 'dark';
            if (currentTheme !== 'light') {
                document.documentElement.setAttribute('data-theme', 'light');
                newTheme = 'light';
            } else {
                document.documentElement.removeAttribute('data-theme');
            }
            localStorage.setItem('theme', newTheme);
        });
    }
```

### Task 3: Update Navbar HTML

**Files:**
- Modify: `/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/oj_server/resources/template_html/shared/navbar.html`

- [ ] **Step 1: Wrap auth-container and add the toggle button**
Replace the current `<div class="navbar-auth" id="auth-container">...</div>` structure by wrapping it inside `.navbar-right` and adding the theme toggle button.

```html
    <div class="navbar-right">
        <button id="theme-toggle" class="btn-icon" aria-label="Toggle Theme">
            <!-- Sun Icon (shown in dark mode) -->
            <svg class="icon-sun" viewBox="0 0 24 24">
                <path d="M12 7c-2.76 0-5 2.24-5 5s2.24 5 5 5 5-2.24 5-5-2.24-5-5-5zM2 13h2c.55 0 1-.45 1-1s-.45-1-1-1H2c-.55 0-1 .45-1 1s.45 1 1 1zm18 0h2c.55 0 1-.45 1-1s-.45-1-1-1h-2c-.55 0-1 .45-1 1s.45 1 1 1zM11 2v2c0 .55.45 1 1 1s1-.45 1-1V2c0-.55-.45-1-1-1s-1 .45-1 1zm0 18v2c0 .55.45 1 1 1s1-.45 1-1v-2c0-.55-.45-1-1-1s-1 .45-1 1zM5.99 4.58c-.39-.39-1.03-.39-1.41 0-.39.39-.39 1.03 0 1.41l1.06 1.06c.39.39 1.03.39 1.41 0 .39-.39.39-1.03 0-1.41L5.99 4.58zm12.37 12.37c-.39-.39-1.03-.39-1.41 0-.39.39-.39 1.03 0 1.41l1.06 1.06c.39.39 1.03.39 1.41 0 .39-.39.39-1.03 0-1.41l-1.06-1.06zm1.06-10.96c.39-.39.39-1.03 0-1.41-.39-.39-1.03-.39-1.41 0l-1.06 1.06c-.39.39-.39 1.03 0 1.41.39.39 1.03.39 1.41 0l1.06-1.06zM7.05 18.36c.39-.39.39-1.03 0-1.41-.39-.39-1.03-.39-1.41 0l-1.06 1.06c-.39.39-.39 1.03 0 1.41.39.39 1.03.39 1.41 0l1.06-1.06z"></path>
            </svg>
            <!-- Moon Icon (shown in light mode) -->
            <svg class="icon-moon" viewBox="0 0 24 24">
                <path d="M12 3c-4.97 0-9 4.03-9 9s4.03 9 9 9 9-4.03 9-9c0-.46-.04-.92-.1-1.36-.98 1.37-2.58 2.26-4.4 2.26-3.03 0-5.5-2.47-5.5-5.5 0-1.82.89-3.42 2.26-4.4-.44-.06-.9-.1-1.36-.1z"></path>
            </svg>
        </button>
        <div class="navbar-auth" id="auth-container">
            {{#user_logged_in}}
            <div class="user-profile" onclick="toggleUserMenu(event)">
                <div class="user-avatar">
                    {{#has_avatar}}<img src="{{avatar_url}}" alt="{{username}}" style="width:100%;height:100%;object-fit:cover;border-radius:50%;" onerror="this.style.display='none';this.parentNode.innerText='{{username}}'.charAt(0).toUpperCase();">{{/has_avatar}}
                    {{#no_avatar}}<span>{{username}}</span>{{/no_avatar}}
                </div>
                <span>{{username}}</span>
            </div>
            <div class="user-menu-backdrop" id="user-menu-backdrop" onclick="hideUserMenu()"></div>
            <div class="user-menu" id="user-menu">
                <a href="/profile" class="user-menu-item">个人中心</a>
                <button type="button" class="user-menu-item" onclick="handleLogout(event)">退出登录</button>
            </div>
            {{/user_logged_in}}
            {{#user_not_logged_in}}
            <a href="/login" class="btn-text">登录 / 注册</a>
            {{/user_not_logged_in}}
        </div>
    </div>
```