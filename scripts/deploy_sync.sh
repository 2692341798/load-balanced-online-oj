#!/bin/bash

# Configuration
SERVER_IP="47.121.117.192"
USER="huangqijun"
KEY_PATH="/Users/huangqijun/Downloads/Mac.pem"
REMOTE_DIR="~/load-balanced-online-oj" 
LOCAL_DB_USER="oj_client"
LOCAL_DB_PASS="123456"
LOCAL_DB_NAME="oj"

echo "=== Online OJ Deployment & Sync Script ==="

# 0. Check for SSH Key
if [ ! -f "$KEY_PATH" ]; then
    echo "Error: Private key not found at $KEY_PATH"
    exit 1
fi

# 1. Check SSH Connection
echo "[1/5] Checking SSH connection..."
ssh -i "$KEY_PATH" -o BatchMode=yes -o ConnectTimeout=5 "$USER@$SERVER_IP" echo "SSH Connected" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Error: SSH connection failed."
    echo "Reason: The server rejected the key $KEY_PATH."
    echo "Action Required: Please ensure your public key is added to ~/.ssh/authorized_keys on the server."
    echo "You can try to copy your ID manually if you have password access:"
    echo "  ssh-copy-id -i $KEY_PATH $USER@$SERVER_IP"
    exit 1
fi

# 1.5. Check Remote Dependencies
echo "[1.5/5] Checking remote build environment..."
CHECK_CMD="
MISSING=\"\"
if [ ! -d /usr/include/jsoncpp ]; then MISSING=\"\$MISSING libjsoncpp-dev\"; fi
if [ ! -d /usr/include/mysql ]; then MISSING=\"\$MISSING libmysqlclient-dev\"; fi
if [ ! -d /usr/include/ctemplate ]; then MISSING=\"\$MISSING libctemplate-dev\"; fi
echo \"\$MISSING\"
"
MISSING_DEPS=$(ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "$CHECK_CMD")

if [ ! -z "$MISSING_DEPS" ]; then
    echo "--------------------------------------------------------"
    echo "Error: Missing dependencies on server:$MISSING_DEPS"
    echo "The automatic deployment cannot proceed without these libraries."
    echo ""
    echo "ACTION REQUIRED: Please login to the server and run:"
    echo ""
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y build-essential libjsoncpp-dev libmysqlclient-dev libssl-dev libctemplate-dev"
    echo ""
    echo "Server Login Command:"
    echo "  ssh -i $KEY_PATH $USER@$SERVER_IP"
    echo "--------------------------------------------------------"
    exit 1
fi

# 2. Sync Code
echo "[2/5] Syncing code..."
# Ensure remote directory exists and pull latest code
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "mkdir -p $REMOTE_DIR && cd $REMOTE_DIR && git pull origin main && make output"
if [ $? -ne 0 ]; then
    echo "Error: Failed to sync code or build on server."
    exit 1
fi

# 3. Sync Database (oj_questions)
echo "[3/5] Syncing database (oj_questions)..."
# Dump local oj_questions
mysqldump -u "$LOCAL_DB_USER" -p"$LOCAL_DB_PASS" "$LOCAL_DB_NAME" oj_questions > oj_questions_dump.sql
if [ $? -ne 0 ]; then
    echo "Error: Failed to dump local database."
    exit 1
fi

# Upload dump
scp -i "$KEY_PATH" oj_questions_dump.sql "$USER@$SERVER_IP:$REMOTE_DIR/"

# Import remote
# Using default db credentials for remote (oj_client/123456/oj)
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "mysql -u oj_client -p123456 oj < $REMOTE_DIR/oj_questions_dump.sql && rm $REMOTE_DIR/oj_questions_dump.sql"

# Clean up local dump
rm oj_questions_dump.sql

# 4. Sync Files (questions directory)
echo "[4/5] Syncing questions directory..."
if [ -d "oj_server/questions" ]; then
    rsync -avz -e "ssh -i $KEY_PATH" oj_server/questions/ "$USER@$SERVER_IP:$REMOTE_DIR/oj_server/questions/"
else
    echo "Warning: Local oj_server/questions directory not found. Skipping file sync."
fi

# 5. Restart Services
echo "[5/5] Restarting remote services..."
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "cd $REMOTE_DIR && ./scripts/stop.sh && ./scripts/start.sh"

echo "=== Deployment Complete! ==="
