import requests

try:
    resp = requests.get("http://localhost:8094/", timeout=5)
    print(f"Status Code: {resp.status_code}")
    print("Headers:", resp.headers)
    print("Content Preview:\n", resp.text[:1000])
except Exception as e:
    print(f"Error: {e}")
