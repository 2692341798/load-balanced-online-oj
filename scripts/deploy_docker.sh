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
echo "[1/6] Checking SSH connection..."
ssh -i "$KEY_PATH" -o BatchMode=yes -o ConnectTimeout=5 "$USER@$SERVER_IP" echo "SSH Connected" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Error: SSH connection failed."
    echo "Please ensure you have access to $USER@$SERVER_IP with key $KEY_PATH"
    exit 1
fi

# 2. Check Remote Docker Environment
echo "[2/6] Checking remote Docker environment..."
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "docker --version && docker compose version" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "Error: Docker or Docker Compose not found on remote server."
    echo "Please install Docker and Docker Compose on the server first."
    exit 1
fi

# 2.5. Detect Remote CPU Cores and Generate Config
echo "[2.5/6] Detecting remote CPU cores and generating config..."
REMOTE_CORES=$(ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "nproc")
if [ -z "$REMOTE_CORES" ]; then
    echo "Warning: Failed to detect remote CPU cores. Defaulting to 1."
    REMOTE_CORES=1
fi
echo "Detected $REMOTE_CORES cores on remote server."

# Generate Config locally
./scripts/generate_docker_compose.sh "$REMOTE_CORES"

# 3. Prepare Local Data
echo "[3/6] Preparing local data..."
# 3.1 Dump Database
echo "  - Dumping local database..."
mysqldump -u "$LOCAL_DB_USER" -p"$LOCAL_DB_PASS" --databases "$LOCAL_DB_NAME" --add-drop-database --hex-blob > full_db_dump.sql
if [ $? -ne 0 ]; then
    echo "Error: Failed to dump local database."
    exit 1
fi

# 4. Sync Code and Data
echo "[4/6] Syncing code and data to remote..."
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

# 5. Deploy with Docker Compose
echo "[5/6] Deploying with Docker Compose..."
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

# 6. Restore Database
echo "[6/6] Restoring database..."
echo "  - Waiting for database to initialize (15s)..."
sleep 15

RESTORE_CMD="
# Check if db container is running
if [ \$(docker inspect -f '{{.State.Running}}' oj_db) != 'true' ]; then
    echo 'Error: Database container is not running.'
    exit 1
fi

# Backup existing remote database (Safety First)
echo '  - Backing up existing remote database...'
docker exec oj_db mysqldump -u root -proot_password --databases oj > $REMOTE_DIR/oj_backup_\$(date +%Y%m%d_%H%M%S).sql 2>/dev/null
if [ \$? -eq 0 ]; then
    echo '    Backup created.'
else
    echo '    No existing database or backup failed (first run?). Continuing.'
fi

# Restore data
echo '  - Importing new data...'
cat $REMOTE_DIR/full_db_dump.sql | docker exec -i oj_db mysql -u root -proot_password
"

ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "$RESTORE_CMD"

if [ $? -ne 0 ]; then
    echo "Error: Database restore failed."
    # Don't exit, just warn, maybe just needs more time
    echo "Warning: Check if database was ready."
else
    echo "Database restored successfully."
fi

# Cleanup local dump
rm full_db_dump.sql
# Cleanup remote dump
ssh -i "$KEY_PATH" "$USER@$SERVER_IP" "rm $REMOTE_DIR/full_db_dump.sql"

echo "=== Deployment Complete! ==="
echo "Access your service at: http://$SERVER_IP:8094"
echo "Verify functionality:"
echo "1. Question List: http://$SERVER_IP:8094/all_questions"
echo "2. Login/Register"
echo "3. Submit Code"
