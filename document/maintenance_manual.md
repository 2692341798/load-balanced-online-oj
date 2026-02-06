# 冻梨OJ 项目维护手册

## 1. 服务器连接信息

### 基本信息
- **公网 IP**: `<Server_IP>`
- **用户名**: `root`
- **操作系统**: Ubuntu / Aliyun Linux (兼容)

### 密钥信息
- **私钥文件路径**: `<Key_Path>`
- **密钥指纹**: `<Key_Fingerprint>`

### 连接方法
1. **权限设置** (首次使用或权限错误时执行):
   为了安全起见，私钥文件必须仅对所有者可读。
   ```bash
   chmod 400 <Key_Path>
   ```

2. **SSH 连接命令**:
   ```bash
   ssh -i <Key_Path> root@<Server_IP>
   ```

### 安全注意事项
- **严禁泄露私钥文件** (`<Key_Name>`)，建议在本地做好备份。
- 登录服务器后，避免在非必要情况下修改 `/root/.ssh/authorized_keys` 文件。
- 建议定期检查服务器登录日志 `/var/log/auth.log` 以监控异常访问。

---

## 2. 部署架构说明

### 目录结构
项目部署在服务器的 `/root/oj_project` 目录下：
```
/root/oj_project/
├── docker/                  # Docker部署目录
│   ├── docker-compose.yml   # 容器编排文件
│   ├── Dockerfile.oj        # OJ Server 构建文件
│   └── Dockerfile.compile   # Compile Server 构建文件
├── setup_database.sql       # 数据库初始化脚本
├── insert_questions.sql     # 题目数据初始化脚本
├── oj_server/               # OJ Server 源码及配置
├── compile_server/          # Compile Server 源码
└── ...
```

### 服务组件配置

该项目基于 Docker Compose 部署，包含以下服务：

| 服务名称 | 容器名称 | 端口映射 | 说明 |
| :--- | :--- | :--- | :--- |
| **oj_server** | `oj_server` | `8080:8080` | 核心 Web 服务，提供 API 和前端页面 |
| **compile_server_1** | `compile_server_1` | `8081:8081` | 判题服务节点 1 |
| **compile_server_2** | `compile_server_2` | `8082:8082` | 判题服务节点 2 |
| **db** | `oj_db` | `3306:3306` | MySQL 8.0 数据库 |

### 依赖环境
- **Docker**: 20.10+
- **Docker Compose**: 2.0+

### 运行状态检查
在 `/root/oj_project/docker` 目录下执行：
```bash
docker compose ps
```
所有服务状态应为 `Up` (其中 db 应为 `Up (healthy)`).

---

## 3. 更新维护流程

### 3.1 代码更新与打包 (本地操作)
1. **本地修改代码**并测试通过。
2. **打包项目** (排除不必要文件):
   ```bash
   cd <Local_Project_Path>
   rm -f project_deploy.tar.gz
   tar --exclude='./output' --exclude='./.git' --exclude='./.vscode' --exclude='./.trae' -czf project_deploy.tar.gz .
   ```

### 3.2 上传与部署 (本地 -> 远程)
1. **上传压缩包**:
   ```bash
   scp -i <Key_Path> project_deploy.tar.gz root@<Server_IP>:/root/
   ```

2. **解压并重构服务**:
   ```bash
   ssh -i <Key_Path> root@<Server_IP> "tar -xzf project_deploy.tar.gz -C oj_project && cd oj_project/docker && docker compose up -d --build"
   ```
   > **注意**: 如果修改了数据库结构或需要重置数据，可添加 `--force-recreate` 参数，或先执行 `docker compose down -v` (慎用，会清空数据)。

### 3.3 健康检查
部署完成后，执行以下命令验证服务日志：
```bash
ssh -i <Key_Path> root@<Server_IP> "cd oj_project/docker && docker compose logs --tail 20"
```

### 3.4 自动化部署 (GitHub Actions)
本项目已配置 GitHub Actions 工作流 (`.github/workflows/deploy.yml`)，当代码推送到 `main` 分支时自动触发部署。

#### 配置步骤
1.  **进入 GitHub 仓库设置**:
    - Settings -> Secrets and variables -> Actions -> New repository secret

