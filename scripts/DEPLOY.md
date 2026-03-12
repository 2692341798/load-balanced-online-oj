# Automated Docker Deployment Guide

This guide describes how to use the `deploy_docker.sh` script to automate the deployment of the Online OJ System to Alibaba Cloud.

## 1. Prerequisites

### Local Environment
- **OS**: macOS or Linux
- **Tools**:
  - `ssh` (for remote connection)
  - `rsync` (for file synchronization)
  - `mysqldump` (for database backup, optional but recommended)
- **SSH Key**: Ensure you have the private key (`.pem`) to access the server.

### Remote Server (Alibaba Cloud)
- **OS**: Ubuntu/CentOS (Linux)
- **Docker**: Installed and running
- **Docker Compose**: Installed

## 2. Configuration

The deployment script uses environment variables for configuration. You should create a `.env` file in the project root.

### Setup
Copy the example configuration:
```bash
cp .env.example .env
```

### Parameters
Edit `.env` with your specific details:

```ini
# Server Configuration
SERVER_IP=47.121.117.192
SSH_USER=huangqijun
SSH_KEY_PATH=/path/to/your/key.pem
REMOTE_DIR=~/load-balanced-online-oj

# Local Database (for dumping data)
LOCAL_DB_USER=oj_client
LOCAL_DB_PASS=123456
LOCAL_DB_NAME=oj

# Remote Database (Docker environment)
MYSQL_ROOT_PASSWORD=root_password
MYSQL_USER=oj_client
MYSQL_PASSWORD=123456
MYSQL_DATABASE=oj
```

### Multi-Environment Support
You can create environment-specific files like `.env.prod` or `.env.test`.
The script will load the corresponding file based on the `-e` argument.

## 3. Usage

Run the script from the project root or `scripts/` directory.

### Basic Usage (Dev Environment)
```bash
./scripts/deploy_docker.sh
```

### Production Deployment
```bash
./scripts/deploy_docker.sh -e prod
```

### Help
```bash
./scripts/deploy_docker.sh -h
```

## 4. Deployment Process Overview

The script performs the following 7 steps:

1.  **Connectivity Check**: Verifies SSH access to the server.
2.  **Remote Environment Check**: Checks if Docker and Docker Compose are installed on the server.
3.  **Local Preparation**: Dumps the local database schema and data to `full_db_dump.sql`.
4.  **Code Sync**: Uses `rsync` to upload code to the remote server (excluding git, node_modules, etc.).
5.  **Remote Deployment**:
    -   Exports environment variables.
    -   **Backs up** existing Docker images (tagging as `:backup`).
    -   Builds new Docker images.
    -   Restarts services using `docker compose up -d`.
6.  **Database Restore**:
    -   Waits for the MySQL container to be healthy.
    -   Backs up the *existing* remote database (safety precaution).
    -   Restores the data from the local dump.
7.  **Health Check & Rollback**:
    -   Polls the OJ Server endpoint (`/all_questions`) to verify it's up.
    -   **Rollback**: If the health check fails, it automatically stops the new containers, restores the `:backup` images, and restarts the service.

## 5. Directory Structure on Remote

```
~/load-balanced-online-oj/
├── backend/            # C++ Backend Code
├── docker/             # Docker Configuration
│   ├── docker-compose.yml
│   ├── Dockerfile.oj
│   ├── Dockerfile.compile
│   └── logs/           # Service Logs
├── scripts/            # Scripts
└── full_db_dump.sql    # Temporary DB dump
```

## 6. Troubleshooting

-   **SSH Permission Denied**: Check `SSH_KEY_PATH` in `.env` and ensure permissions are correct (`chmod 400 key.pem`).
-   **Database Restore Failed**: Check if the local database is running and credentials in `.env` match local MySQL.
-   **Health Check Failed**: Check logs in `docker/logs/oj_server/` on the remote server.
    ```bash
    ssh -i key.pem user@ip "cat ~/load-balanced-online-oj/docker/logs/oj_server/oj_server.log"
    ```
-   **Rollback Triggered**: The script will output "Health check failed! Initiating rollback...". The previous version should be running. Investigate logs to see why the new version failed.
