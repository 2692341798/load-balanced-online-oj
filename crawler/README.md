# Contest Crawler Module

This module is responsible for fetching contest information from competitive programming platforms (currently Codeforces and LeetCode) and storing it in the database.

## Features

- **Multi-Source Support**: Crawls Codeforces and LeetCode weekly contests.
- **Robots.txt Compliance**: Strictly adheres to `robots.txt` rules (Disallow paths and Crawl-delay).
- **Adaptive Rate Limiting**: Implements exponential backoff for 429/5xx errors and random jitter to avoid fingerprinting.
- **Data Persistence**: Stores contest data in MySQL and optionally caches in Redis.
- **Resilience**: Handles network timeouts, parsing errors, and service interruptions gracefully.

## Dependencies

- **C++11** Compiler
- **jsoncpp**: For parsing LeetCode GraphQL responses.
- **OpenSSL**: For HTTPS support in `httplib`.
- **MySQL Client**: For database connectivity.
- **Hiredis** (Optional): For Redis support (enable via `#define ENABLE_REDIS`).

### Installation (macOS)
```bash
brew install jsoncpp openssl mysql-client
```

## Build & Run

1.  **Build**:
    ```bash
    cd crawler
    make contest_crawler
    ```

2.  **Run**:
    ```bash
    ./contest_crawler
    ```
    The crawler runs as a daemon-like process (infinite loop) with a default interval of 2-4 hours.

3.  **Run Tests**:
    ```bash
    make test_crawler
    ./test_crawler
    ```

## Configuration

Configuration is currently hardcoded in `main()` function of `contest_crawler.cc` but can be easily extended to read from a file.

- **Interval**: 2-4 hours (randomized).
- **User-Agent**: `ContestBot/2.0 (+https://yourdomain.com/bot; contact@yourdomain.com)`

## Compliance Statement

This crawler is designed to be a "good citizen" of the web:
- It respects `robots.txt` of target sites.
- It uses a custom User-Agent to identify itself.
- It limits request frequency and respects `Crawl-delay` directives.
- It only fetches public contest list data.

## Code Structure

- `contest_crawler.cc`: Main service logic, `ContestCrawler` class managing both Codeforces and LeetCode tasks.
- `crawler_common.hpp`: Shared data structures (`Contest`), helper functions (`ParseCodeforcesContests`, `ParseLeetCodeContests`), `RobotsParser`, and `BackoffStrategy`.
