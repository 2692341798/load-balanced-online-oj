# 竞赛模块部署文档

## 1. 模块概述
本模块包含两个独立的爬虫工具：
1. **竞赛爬虫 (`contest_crawler`)**：抓取 Codeforces 和 LeetCode 的竞赛信息。
2. **洛谷爬虫 (`luogu_crawler`)**：抓取洛谷题目详情（标题、描述、测试用例）。

爬虫负责获取数据并存储到 MySQL 数据库（及可选的 Redis 缓存）。
主服务器从数据库读取数据并渲染前端页面。

**技术选型说明**：
为了保持技术栈一致性、简化部署运维（无需维护 Python/Node.js 环境）以及利用现有的 C++ 基础设施，所有爬虫均采用 C++ 编写。

## 2. 编译爬虫
爬虫源代码位于 `crawler/` 目录下。

**依赖**：
- `jsoncpp`
- `openssl`
- `libcurl` (新增，用于洛谷爬虫)
- `hiredis` (可选，默认关闭)

**编译命令**：
```bash
cd crawler
# 编译竞赛爬虫
make contest_crawler
# 编译洛谷爬虫
make luogu_crawler
```
编译成功后会生成 `contest_crawler` 和 `luogu_crawler` 可执行文件。

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

## 5. 洛谷爬虫使用
洛谷爬虫为命令行工具，用于按需抓取题目。

**运行方式**：
```bash
# 抓取特定题目 (例如 P1000)
./luogu_crawler P1000

# 批量抓取 (需要编写脚本循环调用或修改源码支持)
```
注意：洛谷有严格的反爬机制，建议控制抓取频率。

## 6. 目录结构
- `crawler/`: 爬虫源码及 Makefile
- `logs/`: 存放爬虫日志
- `oj_server/resources/template_html/contest.html`: 前端模板

## 7. 法律风险与合规说明
1. **仅公开数据**：本爬虫仅抓取 Codeforces 和 LeetCode 公开的竞赛列表（名称、时间、链接），不涉及用户隐私数据或受版权保护的题目内容。
2. **频率限制**：爬虫严格遵守 rate limiting，请求间隔大于 3 秒，且设计为每天仅执行一次，对目标站点服务器负载影响极小。
3. **User-Agent**：使用了明确的 Bot 标识 (`ContestBot/1.0`)，便于管理员识别。
4. **非商业用途**：本项目为毕业设计/学习用途，不用于商业盈利。
5. **免责声明**：使用本模块产生的一切后果由使用者承担。如果 Codeforces 更新 `robots.txt` 禁止此类抓取，请立即停止使用。

## 7. 验证
1. 运行 `./contest_crawler`，检查 `logs/crawler.log` 是否显示 "Crawler finished"。
2. 检查 MySQL `contests` 表是否有数据（或访问 `GET /api/contests` 确认能返回 JSON 列表）。
3. 启动 `oj_server`，访问 `/contest` 路由，确认页面能否正常显示竞赛列表。

---

**文档版本**: v1.0.3  
**最后更新时间**: 2026-02-09  
**维护团队**: 在线评测系统开发团队
