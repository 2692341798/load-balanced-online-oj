#!/bin/bash

# Configuration
SERVER_IP="47.121.117.192"
USER="huangqijun" # Assuming user has docker permissions or is in docker group
KEY_PATH="/Users/huangqijun/Downloads/Mac.pem"
REMOTE_DIR="~/load-balanced-online-oj"
LOCAL_DB_USER="oj_client"
LOCAL_DB_PASS="123456"
LOCAL_DB_NAME="oj"

echo "=== Online OJ Docker Deployment & Sync Script ==="

# 0. Check for SSH Key
if [ ! -f "$KEY_PATH" ]; then
    echo "Error: Private key not found at $KEY_PATH"
    exit 1
fi

# 1. Check SSH Connection
echo "[1/4] Checking SSH connection..."
ssh -i "$KEY_PATH" -o BatchMode=yes -o ConnectTimeout=5 "$USER@$SERVER_IP" echo "SSH Connected" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Error: SSH connection failed."
    echo "Please ensure you have access to $USER@$SERVER_IP with key $KEY_PATH"
    exit 1
fi

# 2. Check Remote Docker Environment
echo "[2/4] Checking remote Docker environment..."
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "docker --version && docker compose version" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Error: Docker or Docker Compose not found on remote server."
    echo "Please install Docker and Docker Compose on the server first."
    exit 1
fi

# 3. Sync Code
echo "[3/4] Syncing code to remote..."
# Create remote directory if not exists
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "mkdir -p $REMOTE_DIR"

# Rsync project files
# Exclude git, build artifacts, local output, and temp files
rsync -avz -e "ssh -i $KEY_PATH" \
    --exclude '.git' \
    --exclude '.vscode' \
    --exclude '.trae' \
    --exclude 'output' \
    --exclude 'build' \
    --exclude '*.o' \
    --exclude '*.d' \
    --exclude '__pycache__' \
    --exclude '.DS_Store' \
    ./ "$USER@$SERVER_IP:$REMOTE_DIR/"

if [ $? -ne 0 ]; then
    echo "Error: Rsync failed."
    exit 1
fi

# 4. Deploy with Docker Compose
echo "[4/4] Deploying with Docker Compose..."
DEPLOY_CMD="
cd $REMOTE_DIR/docker
# Stop existing services
docker compose down
# Build and start services in background
docker compose up -d --build
"
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "$DEPLOY_CMD"

if [ $? -ne 0 ]; then
    echo "Error: Docker deployment failed."
    exit 1
fi

echo "=== Deployment Complete! ==="
echo "Access your service at: http://$SERVER_IP:8094"
echo "Verify functionality:"
echo "1. Question List: http://$SERVER_IP:8094/all_questions"
echo "2. Login/Register"
echo "3. Submit Code"
