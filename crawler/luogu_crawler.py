import requests
import time
import json
import random
import os
import urllib.parse
import re

# Configuration
START_PAGE = 1
END_PAGE = 2  # Crawl first 2 pages (about 100 problems)
OUTPUT_FILE = "insert_luogu_questions.sql"

HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36'
}

def get_difficulty(diff_int):
    # Luogu difficulty: 0=Entry, 1=Junior-, 2=Junior, 3=Junior+, 4=Senior-, 5=Senior, 6=NOI-, 7=NOI
    # Mapping to: 简单 (Simple), 中等 (Medium), 困难 (Hard)
    if diff_int <= 1:
        return '简单'
    elif diff_int <= 3:
        return '中等'
    else:
        return '困难'

def escape_sql(text):
    if text is None:
        return ""
    return str(text).replace("'", "''").replace("\\", "\\\\")

def extract_data(html):
    # New format: <script id="lentille-context" type="application/json">{...}</script>
    pattern = r'<script id="lentille-context" type="application/json">(.*?)</script>'
    match = re.search(pattern, html, re.DOTALL)
    if match:
        try:
            return json.loads(match.group(1))
        except Exception as e:
            print(f"Error parsing JSON from lentille-context: {e}")
            return None
            
    # Fallback to old format just in case
    pattern_old = r'decodeURIComponent\("([^"]+)"\)'
    match_old = re.search(pattern_old, html)
    if match_old:
        encoded_json = match_old.group(1)
        try:
            decoded_json = urllib.parse.unquote(encoded_json)
            return json.loads(decoded_json)
        except Exception as e:
            print(f"Error decoding JSON (old format): {e}")
            return None
            
    return None

def crawl():
    print(f"Starting crawl from page {START_PAGE} to {END_PAGE}...")
    
    sqls = []
    
    for page in range(START_PAGE, END_PAGE + 1):
        list_url = f"https://www.luogu.com.cn/problem/list?page={page}"
        print(f"Fetching list page {page}: {list_url}")
        
        try:
            resp = requests.get(list_url, headers=HEADERS)
            if resp.status_code != 200:
                print(f"Failed to fetch page {page}: Status {resp.status_code}")
                continue
            
            data = extract_data(resp.text)
            if not data:
                print(f"Failed to extract data from page {page}")
                continue
            
            # The structure is data['data']['problems']['result'] based on the log
            # Log: {"instance":"main", ..., "data":{"problems":{...}}}
            
            if 'data' in data and 'problems' in data['data']:
                problems = data['data']['problems']['result']
            elif 'currentData' in data: # Old format check
                problems = data['currentData'].get('problems', {}).get('result', [])
            else:
                print(f"Unexpected JSON structure in page {page}")
                continue
            
            for p in problems:
                pid = p['pid']
                title = p['title']
                difficulty_int = p['difficulty']
                
                print(f"  Processing {pid}: {title}...")
                
                # Fetch problem details
                detail_url = f"https://www.luogu.com.cn/problem/{pid}"
                try:
                    d_resp = requests.get(detail_url, headers=HEADERS)
                    if d_resp.status_code != 200:
                        print(f"    Failed to fetch details for {pid}")
                        continue
                        
                    d_data = extract_data(d_resp.text)
                    if not d_data:
                        print(f"    Failed to extract data for {pid}")
                        continue
                    
                    # Structure for problem details: data['data']['problem']
                    if 'data' in d_data and 'problem' in d_data['data']:
                        p_data = d_data['data']['problem']
                    elif 'currentData' in d_data:
                        p_data = d_data['currentData'].get('problem', {})
                    else:
                        print(f"    Unexpected JSON structure for {pid}")
                        continue
                    
                    # Debug P1001 removed
                    
                    # Extract fields
                    # Content is now nested in 'content' dict
                    content_dict = {}
                    if 'content' in p_data and isinstance(p_data['content'], dict):
                        content_dict = p_data['content']
                    
                    desc = content_dict.get('description', '')
                    background = content_dict.get('background', '')
                    input_fmt = content_dict.get('formatI', '')
                    output_fmt = content_dict.get('formatO', '')
                    hint = content_dict.get('hint', '')
                    
                    # Combine description parts
                    full_desc = ""
                    if background:
                        full_desc += f"## 题目背景\n{background}\n\n"
                    full_desc += f"## 题目描述\n{desc}\n\n"
                    full_desc += f"## 输入格式\n{input_fmt}\n\n"
                    full_desc += f"## 输出格式\n{output_fmt}\n\n"
                    
                    # Samples
                    samples = p_data.get('samples', [])
                    formatted_samples = []
                    
                    full_desc += f"## 样例\n\n"
                    for i, s in enumerate(samples):
                        input_val = s[0]
                        output_val = s[1]
                        formatted_samples.append({
                            "input": input_val,
                            "expect": output_val
                        })
                        full_desc += f"输入 #{i+1}:\n```\n{input_val}\n```\n输出 #{i+1}:\n```\n{output_val}\n```\n\n"
                    
                    if hint:
                        full_desc += f"## 说明/提示\n{hint}\n"
                    
                    # Limits
                    limits = p_data.get('limits', {})
                    time_arr = limits.get('time', [1000])
                    time_limit = int(max(time_arr) / 1000)
                    if time_limit < 1: time_limit = 1
                    
                    mem_arr = limits.get('memory', [128000]) # KB
                    mem_limit = int(max(mem_arr)) # KB
                    
                    star = get_difficulty(difficulty_int)
                    tail_json = json.dumps(formatted_samples, ensure_ascii=False)
                    
                    # Construct SQL
                    # Using 'tail_code' as per actual database schema (though doc says 'tail')
                    sql = (
                        f"INSERT INTO oj_questions (title, star, cpu_limit, mem_limit, description, header, tail_code, status) "
                        f"VALUES ('{pid} {escape_sql(title)}', '{star}', {time_limit}, {mem_limit}, "
                        f"'{escape_sql(full_desc)}', '', '{escape_sql(tail_json)}', 1);"
                    )
                    sqls.append(sql)
                    
                    # Rate limiting
                    time.sleep(random.uniform(1.0, 2.5))
                    
                except Exception as e:
                    print(f"    Error fetching details for {pid}: {e}")
                    
        except Exception as e:
            print(f"Error fetching list page {page}: {e}")
            
    # Write to file
    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write("-- Generated by luogu_crawler.py\n")
        for sql in sqls:
            f.write(sql + "\n")
            
    print(f"Successfully generated {len(sqls)} questions in {OUTPUT_FILE}")

if __name__ == "__main__":
    crawl()
