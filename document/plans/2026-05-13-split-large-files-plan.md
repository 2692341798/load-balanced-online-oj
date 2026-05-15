# 拆分大文件与目录结构优化（最小拆分）实施计划

> **适用范围**：本计划严格按“最小拆分”执行，仅拆分 `oj_server/oj_control.hpp` 与 `oj_server/oj_model.hpp`，保持 `oj_server/makefile` 单编译单元方式不变。
>
> 关联设计文档：[2026-05-13-split-large-files-design.md](../specs/2026-05-13-split-large-files-design.md)

## 目标

- 将 `oj_server/oj_control.hpp`、`oj_server/oj_model.hpp` 拆分到更小、更聚合的头文件
- 保留 `oj_control.hpp`、`oj_model.hpp` 为门面头文件，避免全仓库 include 改动
- `make -C oj_server clean && make -C oj_server` 通过

## 变更文件结构（将创建/修改）

**修改：**
- `oj_server/oj_control.hpp`（改为门面 include）
- `oj_server/oj_model.hpp`（改为门面 include）

**创建（控制层）：**
- `oj_server/control/types.hpp`
- `oj_server/control/control.hpp`
- `oj_server/control/impl/upload.hpp`
- `oj_server/control/impl/auth.hpp`
- `oj_server/control/impl/admin.hpp`
- `oj_server/control/impl/pages.hpp`
- `oj_server/control/impl/contest.hpp`
- `oj_server/control/impl/judge.hpp`
- `oj_server/control/impl/profile.hpp`
- `oj_server/control/impl/discussions.hpp`
- `oj_server/control/impl/training.hpp`
- `oj_server/control/impl/submissions.hpp`

**创建（数据层）：**
- `oj_server/model/types.hpp`
- `oj_server/model/model.hpp`
- `oj_server/model/impl/mysql_core.hpp`
- `oj_server/model/impl/contests.hpp`
- `oj_server/model/impl/inline_comments.hpp`
- `oj_server/model/impl/submissions.hpp`
- `oj_server/model/impl/questions.hpp`
- `oj_server/model/impl/users.hpp`
- `oj_server/model/impl/discussions.hpp`
- `oj_server/model/impl/article_comments.hpp`
- `oj_server/model/impl/training.hpp`
- `oj_server/model/impl/admin.hpp`
- `oj_server/model/impl/stats.hpp`

## 执行策略

- 保持“一个编译单元”不变：所有拆出去的实现以 `inline` 成员函数形式存在，并由 `control.hpp`/`model.hpp` 在末尾 `#include` 聚合。
- 拆分过程采用“先复制、后替换、再删除”的顺序，确保每一步都可编译：
  1. 先创建新文件并从原文件复制对应代码
  2. 在原文件中改为 include 新文件（或删除对应代码块）
  3. 每轮执行一次编译验证，及时修正 include/命名空间/前置声明问题

## Task 1：建立门面头文件与新目录骨架

- [ ] **Step 1.1：创建目录与空文件**
  - 创建 `oj_server/control/`、`oj_server/control/impl/`
  - 创建 `oj_server/model/`、`oj_server/model/impl/`

- [ ] **Step 1.2：将 `oj_control.hpp` 改为门面**

```cpp
#pragma once
#include "control/control.hpp"
```

- [ ] **Step 1.3：将 `oj_model.hpp` 改为门面**

```cpp
#pragma once
#include "model/model.hpp"
```

- [ ] **Step 1.4：编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：成功生成 `oj_server/oj_server`

## Task 2：拆分 `oj_control.hpp` 到 `control/`

### Task 2.1：抽取控制层基础类型到 `control/types.hpp`

- [ ] **Step 2.1.1：创建 `oj_server/control/types.hpp`**
  - 复制以下内容（保持原命名空间 `ns_control` 不变）：
    - `SerializeJson`
    - `struct Session`
    - `class Machine`
    - `class LoadBlance`
    - `service_machine` 常量
  - 确保 `#include` 充分（jsoncpp、mutex、thread、chrono、vector、algorithm、fstream、cassert、cstdlib、cstdio、ctime、unordered_map、string、iostream）

- [ ] **Step 2.1.2：编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：通过（如失败，优先补 include、修正命名空间与 using）

