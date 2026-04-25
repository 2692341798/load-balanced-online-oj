// Theme Initialization (Prevents FOUC)
(function() {
    const savedTheme = localStorage.getItem("theme");
    let theme = "dark";
    if (savedTheme === "light" || savedTheme === "dark") {
        theme = savedTheme;
    } else if (window.matchMedia && window.matchMedia("(prefers-color-scheme: light)").matches) {
        theme = "light";
    }
    document.documentElement.setAttribute("data-theme", theme);
})();
// Common Frontend Logic for OJ

// XSS Protection
function escapeHtml(text) {
    const map = { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#039;' };
    return text ? String(text).replace(/[&<>"']/g, function(m) { return map[m]; }) : '';
}

// User Menu Logic
function hideUserMenu() {
    const menu = document.getElementById('user-menu');
    const backdrop = document.getElementById('user-menu-backdrop');
    if (menu) menu.classList.remove('visible');
    if (backdrop) backdrop.classList.remove('visible');
    document.removeEventListener('click', onUserMenuDocumentClick);
}

function onUserMenuDocumentClick(event) {
    const profile = document.querySelector('.user-profile');
    const menu = document.getElementById('user-menu');
    if (!profile || !menu) {
        hideUserMenu();
        return;
    }
    if (!profile.contains(event.target) && !menu.contains(event.target)) {
        hideUserMenu();
    }
}

function toggleUserMenu(event) {
    event.stopPropagation();
    const menu = document.getElementById('user-menu');
    const backdrop = document.getElementById('user-menu-backdrop');
    if (!menu || !backdrop) return;
    
    const isVisible = menu.classList.contains('visible');
    if (isVisible) {
        hideUserMenu();
    } else {
        menu.classList.add('visible');
        backdrop.classList.add('visible');
        document.addEventListener('click', onUserMenuDocumentClick);
    }
}

async function handleLogout(event) {
    event.stopPropagation();
    try {
        const response = await fetch('/api/logout', { method: 'GET' });
        const data = await response.json();
        if (data.status === 0) {
            window.location.reload();
        } else {
            alert('登出失败：' + (data.reason || '未知错误'));
        }
    } catch (e) {
        alert('登出请求失败，请稍后重试');
    }
}

function updateNavAuth(data) {
    const authContainer = document.getElementById('auth-container');
    if (!authContainer) return;
    
    if (data.status === 0 && data.username) {
        // If needed, update global user variable if exists
        if (typeof currentUser !== 'undefined') currentUser = data;
        
        let avatarHtml = '';
        if (data.avatar) {
            avatarHtml = `<img src="${data.avatar}" alt="${escapeHtml(data.username)}" style="width:100%;height:100%;object-fit:cover;border-radius:50%;" onerror="this.style.display='none';this.parentNode.innerText='${data.username.charAt(0).toUpperCase()}';">`;
        } else {
            avatarHtml = data.username.charAt(0).toUpperCase();
        }

        authContainer.innerHTML = `
            <div class="user-profile" onclick="toggleUserMenu(event)">
                <div class="user-avatar">${avatarHtml}</div>
                <span>${escapeHtml(data.username)}</span>
            </div>
            <div class="user-menu-backdrop" id="user-menu-backdrop" onclick="hideUserMenu()"></div>
            <div class="user-menu" id="user-menu">
                <a href="/profile" class="user-menu-item">个人中心</a>
                <button type="button" class="user-menu-item" onclick="handleLogout(event)">退出登录</button>
            </div>
        `;
    } else {
        authContainer.innerHTML = `<a href="/login" class="btn-text">登录 / 注册</a>`;
    }
}

// Global initialization
window.addEventListener('DOMContentLoaded', () => {
    // Initial Auth Check (Client-side hydration)
    // Only if auth-container exists and we want to refresh/verify state
    const authContainer = document.getElementById('auth-container');
    if (authContainer) {
        fetch('/api/user')
            .then(res => res.json())
            .then(data => {
                // Only update if client-side check differs or for hydration
                // Since we use SSR for initial state, we might not strictly need this 
                // unless we want to handle session expiry or client-side caching issues.
                // However, existing code does it, so we keep it but maybe we can check if it's already rendered?
                // For now, we just call it to be safe and consistent with previous behavior.
                updateNavAuth(data); 
            })
            .catch(err => {
                console.error('Auth check failed:', err);
                updateNavAuth({status: -1});
            });
    }
    
    // Avatar text fallback for any avatars on page
    const avatarEls = document.querySelectorAll('.user-avatar');
    avatarEls.forEach(avatarEl => {
         if (!avatarEl.querySelector('img')) {
            const username = avatarEl.innerText.trim();
            if(username) avatarEl.innerText = username.charAt(0).toUpperCase();
        }
    });

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
                document.documentElement.setAttribute('data-theme', 'dark');
            }
            localStorage.setItem('theme', newTheme);
        });
    }
});
