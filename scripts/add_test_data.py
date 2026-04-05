import os
import json
import time
import re
import pymysql
import requests
import logging

# 配置日志格式
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# MySQL 数据库配置
DB_CONFIG = {
    'host': '127.0.0.1',
    'user': 'oj_client',
    'password': '123456',
    'database': 'oj',
    'port': 3306,
    'charset': 'utf8mb4',
    'cursorclass': pymysql.cursors.DictCursor
}

# DeepSeek API 配置
DEEPSEEK_API_KEY = 'sk-ec8f7b7814974227a6ca9b0eecd2e678'
DEEPSEEK_API_URL = 'https://api.deepseek.com/chat/completions'

# 进度保存文件与相关配置
PROGRESS_FILE = 'scripts/add_test_progress.json'
MAX_RETRIES = 3
RATE_LIMIT_DELAY = 2  # 每次请求后休眠2秒

def load_progress():
    """加载已处理过的题目编号列表"""
    if os.path.exists(PROGRESS_FILE):
        try:
            with open(PROGRESS_FILE, 'r', encoding='utf-8') as f:
                return set(json.load(f))
        except Exception as e:
            logging.error(f"无法加载进度文件: {e}")
    return set()

def save_progress(progress_set):
    """保存已处理的题目编号列表到文件"""
    os.makedirs(os.path.dirname(PROGRESS_FILE), exist_ok=True)
    with open(PROGRESS_FILE, 'w', encoding='utf-8') as f:
        json.dump(list(progress_set), f)

def extract_json_from_text(text):
    """通过正则提取并解析大模型返回文本中的 JSON 数组"""
    # 尝试匹配 markdown 代码块中的 JSON 数组
    match = re.search(r'```(?:json)?\s*(\[.*?\])\s*```', text, re.DOTALL)
    json_str = ""
    if match:
        json_str = match.group(1)
    else:
        # 降级：直接尝试寻找被 [] 包裹的内容
        match = re.search(r'\[.*\]', text, re.DOTALL)
        if match:
            json_str = match.group(0)
    
    if not json_str:
        logging.error("未能从返回结果中匹配到 JSON 数组。")
        return None

    try:
        return json.loads(json_str)
    except json.JSONDecodeError as e:
        logging.error(f"JSON 解析失败: {e}\n原文提取为:\n{json_str}")
        return None

def call_deepseek_api(description):
    """调用 DeepSeek API 生成测试用例，包含重试机制"""
    headers = {
        'Content-Type': 'application/json',
        'Authorization': f'Bearer {DEEPSEEK_API_KEY}'
    }
    
    prompt = (
        "根据以下编程题目的描述，生成 3 到 5 个测试用例。\n"
        "必须严格按照以下 JSON 数组格式输出，不要包含任何其他解释文字或 markdown 格式：\n"
        '[{"input": "输入样例1", "expect": "预期输出1"}, {"input": "输入样例2", "expect": "预期输出2"}]\n\n'
        f"题目描述：\n{description}"
    )

    data = {
        "model": "deepseek-chat",
        "messages": [
            {"role": "system", "content": "你是一个严谨的编程测试用例生成助手。只输出合法的 JSON 数组，不附加任何其他文本。"},
            {"role": "user", "content": prompt}
        ],
        "temperature": 0.3
    }

    for attempt in range(1, MAX_RETRIES + 1):
        try:
            response = requests.post(DEEPSEEK_API_URL, headers=headers, json=data, timeout=30)
            response.raise_for_status()
            result = response.json()
            
            content = result['choices'][0]['message']['content']
            parsed_json = extract_json_from_text(content)
            
            if parsed_json is not None and isinstance(parsed_json, list):
                return parsed_json
            else:
                logging.warning(f"第 {attempt} 次尝试：返回的数据格式无效或不是数组。")
        except Exception as e:
            logging.warning(f"第 {attempt} 次尝试请求失败: {e}")
        
        if attempt < MAX_RETRIES:
            # 失败后指数退避重试
            time.sleep(RATE_LIMIT_DELAY * attempt)
    
    return None

def merge_and_deduplicate(existing_tail_code, new_cases):
    """解析现有 test cases 并与新生成的 cases 合并，基于 input 去重"""
    existing_cases = []
    if existing_tail_code and str(existing_tail_code).strip():
        try:
            existing_cases = json.loads(existing_tail_code)
            if not isinstance(existing_cases, list):
                existing_cases = []
        except json.JSONDecodeError:
            logging.warning("原有的 tail_code 不是有效的 JSON 数组，将忽略原数据。")
            existing_cases = []

    seen_inputs = set()
    merged_cases = []
    
    # 优先保留原有的测试用例
    for case in existing_cases + new_cases:
        if isinstance(case, dict) and 'input' in case and 'expect' in case:
            # 标准化 input 用于去重比较
            input_str = str(case['input']).strip()
            if input_str not in seen_inputs:
                seen_inputs.add(input_str)
                merged_cases.append(case)
                
    return json.dumps(merged_cases, ensure_ascii=False)

def main():
    processed_numbers = load_progress()
    logging.info(f"已加载 {len(processed_numbers)} 个已处理题目的进度记录。")

    # 连接数据库
    try:
        connection = pymysql.connect(**DB_CONFIG)
        cursor = connection.cursor()
        logging.info("成功连接到 MySQL 数据库。")
    except Exception as e:
        logging.error(f"连接数据库失败: {e}")
        return

    try:
        cursor.execute("SELECT number, description, tail_code FROM oj_questions")
        questions = cursor.fetchall()
        logging.info(f"数据库中共有 {len(questions)} 道题目。")

        for q in questions:
            number = str(q['number'])
            
            # 如果已经处理过，直接跳过（支持断点续传）
            if number in processed_numbers:
                logging.debug(f"跳过已处理题目: {number}")
                continue

            logging.info(f"开始处理题目 {number} ...")
            
            description = q['description']
            existing_tail_code = q['tail_code']

            # 请求大模型生成用例
            new_cases = call_deepseek_api(description)
            
            if new_cases:
                # 合并并去重
                updated_tail_code = merge_and_deduplicate(existing_tail_code, new_cases)
                
                try:
                    # 更新回数据库
                    cursor.execute(
                        "UPDATE oj_questions SET tail_code = %s WHERE number = %s",
                        (updated_tail_code, number)
                    )
                    connection.commit()
                    logging.info(f"成功更新题目 {number} 的测试用例。")
                    
                    # 记录进度
                    processed_numbers.add(number)
                    save_progress(processed_numbers)
                except Exception as e:
                    connection.rollback()
                    logging.error(f"题目 {number} 数据库更新失败: {e}")
            else:
                logging.error(f"题目 {number} 测试用例生成失败，重试 {MAX_RETRIES} 次后放弃。")

            # 速率限制：每次处理完一题后暂停，避免触发 API 限流
            time.sleep(RATE_LIMIT_DELAY)

    except Exception as e:
        logging.error(f"运行过程中发生错误: {e}")
    finally:
        cursor.close()
        connection.close()
        logging.info("数据库连接已关闭。")

if __name__ == '__main__':
    main()