2.  **添加以下 Secrets**:

| Secret Name | Value Example | 说明 |
| :--- | :--- | :--- |
| `SERVER_HOST` | `47.121.117.192` | 服务器公网 IP |
| `SERVER_USER` | `root` | 登录用户名 |
| `SSH_PRIVATE_KEY` | `-----BEGIN OPENSSH PRIVATE KEY-----...` | 私钥文件内容 (完整复制) |

> **重要提示**: GitHub Actions 服务器位于海外，且 IP 动态变化。请务必在云服务器**安全组**中放行 **TCP:22** 端口，源 IP 设置为 `0.0.0.0/0` (或所有 IP)，否则会出现 `dial tcp ...:22: i/o timeout` 连接超时错误。

3.  **验证**:
    - 推送代码到 GitHub: `git push origin main`
    - 查看 Actions 选项卡下的构建日志。

---

## 4. 监控告警机制

### 系统资源监控
- **CPU/内存**: 使用 `htop` 或 `docker stats` 查看容器资源占用。
  ```bash
  docker stats
  ```
- **磁盘空间**:
  ```bash
  df -h
  ```

### 应用服务监控
- **日志监控**:
  - OJ Server: `docker compose logs -f oj_server`
  - Database: `docker compose logs -f db`
- **可用性检查**:
  - 定期访问 `http://<Server_IP>:8080/all_questions` 确认页面加载正常。

---

## 5. 备份恢复策略

### 数据库备份
建议设置 Crontab 定时任务，每日备份数据库。
**手动备份命令**:
```bash
# 在服务器上执行
docker exec oj_db mysqldump -u root -p<DB_Root_Password> oj > /root/oj_backup_$(date +%Y%m%d).sql
```

### 配置文件备份
项目核心配置位于 `docker-compose.yml` 和 `oj_server/conf/`，建议通过 Git 进行版本控制，服务器上保留一份最新的 `project_deploy.tar.gz` 即可。

### 灾难恢复
1. **服务崩溃**: 尝试重启服务 `docker compose restart`.
2. **数据损坏**:
   - 停止服务: `docker compose down`
   - 恢复数据: 将备份 SQL 导入数据库 (需重新启动 db 容器并手动导入)。
   - 重新部署: `docker compose up -d`

---

## 6. 安全维护规范

1. **防火墙/安全组**:
   - 仅开放必要端口: `8080` (Web), `22` (SSH).
   - `8081`, `8082`, `3306` 建议仅限内网访问或开发调试时临时开放。
2. **数据库安全**:
   - 生产环境建议修改默认密码 (`<Default_DB_Password>`)。
   - 修改 `docker-compose.yml` 中的 `MYSQL_PASSWORD` 和 `oj_server` 环境变量。
3. **系统更新**:
   - 定期运行 `apt update && apt upgrade` 更新服务器系统补丁。

---

## 7. 故障排查指南

| 症状 | 可能原因 | 排查步骤 |
| :--- | :--- | :--- |
| **网站无法访问** | 服务未启动 / 防火墙拦截 | 1. `docker compose ps` 检查状态<br>2. 阿里云控制台检查安全组规则 |
| **"获取题目失败"** | 数据库连接失败 / 表缺失 | 1. `docker compose logs oj_server` 看报错<br>2. `docker compose logs db` 看数据库状态<br>3. 检查 `setup_database.sql` 是否执行 |
| **判题一直 Pending** | 编译服务器离线 | 1. 检查 `compile_server` 容器状态<br>2. 检查 `service_machine_docker.conf` 配置 |
| **SSH 连接拒绝** | 密钥错误 / 权限错误 | 1. 确认使用 `<Key_Name>`<br>2. 确认权限为 `400`<br>3. 检查 IP 是否变更 |

**应急联系**: [请在此处填写负责人联系方式]

---

## 8. 文档版本管理

| 版本号 | 更新日期 | 修改人 | 修改内容 |
| :--- | :--- | :--- | :--- |
| v1.0.0 | 2026-02-06 | Trae Assistant | 初始版本创建，包含 Docker 部署与维护全流程 |
