# 阿里云 Docker 容器化部署指南

本文档详细说明如何将 Load Balanced Online OJ 项目部署到阿里云服务器，采用全 Docker 容器化方案，确保环境隔离与数据一致性。

## 1. 部署架构

系统包含以下 Docker 容器：
- **oj_db**: MySQL 8.0 数据库，持久化存储
- **oj_server**: 核心 Web 服务与业务逻辑 (内置 React 前端静态资源)
- **compile_server_1 / 2**: 判题服务器集群
- **contest_crawler**: 竞赛爬虫服务

## 2. 环境要求

### 本地环境 (开发机)
- macOS / Linux
- Docker Desktop
- MySQL Client (`mysqldump` 工具)
- `rsync` 工具
- SSH 访问权限 (拥有 `.pem` 私钥)

### 远程服务器 (阿里云)
- Ubuntu 20.04/22.04 LTS
- Docker Engine & Docker Compose
- 安全组开放端口:
    - **8094**: Web 服务访问端口
    - **22**: SSH 远程连接端口
    - (可选) **8081-8082**: 仅调试用，生产环境建议关闭外部访问

## 3. 部署前配置

### 3.1 检查 Docker 配置
确认 `docker/docker-compose.yml` 配置正确。重点关注环境变量与端口映射。

### 3.2 配置部署脚本
编辑 `scripts/deploy_docker.sh`，根据实际情况修改以下变量：

```bash
SERVER_IP="47.121.117.192"       # 阿里云服务器公网 IP
USER="huangqijun"                # 服务器登录用户名 (需有 Docker 权限)
KEY_PATH="path/to/your/key.pem"  # 本地 SSH 私钥路径
REMOTE_DIR="~/load-balanced-online-oj" # 服务器部署目录
LOCAL_DB_USER="oj_client"        # 本地数据库用户名
LOCAL_DB_PASS="123456"           # 本地数据库密码
```

### 3.3 服务器权限设置
确保服务器用户 (`huangqijun`) 已加入 `docker` 用户组，以免执行 docker 命令时需要 sudo：

```bash
# 在服务器上执行
sudo usermod -aG docker $USER
newgrp docker
```

## 4. 执行部署

在本地项目根目录下执行一键部署脚本：

```bash
./scripts/deploy_docker.sh
```

### 脚本执行流程
1. **检查连接**: 验证 SSH 连接与密钥。
2. **环境检查**: 确认服务器已安装 Docker 环境。
3. **数据准备**: 
    - 使用 `mysqldump` 导出本地 `oj` 数据库全量数据（包含结构与数据）。
    - 确保 `full_db_dump.sql` 包含最新的题目、用户、提交记录。
4. **同步代码**: 
    - 使用 `rsync` 将本地代码（排除构建产物）同步至服务器。
    - 确保服务器代码版本与本地完全一致。
5. **容器构建与启动**:
    - 远程执行 `docker compose build` 构建最新镜像。
    - 执行 `docker compose up -d` 启动服务。
6. **数据恢复**:
    - **自动备份**: 部署前自动备份服务器现有数据库到 `oj_backup_YYYYMMDD_HHMMSS.sql`。
    - **数据导入**: 将本地导出的 SQL 导入到远程 `oj_db` 容器，覆盖现有数据，确保**数据强一致性**。

## 5. 验证部署

部署完成后，请进行以下验证：

### 5.1 服务状态检查
在服务器上执行：
```bash
docker compose ps
```
应显示所有容器状态为 `Up` (db 状态为 `healthy`)。

### 5.2 功能验证
访问浏览器：`http://47.121.117.192:8094`
1. **首页**: 确认能加载题目列表，题目数量应与本地一致。
2. **登录**: 使用本地已有账号登录，验证用户数据是否同步。
3. **提交**: 提交一道题目（如 "回文数"），验证判题服务是否正常（Docker 内部网络通讯）。
4. **讨论区**: 检查讨论区帖子和图片是否显示。

### 6. 前端构建说明 (Docker 自动处理)
 
 本项目采用了 **Docker Multi-stage Build** 技术。在 `docker/Dockerfile.oj` 中：
 1. **Stage 1 (frontend-builder)**: 使用 `node:18` 镜像，自动执行 `npm install` 和 `npm run build`，生成前端静态资源 (`dist`)。
 2. **Stage 2 (Final)**: 将 Stage 1 生成的 `dist` 目录复制到 `backend/oj_server/wwwroot`。
 
 因此，使用 Docker 部署时，**无需在本地或服务器上手动安装 Node.js 或构建前端**，容器构建过程会自动完成这一切。
 
 如果需要手动修改前端构建参数，请编辑 `frontend/vite.config.ts` 或 `docker/Dockerfile.oj`。

## 7. 回滚方案

如果部署后出现严重问题，可采用以下方案回滚：

### 方案 A: 恢复数据库
脚本会在每次覆盖数据前在服务器 `~/load-balanced-online-oj` 目录下生成备份文件 `oj_backup_....sql`。

恢复命令 (在服务器执行):
```bash
# 1. 进入目录
cd ~/load-balanced-online-oj

# 2. 找到最近的备份文件
ls -lt oj_backup_*.sql

# 3. 恢复数据
cat oj_backup_20260210_120000.sql | docker exec -i oj_db mysql -u root -proot_password
```

### 方案 B: 回滚代码版本
如果新代码有 Bug，可以在本地回退 git 版本，然后重新运行 `./scripts/deploy_docker.sh`。

## 8. 注意事项

- **数据覆盖**: 部署脚本会**强制覆盖**服务器上的数据库。如果服务器上有用户产生的新数据（如新注册用户、新提交），请先手动备份服务器数据，或修改脚本去掉数据恢复步骤。
- **构建时间**: 首次部署或依赖变更时，Docker 构建可能需要几分钟（包含前端构建和 C++ 编译）。
**文档版本**: v1.2.2
**最后更新时间**: 2026-03-10
**维护团队**: 在线评测系统开发团队
