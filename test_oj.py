import requests
import json
import time

base_url = "http://127.0.0.1:8094"
session = requests.Session()

# 1. Login
login_data = {"username":"admin123", "password":"Ahqjhqj73@you"}
print("--- Login ---")
try:
    resp = session.post(f"{base_url}/api/login", json=login_data)
    print("Status Code:", resp.status_code)
    print("Response:", resp.text)
    print("Cookies:", session.cookies.get_dict())
except Exception as e:
    print(f"Login failed: {e}")

# 2. Get Problems
print("\n--- Get Problems ---")
try:
    resp = session.get(f"{base_url}/api/problems")
    print("Status Code:", resp.status_code)
    if resp.status_code == 200:
        problems = resp.json()
        if len(str(problems)) > 200:
            print("Problems:", str(problems)[:200] + "...")
        else:
            print("Problems:", problems)
except Exception as e:
    print(f"Get Problems failed: {e}")

# 3. Submit Code
def submit_code(name, code):
    print(f"\n--- Submitting {name} ---")
    data = {
        "code": code,
        "input": "",
        "cpu_limit": 1,
        "mem_limit": 102400
    }
    try:
        resp = session.post(f"{base_url}/judge/1", json=data)
        print("Status Code:", resp.status_code)
        try:
            print("Response:", json.dumps(resp.json(), indent=2, ensure_ascii=False))
        except:
            print("Response:", resp.text)
    except Exception as e:
        print(f"Submit Code failed: {e}")

ac_code = """#include<stdio.h>
int main(){
printf("                ********\\n               ************\\n               ####....#.\\n             #..###.....##....\\n             ###.......######              ###            ###\\n                ...........               #...#          #...#\\n               ##*#######                 #.#.#          #.#.#\\n            ####*******######             #.#.#          #.#.#\\n           ...#***.****.*###....          #...#          #...#\\n           ....**********##.....           ###            ###\\n           ....****    *****....\\n             ####        ####\\n           ######        ######\\n##############################################################\\n#...#......#.##...#......#.##...#......#.##------------------#\\n###########################################------------------#\\n#..#....#....##..#....#....##..#....#....#####################\\n##########################################    #----------#\\n#.....#......##.....#......##.....#......#    #----------#\\n##########################################    #----------#\\n#.#..#....#..##.#..#....#..##.#..#....#..#    #----------#\\n##########################################    ############\\n");return 0;
}"""

wa_code = """#include<stdio.h>
int main(){
    printf("Wrong Answer\\n");
    return 0;
}"""

tle_code = """#include<stdio.h>
int main(){
    while(1);
    return 0;
}"""

mle_code = """#include<stdio.h>
#include<stdlib.h>
int main(){
    while(1){
        malloc(1024 * 1024 * 10);
    }
    return 0;
}"""

ce_code = """#include<stdio.h>
int main(){
    printf("Missing semicolon\\n")
    return 0;
}"""

empty_code = ""

submit_code("AC Code", ac_code)
submit_code("WA Code", wa_code)
submit_code("TLE Code", tle_code)
submit_code("MLE Code", mle_code)
submit_code("CE Code", ce_code)
submit_code("Empty Code", empty_code)
