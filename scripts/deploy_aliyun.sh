#!/bin/bash

# Configuration
SERVER_IP="47.121.117.192"
USER="root"
REMOTE_DIR="/root/load-balanced-online-oj"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Online OJ Docker Deployment to Aliyun ===${NC}"

# 1. Check SSH Connection
echo -e "${YELLOW}[1/5] Checking SSH connection...${NC}"
# Try default SSH identity first (id_rsa, config, etc.)
ssh -o BatchMode=yes -o ConnectTimeout=10 -o StrictHostKeyChecking=no "$USER@$SERVER_IP" echo "SSH Connected" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}Error: SSH connection failed.${NC}"
    echo "Please ensure you can connect to $USER@$SERVER_IP"
    echo "Try running: ssh-copy-id $USER@$SERVER_IP"
    exit 1
fi

# 2. Check Remote Docker Environment
echo -e "${YELLOW}[2/5] Checking remote Docker environment...${NC}"
ssh "$USER@$SERVER_IP" "command -v docker >/dev/null 2>&1"
if [ $? -ne 0 ]; then
    echo -e "${YELLOW}Docker not found. Installing Docker on remote server...${NC}"
    # Install Docker automatically
    ssh "$USER@$SERVER_IP" "curl -fsSL https://get.docker.com | sh"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Error: Failed to install Docker automatically.${NC}"
        exit 1
    fi
fi

# 3. Sync Code
echo -e "${YELLOW}[3/5] Syncing code to remote server...${NC}"
# Create remote directory
ssh "$USER@$SERVER_IP" "mkdir -p $REMOTE_DIR"

# Navigate to project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."
cd "$PROJECT_ROOT"

# Rsync project files using default SSH identity
rsync -avz -e "ssh -o StrictHostKeyChecking=no" \
    --exclude '.git' \
    --exclude '.vscode' \
    --exclude '.trae' \
    --exclude 'output' \
    --exclude 'build' \
    --exclude 'node_modules' \
    --exclude '*.o' \
    --exclude '*.d' \
    --exclude '__pycache__' \
    --exclude '.DS_Store' \
    ./ "$USER@$SERVER_IP:$REMOTE_DIR/"

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Rsync failed.${NC}"
    exit 1
fi

# 4. Deploy with Docker Compose
echo -e "${YELLOW}[4/5] Deploying with Docker Compose...${NC}"

DEPLOY_CMD="
cd $REMOTE_DIR
chmod +x scripts/deploy_docker.sh
# Run the local deployment script on the remote server
./scripts/deploy_docker.sh
"

ssh "$USER@$SERVER_IP" "$DEPLOY_CMD"

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Remote deployment failed.${NC}"
    exit 1
fi

echo -e "${GREEN}=== Deployment Complete! ===${NC}"
echo -e "${GREEN}Access your service at: http://$SERVER_IP${NC}"
echo -e "${YELLOW}Note: Ensure Aliyun Security Group allows inbound traffic on port 80.${NC}"
