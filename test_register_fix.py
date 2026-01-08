#!/usr/bin/env python3
"""
用户注册功能修复测试脚本
测试修复后的注册功能是否正常工作
"""

import requests
import json
import time
import random
import string

# 测试配置
BASE_URL = "http://localhost:8080"
REGISTER_API = f"{BASE_URL}/api/register"
LOGIN_API = f"{BASE_URL}/api/login"

def generate_random_username():
    """生成随机用户名"""
    return f"test_user_{''.join(random.choices(string.ascii_lowercase + string.digits, k=8))}"

def generate_random_email():
    """生成随机邮箱"""
    return f"test_{''.join(random.choices(string.ascii_lowercase + string.digits, k=8))}@example.com"

def test_register_success():
    """测试成功注册"""
    print("=== 测试成功注册 ===")
    
    username = generate_random_username()
    email = generate_random_email()
    password = "test123456"
    
    data = {
        "username": username,
        "password": password,
        "email": email,
        "nickname": "测试用户",
        "phone": "13800138000"
    }
    
    try:
        response = requests.post(REGISTER_API, json=data)
        result = response.json()
        
        if result.get("status") == 0:
            print(f"✅ 注册成功: {username}")
            return True, username, password
        else:
            print(f"❌ 注册失败: {result.get('reason', '未知错误')}")
            return False, None, None
    except Exception as e:
        print(f"❌ 注册异常: {e}")
        return False, None, None

def test_register_duplicate_username():
    """测试重复用户名注册"""
    print("\n=== 测试重复用户名注册 ===")
    
    # 先注册一个用户
    success, username, password = test_register_success()
    if not success:
        print("❌ 无法创建初始用户，跳过重复测试")
        return False
    
    # 尝试用相同用户名再次注册
    email = generate_random_email()
    data = {
        "username": username,
        "password": "different_password",
        "email": email
    }
    
    try:
        response = requests.post(REGISTER_API, json=data)
        result = response.json()
        
        if result.get("status") == 1 and "用户名已存在" in result.get("reason", ""):
            print(f"✅ 重复用户名检测正确: {username}")
            return True
        else:
            print(f"❌ 重复用户名检测失败: {result.get('reason', '未知错误')}")
            return False
    except Exception as e:
        print(f"❌ 重复注册异常: {e}")
        return False

def test_register_invalid_params():
    """测试无效参数注册"""
    print("\n=== 测试无效参数注册 ===")
    
    test_cases = [
        {
            "name": "空用户名",
            "data": {"username": "", "password": "test123", "email": "test@example.com"},
            "expected_reason": "用户名、密码和邮箱不能为空"
        },
        {
            "name": "空密码",
            "data": {"username": "testuser", "password": "", "email": "test@example.com"},
            "expected_reason": "用户名、密码和邮箱不能为空"
        },
        {
            "name": "空邮箱",
            "data": {"username": "testuser", "password": "test123", "email": ""},
            "expected_reason": "用户名、密码和邮箱不能为空"
        },
        {
            "name": "用户名太短",
            "data": {"username": "ab", "password": "test123", "email": "test@example.com"},
            "expected_reason": "用户名长度必须在3-20个字符之间"
        },
        {
            "name": "密码太短",
            "data": {"username": "testuser", "password": "12345", "email": "test@example.com"},
            "expected_reason": "密码长度必须在6-30个字符之间"
        }
    ]
    
    all_passed = True
    for test_case in test_cases:
        try:
            response = requests.post(REGISTER_API, json=test_case["data"])
            result = response.json()
            
            if (result.get("status") == 1 and 
                test_case["expected_reason"] in result.get("reason", "")):
                print(f"✅ {test_case['name']} 检测正确")
            else:
                print(f"❌ {test_case['name']} 检测失败: {result.get('reason', '未知错误')}")
                all_passed = False
        except Exception as e:
            print(f"❌ {test_case['name']} 异常: {e}")
            all_passed = False
    
    return all_passed

def test_login_after_register():
    """测试注册后登录"""
    print("\n=== 测试注册后登录 ===")
    
    # 注册用户
    success, username, password = test_register_success()
    if not success:
        return False
    
    # 尝试登录
    login_data = {
        "username": username,
        "password": password
    }
    
    try:
        response = requests.post(LOGIN_API, json=login_data)
        result = response.json()
        
        if result.get("status") == 0:
            print(f"✅ 注册后登录成功: {username}")
            return True
        else:
            print(f"❌ 注册后登录失败: {result.get('reason', '未知错误')}")
            return False
    except Exception as e:
        print(f"❌ 登录异常: {e}")
        return False

def run_all_tests():
    """运行所有测试"""
    print("🚀 开始用户注册功能测试")
    print(f"API地址: {REGISTER_API}")
    print("=" * 50)
    
    tests = [
        ("成功注册", test_register_success),
        ("重复用户名检测", test_register_duplicate_username),
        ("无效参数检测", test_register_invalid_params),
        ("注册后登录", test_login_after_register)
    ]
    
    results = []
    for test_name, test_func in tests:
        try:
            result = test_func()
            results.append((test_name, result))
        except Exception as e:
            print(f"❌ {test_name} 测试异常: {e}")
            results.append((test_name, False))
    
    # 汇总结果
    print("\n" + "=" * 50)
    print("📊 测试结果汇总:")
    passed = 0
    for test_name, result in results:
        status = "✅ 通过" if result else "❌ 失败"
        print(f"{status} {test_name}")
        if result:
            passed += 1
    
    print(f"\n总计: {passed}/{len(tests)} 测试通过")
    
    if passed == len(tests):
        print("🎉 所有测试通过！注册功能修复成功！")
        return True
    else:
        print("⚠️  部分测试失败，请检查修复结果")
        return False

if __name__ == "__main__":
    # 等待服务启动
    print("等待服务启动...")
    time.sleep(2)
    
    # 检查服务是否可用
    try:
        response = requests.get(f"{BASE_URL}/login", timeout=5)
        print("✅ 服务已启动，开始测试")
    except Exception as e:
        print(f"❌ 无法连接服务: {e}")
        print("请确保服务已启动并监听在8080端口")
        exit(1)
    
    run_all_tests()