### Task 2.2：重建 `Control` 声明 + 分片实现聚合

- [ ] **Step 2.2.1：创建 `oj_server/control/control.hpp`**
  - 顶部引入原 `oj_control.hpp` 中 `Control` 依赖：
    - `../comm/util.hpp`、`../comm/log.hpp`、`../comm/httplib.h`
    - `../oj_server/oj_view.hpp`、`../oj_server/deepseek_api.hpp`
    - `../oj_server/oj_model.hpp`（门面后仍可用）
    - 以及 `control/types.hpp`
  - 保留原命名空间 `ns_control`、原 `using namespace` 组合（避免行为差异）
  - 放置 `class Control` 的完整类定义（成员字段、方法签名）
  - 在文件末尾按顺序 include 实现分片：

```cpp
#include "impl/upload.hpp"
#include "impl/auth.hpp"
#include "impl/admin.hpp"
#include "impl/pages.hpp"
#include "impl/contest.hpp"
#include "impl/judge.hpp"
#include "impl/submissions.hpp"
#include "impl/profile.hpp"
#include "impl/discussions.hpp"
#include "impl/training.hpp"
```

- [ ] **Step 2.2.2：创建实现分片文件（仅复制代码，不改逻辑）**
  - 从原 `oj_control.hpp` 复制对应成员函数定义到以下文件（保持 `inline`）：
    - `impl/upload.hpp`：`UploadImage`、`UploadAvatar`
    - `impl/auth.hpp`：`GenerateToken`、`CheckUserExists`、`Register`、`Login`、`AuthCheck`、`AdminAuthCheck`、`Logout`、`UpdateUserInfo`
    - `impl/admin.hpp`：`AllQuestionsAdmin`、`GetOneQuestionAdmin`、`AddQuestion`、`UpdateQuestion`、`DeleteQuestion`、`LogAdminOp`、`UpdateUserStatus`、`UpdateUserRole`、`GenerateRandomPassword`、`ResetUserPassword`、`AdminRegister`、`GenerateInvitationCode`、`GetLogs`、`GetUsers`、`GetDashboardStats`
    - `impl/pages.hpp`：`Home`、`AllQuestions`、`Question`、`LoginPage`、`DiscussionPage`、`TrainingListPage`、`TrainingDetail`、`Contest`、`GetProfile`
    - `impl/contest.hpp`：`GetContestList`
    - `impl/judge.hpp`：`Judge`、`GenerateAiHint`
    - `impl/submissions.hpp`：`SearchSubmissions`
    - `impl/profile.hpp`：`GetProfileData`
    - `impl/discussions.hpp`：`AddInlineComment`、`GetInlineComments`、`DeleteInlineComment`、`GetAllDiscussions`、`GetDiscussion`、`AddDiscussion`、`DeleteDiscussion`、`GetDiscussionsByQuestionId`、`AddArticleComment`、`GetArticleComments`
    - `impl/training.hpp`：`CreateTrainingList`、`UpdateTrainingList`、`DeleteTrainingList`、`GetQuestionsByPageJson`、`AddProblemsToTrainingList`、`AddProblemToTrainingList`、`RemoveProblemFromTrainingList`、`ReorderTrainingListProblems`、`GetTrainingLists`、`GetTrainingListDetailJson`
  - `RecoveryMachine` 保持在 `control.hpp` 或单独 `impl/machines.hpp`（二选一，优先放入 `auth.hpp` 或 `types.hpp` 附近，避免遗漏）

- [ ] **Step 2.2.3：删除原 `oj_control.hpp` 中被迁移的实现代码**
  - 由于 `oj_control.hpp` 会变为门面文件，原实现整体会被替换；确认无残留逻辑

- [ ] **Step 2.2.4：编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：通过

## Task 3：拆分 `oj_model.hpp` 到 `model/`

### Task 3.1：抽取实体类型到 `model/types.hpp`

- [ ] **Step 3.1.1：创建 `oj_server/model/types.hpp`**
  - 复制所有 `struct`：
    - `Question`、`User`、`InvitationCode`、`OperationLog`、`Submission`
    - `InlineComment`、`Discussion`、`ArticleComment`
    - `Contest`、`TrainingList`、`TrainingListItem`
  - 保持命名空间 `ns_model`

