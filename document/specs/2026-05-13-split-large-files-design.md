# 拆分大文件与目录结构优化（最小拆分）设计文档

## 1. 背景与动机

当前仓库中存在少量超大源码文件，导致：

- 单文件职责过多，阅读与定位成本高
- 容易产生“改一处牵一片”的风险
- 新功能/修复难以按领域聚合，代码组织持续恶化

本次目标是在不改变现有构建方式与运行行为的前提下，对项目自有代码进行“最小拆分”，让核心模块更易维护。

参考：
- 仓库架构说明：[architecture.md](../architecture.md)
- API 约定与模块划分：[api_reference.md](../api_reference.md)
- 个人知识库概念卡：[[在线评测系统（Online Judge，OJ）]]

## 2. 目标与非目标

### 2.1 目标

- 将超大头文件按职责拆分为多个小文件（重点：`oj_server/oj_control.hpp`、`oj_server/oj_model.hpp`）
- 拆分后保持对外 include 入口稳定，尽量不需要全仓库批量改 include
- 不改变现有的编译方式：仍由 `oj_server/oj_server.cc` 作为单编译单元进行编译链接（当前 `oj_server/makefile` 为单文件编译）
- 为后续继续模块化（多 .cc 编译）预留清晰的目录边界

### 2.2 非目标

- 不重写或拆分第三方库文件（例如 `comm/httplib.h`）
- 不进行跨模块的大规模重构（例如引入新的构建系统、引入新依赖、接口风格重塑）
- 不在本次把实现迁移到多个 `.cc` 并改 makefile（属于“模块化构建”阶段，不在本次范围）

## 3. 现状与问题定位

已识别的超大文件（以行数衡量）：

- `comm/httplib.h`（第三方库，约 6700 行）——本次不动
- `oj_server/oj_control.hpp`（约 2700 行）——本次拆分重点
- `oj_server/oj_model.hpp`（约 2600 行）——本次拆分重点

## 4. 总体方案

### 4.1 拆分策略（最小拆分）

保持 `oj_server/oj_control.hpp` 与 `oj_server/oj_model.hpp` 文件名不变，将其改造成“门面头文件（facade header）”，仅负责聚合 include：

- `oj_control.hpp` → `#include "control/control.hpp"`
- `oj_model.hpp` → `#include "model/model.hpp"`

其余代码移动到新的目录结构中，并通过 include 聚合保证对外可用。

### 4.2 新目录结构（仅影响 oj_server 内部）

在 `oj_server/` 下新增：

- `oj_server/control/`
  - `control.hpp`：`class Control` 声明与依赖聚合
  - `types.hpp`：Session、Machine、LoadBlance 等控制层类型（如存在）
  - `impl/`：按功能域拆分 `Control::xxx` 的成员函数实现（以 `inline` 形式）
- `oj_server/model/`
  - `types.hpp`：Question/User/Submission/Discussion 等实体结构体
  - `model.hpp`：`class Model` 对外接口声明与配置常量
  - `impl/`：按业务域拆分 Model 的 DB 实现（以 `inline` 形式）

示例布局（最终以实际拆分结果为准）：

```
oj_server/
  oj_control.hpp              # 门面
  oj_model.hpp                # 门面
  control/
    control.hpp
    types.hpp
    impl/
      auth.hpp
      problems.hpp
      judge.hpp
      discussions.hpp
      training.hpp
      contests.hpp
      admin.hpp
      upload.hpp
  model/
    types.hpp
    model.hpp
    impl/
      users.hpp
      questions.hpp
      submissions.hpp
      discussions.hpp
      comments.hpp
      contests.hpp
      training.hpp
      admin.hpp
```

### 4.3 编译与链接保持不变

仍然由 `oj_server/oj_server.cc` 编译，拆分出的实现将以 `inline` + `#include` 的方式被同一个编译单元吸收，确保：

- 不需要修改 `oj_server/makefile` 的构建规则
- 不引入跨 TU 的符号链接问题

## 5. 详细拆分方案

### 5.1 oj_control.hpp 拆分建议

`oj_control.hpp` 内目前混合了：

- 控制器类 `Control`（业务编排、路由处理相关）
- 负载均衡相关类型与逻辑（Machine、LoadBlance）
- 会话/认证相关（Session、sessions_）
- 上传相关（UploadImage/UploadAvatar）
- 与 `Model`/`View` 交互的多种业务方法

拆分到：

- `control/types.hpp`
  - Session、Machine、LoadBlance
  - SerializeJson（如与控制层强绑定）
- `control/control.hpp`
  - `class Control` 的声明、成员字段
  - `#include "impl/*.hpp"` 聚合实现分片
- `control/impl/*.hpp`
  - 按 API/页面域拆分成员函数定义（例如 auth/problems/judge/discussions/training/contests/admin/upload 等）

### 5.2 oj_model.hpp 拆分建议

`oj_model.hpp` 内目前混合了：

