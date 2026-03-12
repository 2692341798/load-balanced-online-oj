# Docker 容器化部署指南

本文档详细说明如何使用 Docker 和 Docker Compose 部署负载均衡在线评测系统。该方案实现了前后端完全分离，支持一键部署。

## 1. 部署架构

- **oj_frontend**: Nginx 容器，托管 React 静态资源，并反向代理 API 请求。
- **oj_server**: 后端 API 服务，运行在内部网络，不暴露对外端口。
- **compile_server**: 两个编译服务器实例，负责代码沙箱执行。
- **oj_db**: MySQL 8.0 数据库。
- **oj_redis**: Redis 缓存服务。
- **contest_crawler**: 竞赛题目爬虫服务。

## 2. 环境要求

### 本地开发 (macOS/Windows)
- 安装 [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- 确保 Docker 服务已启动

### 服务器部署 (Linux - Ubuntu/CentOS)
- 安装 Docker Engine:
  ```bash
  curl -fsSL https://get.docker.com | sh
  ```
- 安装 Docker Compose (如果未包含在 Docker 中):
  ```bash
  sudo apt-get install docker-compose-plugin
  ```

## 3. 快速部署

在项目根目录下，运行自动化部署脚本：

```bash
chmod +x scripts/deploy_docker.sh
./scripts/deploy_docker.sh
```

脚本会自动执行以下操作：
1. 检查 Docker 环境。
2. 创建必要的数据持久化目录。
3. 构建前端和后端镜像。
4. 启动所有服务容器。

## 4. 配置说明

可以通过设置环境变量来调整部署配置。推荐在项目根目录创建 `.env` 文件（docker-compose 会自动读取），或者直接导出环境变量。

| 变量名 | 默认值 | 说明 |
|--------|--------|------|
| `FRONTEND_PORT` | `80` | 前端服务对外暴露的端口 |
| `MYSQL_ROOT_PASSWORD` | `root_password` | 数据库 root 密码 |
| `MYSQL_PASSWORD` | `123456` | 数据库 oj_client 用户密码 |
| `MYSQL_DB` | `oj` | 数据库名称 |

### 示例：修改对外端口为 8080

```bash
export FRONTEND_PORT=8080
./scripts/deploy_docker.sh
```

## 5. 常见问题

### Q: 脚本提示 "Error: Docker is not installed"
**A:** 请确保已安装 Docker Desktop (Mac/Win) 或 Docker Engine (Linux) 并已启动。在 Mac 上，请检查顶部菜单栏是否有 Docker 图标。

### Q: 数据库连接失败
**A:** 首次启动时 MySQL 初始化可能需要一点时间。Docker Compose 配置了 `healthcheck`，服务会自动等待数据库就绪。如果长时间失败，请检查 `docker logs oj_db`。

### Q: 如何查看服务日志？
**A:** 使用以下命令查看日志：
```bash
# 查看所有日志
docker compose -f docker/docker-compose.yml logs -f

# 查看特定服务日志
docker compose -f docker/docker-compose.yml logs -f oj_server
```

### Q: 如何更新代码？
**A:** 代码更新后，再次运行 `./scripts/deploy_docker.sh` 即可。脚本会自动重建镜像并重启容器。

## 6. 数据持久化

所有重要数据都挂载在 `docker/data` 目录下：
- `docker/data/mysql`: 数据库文件
- `docker/data/oj_server`: 题目数据和配置
- `docker/data/questions`: 题目详细信息

请定期备份该目录。
