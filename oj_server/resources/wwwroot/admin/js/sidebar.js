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
            <li><a href="#" onclick="logout(event)">退出登录</a></li>
        </ul>
    </div>
    `;

    document.body.insertAdjacentHTML('afterbegin', sidebarHtml);
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
