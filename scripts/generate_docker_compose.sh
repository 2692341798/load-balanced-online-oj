#!/bin/bash
# Generate docker-compose.yml and service_machine.conf based on CPU cores

# 1. Get CPU Cores from argument or detection
if [ -n "$1" ]; then
    CPU_CORES=$1
else
    # Fallback to local detection
    if command -v nproc >/dev/null 2>&1; then
        CPU_CORES=$(nproc)
    elif command -v sysctl >/dev/null 2>&1; then
        CPU_CORES=$(sysctl -n hw.ncpu)
    else
        CPU_CORES=1
    fi
fi

# Limit max
if [ "$CPU_CORES" -gt 8 ]; then CPU_CORES=8; fi

echo "Generating configuration for $CPU_CORES compile servers..."

# 2. Prepare Conf File
# Use absolute path relative to script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CONF_FILE="$PROJECT_ROOT/backend/oj_server/conf/service_machine_docker.conf"

mkdir -p "$(dirname "$CONF_FILE")"
> "$CONF_FILE"

# 3. Generate docker-compose.yml
COMPOSE_FILE="$PROJECT_ROOT/docker/docker-compose.yml"

# Header
cat > "$COMPOSE_FILE" <<EOF
version: '3.8'

services:
  db:
    image: mysql:8.0
    container_name: oj_db
    command: --character-set-server=utf8mb4 --collation-server=utf8mb4_unicode_ci
    environment:
      MYSQL_ROOT_PASSWORD: root_password
      MYSQL_DATABASE: oj
      MYSQL_USER: oj_client
      MYSQL_PASSWORD: 123456
    volumes:
      - db_data:/var/lib/mysql
      - ../sql/setup_database.sql:/docker-entrypoint-initdb.d/01_setup.sql
      - ../sql/insert_questions.sql:/docker-entrypoint-initdb.d/02_insert.sql
    networks:
      - oj_network
    healthcheck:
      test: ["CMD", "mysqladmin" ,"ping", "-h", "localhost"]
      timeout: 20s
      retries: 10
    restart: always

EOF

# Generate Compile Servers
for ((i=1; i<=CPU_CORES; i++)); do
    # Use 8080 internally for all containers
    PORT=8080
    NAME="compile_server_$i"
    
    cat >> "$COMPOSE_FILE" <<EOF
  $NAME:
    build:
      context: ..
      dockerfile: docker/Dockerfile.compile
    container_name: $NAME
    command: ["./compile_server", "$PORT"]
    networks:
      - oj_network
    restart: always

EOF
    
    # Add to config (internal hostname:port)
    echo "$NAME:$PORT" >> "$CONF_FILE"
done

# OJ Server
cat >> "$COMPOSE_FILE" <<EOF
  oj_server:
    build:
      context: ..
      dockerfile: docker/Dockerfile.oj
    container_name: oj_server
    ports:
      - "8094:8094"
    environment:
      - MYSQL_HOST=db
      - MYSQL_PORT=3306
      - MYSQL_USER=oj_client
      - MYSQL_PASSWORD=123456
      - MYSQL_DB=oj
    volumes:
      - ../backend/oj_server/conf/service_machine_docker.conf:/app/output/oj_server/conf/service_machine.conf
    depends_on:
      db:
        condition: service_healthy
EOF

# Depends on compile servers
for ((i=1; i<=CPU_CORES; i++)); do
    echo "      $NAME:" >> "$COMPOSE_FILE"
    echo "        condition: service_started" >> "$COMPOSE_FILE"
done

# Contest Crawler
cat >> "$COMPOSE_FILE" <<EOF
    networks:
      - oj_network
    restart: always

  contest_crawler:
    build:
      context: ..
      dockerfile: docker/Dockerfile.oj
    container_name: contest_crawler
    command: ["/app/output/crawler/contest_crawler"]
    environment:
      - MYSQL_HOST=db
      - MYSQL_PORT=3306
      - MYSQL_USER=oj_client
      - MYSQL_PASSWORD=123456
      - MYSQL_DB=oj
    depends_on:
      db:
        condition: service_healthy
    networks:
      - oj_network
    restart: always

networks:
  oj_network:
    driver: bridge

volumes:
  db_data:
EOF

echo "Configuration generated successfully."
echo "Config file: $CONF_FILE"
echo "Compose file: $COMPOSE_FILE"