- 大量实体结构体定义
- MySQL/加密工具（SHA）
- 环境变量读取与常量表名定义
- `class Model` 的 DB 操作实现（覆盖用户/题目/提交/讨论/评论/题单/竞赛/管理员等）

拆分到：

- `model/types.hpp`
  - 所有实体结构体：Question/User/Submission/Discussion/InlineComment/ArticleComment/Contest/TrainingList/TrainingListItem/OperationLog/InvitationCode 等
- `model/model.hpp`
  - 表名常量
  - `GetEnv`、MySQL 连接配置常量
  - `class Model` 的接口声明
  - `#include "impl/*.hpp"` 聚合实现分片
- `model/impl/*.hpp`
  - 按领域拆分 Model 方法定义（users/questions/submissions/discussions/comments/contests/training/admin 等）

## 6. 兼容性与风险控制

### 6.1 include 兼容

通过保留 `oj_control.hpp` / `oj_model.hpp` 作为门面头文件，避免改动：

- `oj_server.cc` 中的 `#include "oj_control.hpp"`
- 其他可能包含这两个头文件的源码

### 6.2 行为一致性

拆分仅改变“代码物理组织”，不改变：

- HTTP 路由行为（仍由 `oj_server.cc` 调用 `Control` 的同名方法）
- JSON 字段与接口返回（遵循现有 `api_reference.md`）
- 数据库交互逻辑（仍复用现有 SQL/字段映射）

### 6.3 已知风险

- 拆分后 include 顺序变化可能暴露“隐式依赖”（例如某段代码以前靠上方 include 间接引入类型/函数）
  - 缓解：为每个新文件显式补齐必要 include，并以 `make -C oj_server` 作为门禁验证

## 7. 验证计划

- 编译验证：`make -C oj_server clean && make -C oj_server`
- 冒烟验证（可选）：启动 `oj_server` 后访问 `/`、`/all_questions`、`/api/problems` 确认无崩溃且能正常响应

## 8. 实施边界（本次交付）

- 新增上述目录与文件，并完成 `oj_control.hpp/oj_model.hpp` 的门面化
- 保持 `oj_server/makefile` 不变
- 不移动/改写第三方库文件

## 9. 第二阶段：按业务域继续拆分（Round 2）

> 本阶段在“保持单编译单元 + 头文件内联实现”的前提下，把剩余的大块逻辑继续按业务域拆分，目标是让每个文件聚焦一个业务域（约 150-400 行/文件），降低后续维护成本。

### 9.1 总体策略

- 将 `oj_server/oj_control.hpp` 进一步门面化为 `#include "control/control.hpp"`（对外 include 入口仍稳定）
- 将 `oj_server/oj_model.hpp` 进一步门面化为 `#include "model/model.hpp"`
- `control/control.hpp` 与 `model/model.hpp` 作为模块聚合入口：
  - 放置 `class Control` / `class Model` 的声明（成员字段与方法签名）
  - 在末尾 `#include "impl/*.hpp"` 聚合对应的内联实现分片

### 9.2 控制层拆分目录

```
oj_server/control/
  types.hpp
  control.hpp
  impl/
    upload.hpp
    auth.hpp
    admin.hpp
    pages.hpp
    contest.hpp
    judge.hpp
    submissions.hpp
    discussions.hpp
    training.hpp
```

拆分原则（按业务域）：

- **upload**：文件/头像上传相关
- **auth**：注册/登录/会话鉴权/登出
- **admin**：后台管理 API（题目管理、用户管理、日志、统计、邀请码、管理员注册等）
- **pages**：HTML 页面渲染（Home/AllQuestions/Question/Login/Training/Discussion/Contest/Profile 等）
- **contest**：竞赛数据 API
- **judge**：评测下发与 AI 提示
- **submissions**：提交记录查询与筛选
- **discussions**：讨论与评论（含内联评论、文章评论）
- **training**：题单业务（CRUD、题目增删、排序、列表/详情 JSON）

### 9.3 数据层拆分目录

```
oj_server/model/
  types.hpp
  core.hpp
  model.hpp
  impl/
    sql.hpp
    questions.hpp
    users.hpp
    submissions.hpp
    inline_comments.hpp
    discussions.hpp
    article_comments.hpp
    contests.hpp
    training.hpp
    admin.hpp
    stats.hpp
```

拆分原则（按业务域/表/通用能力）：

- **sql**：`ExecuteSql`、`QueryMySql`、`QueryUserMySql` 等通用 SQL 执行与结果装配
- **questions/users/submissions/...**：按表/域拆分 Model 方法实现
- **admin**：操作日志、邀请码等后台相关表操作
- **stats**：dashboard 所需统计查询

### 9.4 验证门禁

- 每完成一个业务域迁移，执行一次：`make -C oj_server clean && make -C oj_server`
- 本阶段不新增依赖，不修改 `oj_server/makefile`
