function renderSidebar(activePage) {
    const sidebarHtml = `
    <div class="sidebar">
        <div class="sidebar-header">
            <h3>管理后台</h3>
        </div>
        <ul class="sidebar-menu">
            <li><a href="index.html" class="${activePage === 'dashboard' ? 'active' : ''}">仪表盘</a></li>
            <li><a href="problems.html" class="${activePage === 'problems' ? 'active' : ''}">题目管理</a></li>
            <li><a href="users.html" class="${activePage === 'users' ? 'active' : ''}">用户管理</a></li>
            <li><a href="logs.html" class="${activePage === 'logs' ? 'active' : ''}">系统日志</a></li>
            <li><a href="#" onclick="toggleTheme(event)" id="theme-toggle-btn">切换主题 🌓</a></li>
            <li><a href="#" onclick="logout(event)">退出登录</a></li>
        </ul>
    </div>
    `;

    document.body.insertAdjacentHTML('afterbegin', sidebarHtml);
    initTheme();
}

function initTheme() {
    const savedTheme = localStorage.getItem('theme');
    const prefersLight = window.matchMedia('(prefers-color-scheme: light)').matches;
    
    if (savedTheme) {
        document.documentElement.setAttribute('data-theme', savedTheme);
        updateThemeButton(savedTheme);
    } else if (prefersLight) {
        document.documentElement.setAttribute('data-theme', 'light');
        updateThemeButton('light');
    } else {
        document.documentElement.setAttribute('data-theme', 'dark');
        updateThemeButton('dark');
    }
}

function toggleTheme(event) {
    if(event) event.preventDefault();
    const currentTheme = document.documentElement.getAttribute('data-theme') || 'dark';
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
    
    document.documentElement.setAttribute('data-theme', newTheme);
    localStorage.setItem('theme', newTheme);
    updateThemeButton(newTheme);
    
    // 如果存在 Chart.js 图表，重新刷新页面以重新渲染图表颜色
    if (typeof Chart !== 'undefined' && window.location.pathname.includes('index.html')) {
        window.location.reload();
    }
}

function updateThemeButton(theme) {
    const btn = document.getElementById('theme-toggle-btn');
    if (btn) {
        btn.innerHTML = theme === 'dark' ? '切换亮色 ☀️' : '切换暗色 🌙';
    }
}

async function logout(event) {
    if(event) event.preventDefault();
    try {
        await fetch('/api/logout', { method: 'GET' });
        window.location.href = '/admin/login.html';
    } catch (e) {
        console.error('Logout failed', e);
        window.location.href = '/admin/login.html';
    }
}
