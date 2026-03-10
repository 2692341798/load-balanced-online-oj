# Crawler 模块说明

## 依赖
本模块依赖 `hiredis` 库来连接 Redis。

### 安装方法
- **macOS**: `brew install hiredis`
- **Ubuntu/Debian**: `sudo apt-get install libhiredis-dev`
- **CentOS/RHEL**: `sudo yum install hiredis-devel`

## RedisQueue 使用
`redis_queue.hpp` 提供了简单的 Redis 队列操作封装。
- `Push(queue, value)`: 入队
- `Pop(queue)`: 出队
- `IsVisitedWithTTL(hash)`: 检查是否已访问（带过期时间）
- `AddVisitedWithTTL(hash, ttl)`: 标记已访问（带过期时间）
