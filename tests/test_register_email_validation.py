import os
import sys
import time
import subprocess

import pymysql
import requests


DB_HOST = os.environ.get("MYSQL_HOST", "127.0.0.1")
DB_USER = os.environ.get("MYSQL_USER", "oj_client")
DB_PASS = os.environ.get("MYSQL_PASSWORD", "123456")
DB_NAME = os.environ.get("MYSQL_DB", "oj")
DB_PORT = int(os.environ.get("MYSQL_PORT", 3306))

SERVER_PORT = 8094
SERVER_BASE_URL = f"http://127.0.0.1:{SERVER_PORT}"


def setup_database(test_usernames: list[str]):
    conn = pymysql.connect(
        host=DB_HOST,
        user=DB_USER,
        password=DB_PASS,
        db=DB_NAME,
        port=DB_PORT,
        autocommit=True,
    )
    cursor = conn.cursor()
    cursor.execute(
        """
        CREATE TABLE IF NOT EXISTS `users` (
          `id` int(11) NOT NULL AUTO_INCREMENT,
          `username` varchar(50) NOT NULL UNIQUE,
          `password` varchar(128) NOT NULL,
          `email` varchar(100) DEFAULT NULL,
          `nickname` varchar(100) DEFAULT NULL,
          `phone` varchar(20) DEFAULT NULL,
          `avatar` varchar(255) DEFAULT NULL,
          `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
          `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
          `role` INT DEFAULT 0,
          `status` INT DEFAULT 0,
          PRIMARY KEY (`id`)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """
    )
    for u in test_usernames:
        cursor.execute("DELETE FROM users WHERE username=%s", (u,))
    conn.close()


def start_server():
    os.system(f"lsof -t -i:{SERVER_PORT} | xargs kill -9 2>/dev/null")
    server_path = "./oj_server/oj_server"
    if not os.path.exists(server_path):
        print(f"Error: Server binary not found at {server_path}", file=sys.stderr)
        sys.exit(1)
    cwd = os.path.dirname(server_path)
    if cwd == "":
        cwd = "."
    cmd = ["./" + os.path.basename(server_path)]
    proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(10):
        try:
            requests.get(SERVER_BASE_URL, timeout=1)
            return proc
        except Exception:
            time.sleep(1)
    proc.kill()
    print("Server failed to start within timeout.", file=sys.stderr)
    sys.exit(1)


def main():
    now = int(time.time())
    suffix = str(now % 1000000)
    valid_username = f"ev{suffix}"
    invalid_username = f"ei{suffix}"
    setup_database([valid_username, invalid_username])
    proc = start_server()
    try:
        resp = requests.post(
            f"{SERVER_BASE_URL}/api/register",
            json={"username": valid_username, "password": "password123", "email": "t@example.com"},
            timeout=5,
        )
        if resp.status_code != 200:
            print(f"http failed: {resp.status_code} {resp.text}", file=sys.stderr)
            sys.exit(1)
        data = resp.json()
        if data.get("status") != 0:
            print(f"expected valid email accepted, got error: {resp.text}", file=sys.stderr)
            sys.exit(1)

        resp = requests.post(
            f"{SERVER_BASE_URL}/api/register",
            json={"username": invalid_username, "password": "password123", "email": "invalid-email"},
            timeout=5,
        )
        if resp.status_code != 200:
            print(f"http failed: {resp.status_code} {resp.text}", file=sys.stderr)
            sys.exit(1)
        data = resp.json()
        if data.get("status") == 0:
            print(f"expected invalid email rejected, got success: {resp.text}", file=sys.stderr)
            sys.exit(1)
        if "邮箱" not in str(data.get("reason", "")):
            print(f"expected email format error message, got: {resp.text}", file=sys.stderr)
            sys.exit(1)
        print("PASS")
    finally:
        proc.kill()


if __name__ == "__main__":
    main()
