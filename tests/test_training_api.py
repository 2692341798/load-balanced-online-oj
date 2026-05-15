import os
import sys
import time
import json
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


def setup_database(test_username: str, training_title: str, question_number: int):
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

    cursor.execute(
        """
        CREATE TABLE IF NOT EXISTS `training_lists` (
          `id` INT PRIMARY KEY AUTO_INCREMENT,
          `title` VARCHAR(255) NOT NULL,
          `description` TEXT,
          `difficulty` VARCHAR(50) DEFAULT 'Unrated',
          `tags` TEXT,
          `author_id` INT NOT NULL,
          `visibility` VARCHAR(20) DEFAULT 'public',
          `likes` INT DEFAULT 0,
          `collections` INT DEFAULT 0,
          `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
          `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
          INDEX idx_author_id (author_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """
    )

    cursor.execute(
        """
        CREATE TABLE IF NOT EXISTS `training_list_items` (
          `id` INT PRIMARY KEY AUTO_INCREMENT,
          `training_list_id` INT NOT NULL,
          `question_id` INT NOT NULL,
          `order_index` INT NOT NULL DEFAULT 0,
          `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
          INDEX idx_list_id (training_list_id),
          INDEX idx_question_id (question_id),
          UNIQUE KEY unique_item (training_list_id, question_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """
    )

    cursor.execute(
        """
        CREATE TABLE IF NOT EXISTS `oj_questions` (
          `number` INT PRIMARY KEY AUTO_INCREMENT,
          `title` VARCHAR(255) NOT NULL,
          `star` VARCHAR(50) NOT NULL,
          `cpu_limit` INT NOT NULL DEFAULT 1,
          `mem_limit` INT NOT NULL DEFAULT 30000,
          `description` TEXT NOT NULL,
          `header` TEXT DEFAULT NULL,
          `tail_code` TEXT NOT NULL,
          `status` INT DEFAULT 1
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """
    )

    cursor.execute("DELETE FROM users WHERE username=%s", (test_username,))
    cursor.execute("DELETE FROM training_lists WHERE title=%s", (training_title,))
    cursor.execute("DELETE FROM oj_questions WHERE number=%s", (question_number,))

    cursor.execute(
        """
        INSERT INTO oj_questions
          (number, title, star, cpu_limit, mem_limit, description, tail_code, status)
        VALUES
          (%s, %s, %s, %s, %s, %s, %s, %s)
        """,
        (
            question_number,
            "TDD Dummy Question",
            "简单",
            1,
            30000,
            "desc",
            json.dumps([{"input": "1 1\n", "output": "2\n"}], ensure_ascii=False),
            1,
        ),
    )

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
    test_username = f"tt{now}"
    training_title = f"test_training_list_{now}"
    question_number = 10001

    setup_database(test_username, training_title, question_number)
    proc = start_server()

    try:
        session = requests.Session()

        resp = session.post(
            f"{SERVER_BASE_URL}/api/register",
            json={"username": test_username, "password": "password123", "email": "t@example.com"},
            timeout=5,
        )
        if resp.status_code != 200 or resp.json().get("status") != 0:
            print(f"register failed: {resp.status_code} {resp.text}", file=sys.stderr)
            sys.exit(1)

        resp = session.post(
            f"{SERVER_BASE_URL}/api/login",
            json={"username": test_username, "password": "password123"},
            timeout=5,
        )
        if resp.status_code != 200 or resp.json().get("status") != 0:
            print(f"login failed: {resp.status_code} {resp.text}", file=sys.stderr)
            sys.exit(1)

        resp = session.post(
            f"{SERVER_BASE_URL}/api/training/create",
            json={"title": training_title, "description": "d", "difficulty": "简单", "tags": "[]", "visibility": "public"},
            timeout=5,
        )
        if resp.status_code != 200 or resp.json().get("status") != 0:
            print(f"create training list failed: {resp.status_code} {resp.text}", file=sys.stderr)
            sys.exit(1)

        training_list_id = str(resp.json().get("id"))

        resp = session.post(
            f"{SERVER_BASE_URL}/api/training/add_problems",
            json={"training_list_id": training_list_id, "question_ids": [str(question_number)]},
            timeout=5,
        )
        if resp.status_code != 200:
            print(f"add_problems http failed: {resp.status_code} {resp.text}", file=sys.stderr)
            sys.exit(1)
        data = resp.json()
        if data.get("status") != 0 or int(data.get("success_count", -1)) != 1:
            print(f"add_problems failed: {resp.text}", file=sys.stderr)
            sys.exit(1)

        print("PASS")
    finally:
        proc.kill()


if __name__ == "__main__":
    main()
