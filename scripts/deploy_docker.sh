#!/bin/bash

# Load Balanced Online OJ - Docker Deployment Script
# This script builds and deploys the entire stack using Docker Compose.

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}==> Starting Docker Deployment...${NC}"

# Function to check command existence
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed.${NC}"
        return 1
    fi
    return 0
}

# Check for Docker
if ! check_command "docker"; then
    echo -e "${YELLOW}Please install Docker first:${NC}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  - macOS: Install Docker Desktop from https://www.docker.com/products/docker-desktop/"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "  - Linux: Run 'curl -fsSL https://get.docker.com | sh'"
    else
        echo "  - Visit https://docs.docker.com/get-docker/"
    fi
    exit 1
fi

# Check for Docker Compose
# Try 'docker compose' (v2) first, then 'docker-compose' (v1)
if docker compose version &> /dev/null; then
    COMPOSE_CMD="docker compose"
elif command -v docker-compose &> /dev/null; then
    COMPOSE_CMD="docker-compose"
else
    echo -e "${RED}Error: Docker Compose is not installed.${NC}"
    echo -e "${YELLOW}Please install Docker Compose or update Docker Desktop.${NC}"
    exit 1
fi

echo -e "${GREEN}==> Using Docker Compose command: $COMPOSE_CMD${NC}"

# Navigate to project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/.."
cd "$PROJECT_ROOT"

echo -e "${YELLOW}==> Project Root: $PROJECT_ROOT${NC}"

# Create necessary directories for volumes if they don't exist
# This ensures permissions are managed correctly on host
echo -e "${YELLOW}==> Preparing data directories...${NC}"
mkdir -p "$PROJECT_ROOT/docker/data/mysql"
mkdir -p "$PROJECT_ROOT/docker/data/oj_server"
mkdir -p "$PROJECT_ROOT/docker/data/questions"

# Set permissions if needed (optional, depending on environment)
# chmod -R 777 "$PROJECT_ROOT/docker/data"

echo -e "${YELLOW}==> Building and Starting Services...${NC}"

# Build and Start
$COMPOSE_CMD -f docker/docker-compose.yml down --remove-orphans
$COMPOSE_CMD -f docker/docker-compose.yml up -d --build

echo -e "${GREEN}==> Deployment Successful!${NC}"
echo -e "${GREEN}==> Frontend is accessible at http://localhost${NC}"
echo -e "${YELLOW}==> Backend API is accessible internally at http://oj_server:8094${NC}"

# Show status
$COMPOSE_CMD -f docker/docker-compose.yml ps
