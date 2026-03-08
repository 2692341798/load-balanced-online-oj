# 训练卡片点击导致应用卡死问题修复报告

## 问题描述
用户反馈在点击训练题单卡片（或浏览题单列表）时，应用程序（浏览器标签页）会发生卡死/无响应现象。

## 根本原因分析
经过排查，发现问题根源在于前端 `img` 标签的 `onerror` 处理逻辑存在无限循环漏洞：
1. `training_list.html` 和 `training_detail.html` 中，当作者头像加载失败时，`onerror` 事件被触发。
2. `onerror` 处理函数尝试将图片 `src` 设置为 `/images/default_avatar.png`。
3. 由于服务器上缺少 `default_avatar.png` 文件，导致该备用图片也加载失败。
4. 加载失败再次触发 `onerror` 事件，再次尝试设置相同的 `src`，从而陷入无限递归调用。
5. 浏览器的事件循环被阻塞，导致页面完全卡死。

## 修复方案

### 1. 资源补充
- 在 `oj_server/resources/wwwroot/images/` 目录下添加了缺失的 `default_avatar.png` 文件（从现有游戏资源中复制）。
- 确保构建脚本 (`make output`) 能够正确将该资源复制到发布目录。

### 2. 代码加固（错误边界）
- 修改前端模板文件中的 `onerror` 处理逻辑，增加了防递归机制：
  ```html
  <!-- 修改前 -->
  <img ... onerror="this.src='/images/default_avatar.png'">
  
  <!-- 修改后 -->
  <img ... onerror="this.onerror=null;this.src='/images/default_avatar.png'">
  ```
- 通过 `this.onerror=null`，确保如果备用图片也加载失败，不会再次触发错误处理，从而打破无限循环。

### 3. 受影响文件
- `oj_server/resources/template_html/training_list.html`
- `oj_server/resources/template_html/training_detail.html`
- `oj_server/resources/wwwroot/images/default_avatar.png` (新增)

## 验证结果
- **资源验证**: 访问 `http://localhost:8088/images/default_avatar.png` 返回 200 OK。
- **逻辑验证**: 代码审查确认 `onerror` 属性已包含 `this.onerror=null`。
- **回归测试**: 即使删除了默认头像文件，新的代码逻辑也会阻止浏览器卡死（图片会显示为破碎图标，但页面保持响应）。
