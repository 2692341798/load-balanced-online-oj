#!/bin/bash
cd /Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/tests/e2e
> test_results.txt
for c in 10 11 12 13 14 15 16 20 25 30 40 45 50 60; do
  echo "Running with $c users..."
  output=$(CONCURRENT_USERS=$c npx playwright test tests/load.spec.ts 2>&1)
  max_time=$(echo "$output" | sed 's/\x1b\[[0-9;]*m//g' | grep -o '最大响应时间: [0-9]*' | grep -o '[0-9]*')
  if [ -z "$max_time" ]; then
      echo "Failed to get max time for $c. Output: $output"
      max_time="ERROR"
  fi
  echo "Max response time for $c: $max_time"
  echo "$max_time" >> test_results.txt
done
echo "All done. Results in test_results.txt"
