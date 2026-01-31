# 竞赛模块部署文档

## 1. 模块概述
本模块包含一个独立的竞赛爬虫（`contest_crawler`）和一个集成在 `oj_server` 中的展示页面。
爬虫负责从 Codeforces 抓取最新的竞赛信息，并存储到本地 JSON 文件（及可选的 Redis 缓存）。
主服务器读取该 JSON 文件并渲染前端页面。

**技术选型说明**：
尽管原需求提到了 Node.js + Express，但考虑到项目整体架构为 C++ 分布式系统，为了保持技术栈一致性、简化部署运维（无需维护两套运行时环境）以及利用现有的 C++ 模版引擎和权限管理机制，我们选择在现有的 `oj_server` 中集成展示逻辑，并使用 C++ 编写爬虫。这种方案更加轻量级且易于维护。

## 2. 编译爬虫
爬虫源代码位于 `crawler/` 目录下。

**依赖**：
- `jsoncpp`
- `openssl`
- `hiredis` (可选，默认关闭)

**编译命令**：
```bash
cd crawler
make
```
编译成功后会生成 `contest_crawler` 可执行文件。

## 3. Redis 配置 (可选)
如果需要启用 Redis 缓存同步：
1. 确保系统已安装 `hiredis` 库。
2. 修改 `crawler/contest_crawler.cc`，取消 `// #define ENABLE_REDIS` 的注释。
3. 重新执行 `make`。
4. 确保本地 Redis 服务运行在默认端口 6379。

## 4. 定时任务配置 (CRON)
建议每天凌晨 02:00 执行一次爬取任务。

编辑 crontab：
```bash
crontab -e
```

添加如下行（请修改路径为实际路径）：
```bash
0 2 * * * cd /Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/crawler && ./contest_crawler >> ../logs/crawler.log 2>&1
```

## 5. 目录结构
- `crawler/`: 爬虫源码及 Makefile
- `data/`: 存放 `contests.json` 数据文件
- `logs/`: 存放爬虫日志
- `oj_server/template_html/contest.html`: 前端模板

## 6. 法律风险与合规说明
1. **仅公开数据**：本爬虫仅抓取 Codeforces 公开的竞赛列表（名称、时间、链接），不涉及用户隐私数据或受版权保护的题目内容。
2. **频率限制**：爬虫严格遵守 rate limiting，请求间隔大于 3 秒，且设计为每天仅执行一次，对目标站点服务器负载影响极小。
3. **User-Agent**：使用了明确的 Bot 标识 (`ContestBot/1.0`)，便于管理员识别。
4. **非商业用途**：本项目为毕业设计/学习用途，不用于商业盈利。
5. **免责声明**：使用本模块产生的一切后果由使用者承担。如果 Codeforces 更新 `robots.txt` 禁止此类抓取，请立即停止使用。

## 7. 验证
1. 运行 `./contest_crawler`，检查 `logs/crawler.log` 是否显示 "Crawler finished"。
2. 检查 `data/contests.json` 是否有内容。
3. 启动 `oj_server`，访问 `/contest` 路由，确认页面能否正常显示竞赛列表。

---

**文档版本**: v0.4.0  
**最后更新时间**: 2026-01-31  
**维护团队**: 在线评测系统开发团队
