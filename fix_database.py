#!/usr/bin/env python3
"""
检查和修复数据库表结构
"""

import pymysql
import sys

# 数据库配置
DB_CONFIG = {
    'host': '127.0.0.1',
    'user': 'oj_client',
    'password': '123456',
    'database': 'oj',
    'port': 3306,
    'charset': 'utf8'
}

def check_and_fix_table():
    """检查并修复users表结构"""
    try:
        conn = pymysql.connect(**DB_CONFIG)
        cursor = conn.cursor()
        
        # 检查表结构
        cursor.execute("DESCRIBE users;")
        columns = cursor.fetchall()
        
        print("当前users表结构:")
        for col in columns:
            print(f"  {col[0]}: {col[1]} {col[2]} {col[3]}")
        
        # 检查是否缺少nickname列
        column_names = [col[0] for col in columns]
        
        if 'nickname' not in column_names:
            print("\n添加nickname列...")
            cursor.execute("ALTER TABLE users ADD COLUMN nickname varchar(100) DEFAULT NULL;")
            print("✅ nickname列添加成功")
        
        if 'phone' not in column_names:
            print("\n添加phone列...")
            cursor.execute("ALTER TABLE users ADD COLUMN phone varchar(20) DEFAULT NULL;")
            print("✅ phone列添加成功")
        
        conn.commit()
        print("\n✅ 表结构检查和修复完成")
        
    except Exception as e:
        print(f"❌ 数据库操作失败: {e}")
        return False
    finally:
        if 'cursor' in locals():
            cursor.close()
        if 'conn' in locals():
            conn.close()
    
    return True

def test_user_query():
    """测试用户查询"""
    try:
        conn = pymysql.connect(**DB_CONFIG)
        cursor = conn.cursor()
        
        # 测试查询
        cursor.execute("SELECT * FROM users WHERE username = 'testuser123';")
        result = cursor.fetchall()
        
        if result:
            print(f"找到用户: {result}")
        else:
            print("用户不存在")
        
        return True
        
    except Exception as e:
        print(f"❌ 查询失败: {e}")
        return False
    finally:
        if 'cursor' in locals():
            cursor.close()
        if 'conn' in locals():
            conn.close()

if __name__ == "__main__":
    print("🔍 开始检查和修复数据库表结构")
    print("=" * 50)
    
    # 检查并修复表结构
    if check_and_fix_table():
        print("\n" + "=" * 50)
        print("🧪 测试用户查询")
        test_user_query()
    
    print("\n✅ 检查和修复完成")