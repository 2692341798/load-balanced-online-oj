import pymysql
import requests
import subprocess
import time
import os
import sys
import json

# Configuration
DB_HOST = os.environ.get('MYSQL_HOST', '127.0.0.1')
DB_USER = os.environ.get('MYSQL_USER', 'oj_client')
DB_PASS = os.environ.get('MYSQL_PASSWORD', '123456')
DB_NAME = os.environ.get('MYSQL_DB', 'oj')
DB_PORT = int(os.environ.get('MYSQL_PORT', 3306))

SERVER_PORT = 8094
SERVER_BASE_URL = f"http://127.0.0.1:{SERVER_PORT}"
ADMIN_INVITATION_CODE = "TEST_CODE_123"

ADMIN_USER = {
    "username": "test_admin",
    "password": "password123",
    "email": "admin@example.com",
    "invitation_code": ADMIN_INVITATION_CODE
}

DUMMY_USER = {
    "username": "dummy_user",
    "password": "password123",
    "email": "dummy@example.com"
}

def print_step(step, msg):
    print(f"\n[Step {step}] {msg}")

def setup_database():
    print("Setting up database...")
    try:
        conn = pymysql.connect(host=DB_HOST, user=DB_USER, password=DB_PASS, db=DB_NAME, port=DB_PORT, autocommit=True)
        cursor = conn.cursor()
        
        # Create tables if not exist
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS `invitation_codes` (
              `id` INT PRIMARY KEY AUTO_INCREMENT,
              `code` VARCHAR(50) NOT NULL UNIQUE,
              `is_used` TINYINT(1) DEFAULT 0,
              `used_by` INT DEFAULT 0,
              `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """)
        
        cursor.execute("""
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
              `role` INT DEFAULT 0 COMMENT '0:User, 1:Admin',
              `status` INT DEFAULT 0 COMMENT '0:Normal, 1:Banned',
              PRIMARY KEY (`id`)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        """)
        
        # Clean up existing test data
        cursor.execute("DELETE FROM invitation_codes WHERE code=%s", (ADMIN_INVITATION_CODE,))
        cursor.execute("DELETE FROM users WHERE username=%s", (ADMIN_USER['username'],))
        cursor.execute("DELETE FROM users WHERE username=%s", (DUMMY_USER['username'],))
        
        # Insert invitation code
        cursor.execute("INSERT INTO invitation_codes (code) VALUES (%s)", (ADMIN_INVITATION_CODE,))
        
        conn.close()
        print("Database setup complete.")
    except Exception as e:
        print(f"Database setup failed: {e}")
        sys.exit(1)

def start_server():
    print("Starting server...")
    # Kill existing process on port 8094
    os.system(f"lsof -t -i:{SERVER_PORT} | xargs kill -9 2>/dev/null")
    
    # Start server in background
    # Assuming running from project root
    server_path = "./oj_server/oj_server"
    if not os.path.exists(server_path):
        print(f"Error: Server binary not found at {server_path}")
        # Try to find it in current directory if running inside oj_server
        if os.path.exists("./oj_server"):
             server_path = "./oj_server"
        else:
             sys.exit(1)
            
    # Run from oj_server directory to ensure resources are found
    cwd = os.path.dirname(server_path)
    if cwd == "": cwd = "."
    cmd = ["./" + os.path.basename(server_path)]
    
    # We need to run it in background
    # Using subprocess.Popen
    try:
        proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print(f"Server started with PID {proc.pid}")
        
        # Wait for server to be ready
        for i in range(10):
            try:
                requests.get(SERVER_BASE_URL, timeout=1)
                print("Server is ready.")
                return proc
            except:
                time.sleep(1)
                print("Waiting for server...")
        
        print("Server failed to start within timeout.")
        proc.kill()
        sys.exit(1)
    except Exception as e:
        print(f"Failed to start server: {e}")
        sys.exit(1)

def run_tests():
    session = requests.Session()
    
    # 2. Register Admin
    print_step(2, "Register Admin")
    url = f"{SERVER_BASE_URL}/api/admin/register"
    resp = session.post(url, json=ADMIN_USER)
    print(f"Response: {resp.status_code} {resp.text}")
    if resp.status_code != 200 or resp.json().get('status') != 0:
        print("Failed to register admin")
        return False

    # 3. Login Admin
    print_step(3, "Login Admin")
    url = f"{SERVER_BASE_URL}/api/login"
    login_data = {
        "username": ADMIN_USER['username'],
        "password": ADMIN_USER['password']
    }
    resp = session.post(url, json=login_data)
    print(f"Response: {resp.status_code} {resp.text}")
    if resp.status_code != 200 or resp.json().get('status') != 0:
        print("Failed to login admin")
        return False
    
    # 4. List Users
    print_step(4, "List Users")
    url = f"{SERVER_BASE_URL}/api/admin/users"
    resp = session.get(url)
    print(f"Response: {resp.status_code} {resp.text[:200]}...") # Truncate long output
    if resp.status_code != 200:
        print("Failed to list users")
        return False

    # 5. Ban User
    # First create a dummy user
    print_step(5, "Create & Ban Dummy User")
    url = f"{SERVER_BASE_URL}/api/register"
    resp = requests.post(url, json=DUMMY_USER) # Use new session or requests directly
    if resp.status_code != 200 or resp.json().get('status') != 0:
         print("Failed to register dummy user")
         return False
    
    # Get Dummy User ID
    # We can find it in the user list
    url = f"{SERVER_BASE_URL}/api/admin/users?keyword={DUMMY_USER['username']}"
    resp = session.get(url)
    users_data = resp.json()
    if users_data.get('status') != 0:
         print("Failed to search user")
         return False
         
    users = users_data.get('data', [])
    if not users:
        print("Dummy user not found in list")
        return False
    
    dummy_id = None
    for u in users:
        if u['username'] == DUMMY_USER['username']:
            dummy_id = u['id']
            break
            
    if not dummy_id:
        print("Dummy user ID not found")
        return False
        
    print(f"Dummy User ID: {dummy_id}")
    
    # Ban
    url = f"{SERVER_BASE_URL}/api/admin/user/status"
    resp = session.post(url, json={"id": dummy_id, "status": 1})
    print(f"Ban Response: {resp.status_code} {resp.text}")
    if resp.status_code != 200 or resp.json().get('status') != 0:
        print("Failed to ban user")
        return False

    # 6. Promote User
    print_step(6, "Promote User")
    url = f"{SERVER_BASE_URL}/api/admin/user/role"
    resp = session.post(url, json={"id": dummy_id, "role": 1})
    print(f"Promote Response: {resp.status_code} {resp.text}")
    if resp.status_code != 200 or resp.json().get('status') != 0:
        print("Failed to promote user")
        return False

    # 7. Reset Password
    print_step(7, "Reset Password")
    url = f"{SERVER_BASE_URL}/api/admin/user/reset_password"
    resp = session.post(url, json={"id": dummy_id})
    print(f"Reset Password Response: {resp.status_code} {resp.text}")
    if resp.status_code != 200 or resp.json().get('status') != 0:
        print("Failed to reset password")
        return False
    print(f"New Password: {resp.json().get('new_password')}")

    # 8. Check Logs
    print_step(8, "Check Logs")
    url = f"{SERVER_BASE_URL}/api/admin/logs"
    resp = session.get(url)
    print(f"Response: {resp.status_code} {resp.text[:200]}...")
    if resp.status_code != 200:
        print("Failed to get logs")
        return False

    # 9. Check Stats
    print_step(9, "Check Stats")
    url = f"{SERVER_BASE_URL}/api/admin/stats"
    resp = session.get(url)
    print(f"Response: {resp.status_code} {resp.text[:200]}...")
    if resp.status_code != 200:
        print("Failed to get stats")
        return False

    # 10. Check Frontend Pages
    print_step(10, "Check Frontend Pages")
    pages = ["/admin/index.html", "/admin/users.html", "/admin/logs.html", "/admin/register.html"]
    for page in pages:
        url = f"{SERVER_BASE_URL}{page}"
        resp = session.get(url)
        print(f"Checking {page}: {resp.status_code}")
        if resp.status_code != 200:
            print(f"Failed to load {page}")
            return False
            
    print("\nAll tests passed!")
    return True

if __name__ == "__main__":
    setup_database()
    proc = start_server()
    try:
        success = run_tests()
        if not success:
            sys.exit(1)
    finally:
        print("Stopping server...")
        proc.terminate()
        proc.wait()