- [ ] **Step 3.1.2：编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：通过

### Task 3.2：重建 `Model` 声明 + 分片实现聚合

- [ ] **Step 3.2.1：创建 `oj_server/model/model.hpp`**
  - 顶部 include：
    - `../comm/util.hpp`、`../comm/log.hpp`
    - `mysql/mysql.h`、`openssl/sha.h`
    - `model/types.hpp`
  - 复制并保留：
    - `GetEnv`
    - 表名常量（`oj_questions`、`users`、`submissions`...）
    - MySQL 连接配置常量（`host/user/passwd/...`）
    - `class MySqlPool`（如存在）、`class Model` 的完整声明（成员字段、方法签名）
  - 在文件末尾 include 实现分片：

```cpp
#include "impl/mysql_core.hpp"
#include "impl/questions.hpp"
#include "impl/users.hpp"
#include "impl/submissions.hpp"
#include "impl/inline_comments.hpp"
#include "impl/discussions.hpp"
#include "impl/article_comments.hpp"
#include "impl/contests.hpp"
#include "impl/training.hpp"
#include "impl/admin.hpp"
#include "impl/stats.hpp"
```

- [ ] **Step 3.2.2：创建实现分片文件（仅复制代码，不改逻辑）**
  - `impl/mysql_core.hpp`：
    - 连接池实现（如 `MySqlPool::GetConnection/ReleaseConnection`）
    - `CheckAndUpgradeTable`、各 `Init*Table`
    - `ExecuteSql`、`QueryMySql`、`QueryUserMySql`
    - `SHA256Hash`
  - `impl/questions.hpp`：`GetAllQuestions`（两份：缓存/DB 侧按原逻辑拆）、`GetQuestionsByPage`、`GetAllQuestionsAdmin`、`GetOneQuestion`、`AddQuestion`、`UpdateQuestion`、`DeleteQuestion`
  - `impl/users.hpp`：`RegisterUser`、`LoginUser`、`UpdateUser`、`GetUsers`、`UpdateUserStatus`、`UpdateUserRole`、`ResetUserPassword`、`GetUserById`、`VerifyAndUseInvitationCode`、`GenerateInvitationCode`
  - `impl/submissions.hpp`：`AddSubmission`、`GetSubmissions`、`GetUserSolvedStats`
  - `impl/inline_comments.hpp`：`AddInlineComment`、`GetInlineComments`、`DeleteInlineComment`
  - `impl/discussions.hpp`：`AddDiscussion`、`DeleteDiscussion`、`GetAllDiscussions`、`GetDiscussionsByQuestionId`、`GetOneDiscussion`
  - `impl/article_comments.hpp`：`AddArticleComment`、`GetArticleComments`
  - `impl/contests.hpp`：`UpsertContest`、`GetContests`
  - `impl/training.hpp`：`CreateTrainingList`、`UpdateTrainingList`、`DeleteTrainingList`、`GetTrainingList`、`GetTrainingLists`、`AddProblemToTrainingList`、`RemoveProblemFromTrainingList`、`ReorderTrainingListProblems`、`GetTrainingListProblems`
  - `impl/admin.hpp`：`LogOperation`、`GetLogs`
  - `impl/stats.hpp`：`GetTotalUserCount/GetTotalProblemCount/GetTotalSubmissionCount`、`GetUserGrowthStats/GetSubmissionStats/GetDailyActivityStats`

- [ ] **Step 3.2.3：确认 `oj_model.hpp` 已仅剩门面 include**

- [ ] **Step 3.2.4：编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：通过

## Task 4：目录结构优化收尾

- [ ] **Step 4.1：检查 include 走向**
  - `oj_server/oj_server.cc` 仍只 include `oj_control.hpp`
  - `control/control.hpp` include `oj_model.hpp`（门面）与 `oj_view.hpp` 等依赖

- [ ] **Step 4.2：确认无多余重复定义**
  - 重点关注：namespace、struct、class 是否被复制导致重复定义
  - 重点关注：`static`/`inline` 与头文件 ODR 问题（本项目保持单 TU 编译，风险较低）

- [ ] **Step 4.3：最终编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：通过

## 验证输出（执行时记录）

- 编译命令与结果截图/日志（至少一次完整 `make -C oj_server clean && make -C oj_server` 成功）

