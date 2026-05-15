# Round 2（按业务域）继续拆分实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans（本会话内执行）  
>
> **Goal:** 继续将 `oj_server/oj_control.hpp` 与 `oj_server/oj_model.hpp` 按业务域拆分成更小的模块文件，并将两者最终门面化。  
>
> **Architecture:** 保持单编译单元（`oj_server.cc`）不变，拆分出的实现仍以头文件内 `inline` 形式存在，由聚合头 `control/control.hpp` 与 `model/model.hpp` 统一 include。  
>
> **Tech Stack:** C++11, httplib, jsoncpp, MySQL client, OpenSSL
>
> 关联设计文档：[2026-05-13-split-large-files-design.md](../specs/2026-05-13-split-large-files-design.md#L181)

---

## 0. 门禁与原则

- **每完成一个业务域迁移就编译一次**：`make -C oj_server clean && make -C oj_server`
- **只做拆分，不改行为**：不更改路由、SQL、JSON 字段、错误码与文案
- **保持对外 include 入口稳定**：最终 `oj_control.hpp`/`oj_model.hpp` 作为门面头文件
- **不引入新依赖**、不修改 `oj_server/makefile`

## 1. 当前状态快照（已完成的拆分）

控制层已拆：
- `oj_server/control/types.hpp`（SerializeJson / Session / Machine / LoadBlance）
- `oj_server/control/impl/upload.hpp`
- `oj_server/control/impl/auth.hpp`

数据层已拆：
- `oj_server/model/types.hpp`（实体结构体）
- `oj_server/model/core.hpp`（连接池/ConnectionGuard/Cache）

## 2. 目标文件结构（Round 2 结束态）

```
oj_server/
  oj_control.hpp                 # 门面 -> control/control.hpp
  oj_model.hpp                   # 门面 -> model/model.hpp
  control/
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
  model/
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

## 3. Task 1：完成控制层按业务域拆分（基于现有 oj_control.hpp）

**Files:**
- Create:
  - `oj_server/control/control.hpp`
  - `oj_server/control/impl/admin.hpp`
  - `oj_server/control/impl/pages.hpp`
  - `oj_server/control/impl/contest.hpp`
  - `oj_server/control/impl/judge.hpp`
  - `oj_server/control/impl/submissions.hpp`
  - `oj_server/control/impl/discussions.hpp`
  - `oj_server/control/impl/training.hpp`
- Modify:
  - `oj_server/oj_control.hpp`（逐段替换为 include 分片，最终门面化）

- [ ] **Step 1.1：创建 `control/control.hpp`（聚合入口）**
  - 内容：
    - `#include "types.hpp"`
    - `#include "../oj_model.hpp"`, `#include "../oj_view.hpp"`, `#include "../deepseek_api.hpp"`
    - `namespace ns_control { ... class Control { ... }; }`
    - `#include "impl/upload.hpp"`、`#include "impl/auth.hpp"`、以及本任务新增的其他 `impl/*.hpp`

- [ ] **Step 1.2：从 `oj_control.hpp` 抽取 admin 域**
  - Move 到 `control/impl/admin.hpp`：
    - `AllQuestionsAdmin`
    - `GetOneQuestionAdmin`
    - `AddQuestion` / `UpdateQuestion` / `DeleteQuestion`
    - `LogAdminOp`
    - `UpdateUserStatus` / `UpdateUserRole` / `ResetUserPassword` / `GenerateInvitationCode` / `GetLogs` / `GetUsers` / `GetDashboardStats` / `AdminRegister`（如存在）
  - 在 `oj_control.hpp` 原位置替换为 `#include "control/impl/admin.hpp"`

- [ ] **Step 1.3：抽取 pages 域**
  - Move 到 `control/impl/pages.hpp`：
    - `Home` / `AllQuestions` / `Question` / `GetQuestionJson`
    - `LoginPage`
    - `TrainingListPage` / `TrainingDetail`
    - `DiscussionPage`
    - `Contest`（页面）与 `GetProfile`（页面）等纯 HTML 渲染方法
  - 在 `oj_control.hpp` 原位置替换为 include

- [ ] **Step 1.4：抽取 contest/judge/submissions/discussions/training 域**
  - `control/impl/contest.hpp`：`GetContestList`
  - `control/impl/judge.hpp`：`Judge`、`GenerateAiHint`
  - `control/impl/submissions.hpp`：`SearchSubmissions`
  - `control/impl/discussions.hpp`：内联评论 + 文章评论 + 讨论 CRUD + query by qid
  - `control/impl/training.hpp`：题单 CRUD + add/remove/reorder + 列表/详情 JSON
  - 在 `oj_control.hpp` 对应位置替换为 include

- [ ] **Step 1.5：将 `oj_control.hpp` 最终门面化**
  - 内容最终应为：

```cpp
#pragma once
#include "control/control.hpp"
```

- [ ] **Step 1.6：编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：成功（允许保留既有 OpenSSL deprecated 警告）

## 4. Task 2：完成数据层按业务域拆分（基于现有 oj_model.hpp）

**Files:**
- Create:
  - `oj_server/model/model.hpp`
  - `oj_server/model/impl/sql.hpp`
  - `oj_server/model/impl/questions.hpp`
  - `oj_server/model/impl/users.hpp`
  - `oj_server/model/impl/submissions.hpp`
  - `oj_server/model/impl/inline_comments.hpp`
  - `oj_server/model/impl/discussions.hpp`
  - `oj_server/model/impl/article_comments.hpp`
  - `oj_server/model/impl/contests.hpp`
  - `oj_server/model/impl/training.hpp`
  - `oj_server/model/impl/admin.hpp`
  - `oj_server/model/impl/stats.hpp`
- Modify:
  - `oj_server/oj_model.hpp`（逐段替换为 include 分片，最终门面化）

- [ ] **Step 2.1：创建 `model/model.hpp`（聚合入口）**
  - 内容：
    - include 现有：`model/types.hpp`、`model/core.hpp`
    - include MySQL/OpenSSL 依赖（沿用原 `oj_model.hpp`）
    - 保留原命名空间 `ns_model` 与 `using namespace` 组合
    - 放置 `class Model` 的声明（字段与方法签名）
    - 末尾 include `impl/*.hpp`

- [ ] **Step 2.2：抽取 sql 域（通用 DB 工具）**
  - Move 到 `model/impl/sql.hpp`：
    - `ExecuteSql`
    - `QueryMySql`
    - `QueryUserMySql`
    - `CheckAndUpgradeTable`、各 `Init*Table`（如你希望更细，也可拆为 `schema.hpp`，但本轮先放 sql.hpp）
  - 在 `oj_model.hpp` 原位置替换为 include

- [ ] **Step 2.3：按业务域抽取 Model 方法**
  - `questions.hpp`：GetAllQuestions/GetQuestionsByPage/GetAllQuestionsAdmin/GetOneQuestion/AddQuestion/UpdateQuestion/DeleteQuestion
  - `users.hpp`：RegisterUser/LoginUser/UpdateUser/GetUsers/UpdateUserStatus/UpdateUserRole/ResetUserPassword/GetUserById/VerifyAndUseInvitationCode/GenerateInvitationCode
  - `submissions.hpp`：AddSubmission/GetSubmissions/GetUserSolvedStats
  - `inline_comments.hpp`：AddInlineComment/GetInlineComments/DeleteInlineComment
  - `discussions.hpp`：AddDiscussion/DeleteDiscussion/GetAllDiscussions/GetDiscussionsByQuestionId/GetOneDiscussion
  - `article_comments.hpp`：AddArticleComment/GetArticleComments
  - `contests.hpp`：UpsertContest/GetContests
  - `training.hpp`：CreateTrainingList/UpdateTrainingList/DeleteTrainingList/GetTrainingList/GetTrainingLists/AddProblemToTrainingList/RemoveProblemFromTrainingList/ReorderTrainingListProblems/GetTrainingListProblems
  - `admin.hpp`：LogOperation/GetLogs/InitInvitationCodeTable/InitOperationLogTable（如仍在 Model 内）
  - `stats.hpp`：GetTotalUserCount/GetTotalProblemCount/GetTotalSubmissionCount/GetUserGrowthStats/GetSubmissionStats/GetDailyActivityStats

- [ ] **Step 2.4：将 `oj_model.hpp` 最终门面化**
  - 内容最终应为：

```cpp
#pragma once
#include "model/model.hpp"
```

- [ ] **Step 2.5：编译验证**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：成功（允许保留既有 OpenSSL deprecated 警告）

## 5. Task 3：最终收尾验证

- [ ] **Step 3.1：全量 clean build**
  - Run：`make -C oj_server clean && make -C oj_server`
  - Expected：成功生成 `oj_server/oj_server`

- [ ] **Step 3.2：检查 include 入口**
  - `oj_server/oj_server.cc` 仍只 include `oj_control.hpp`
  - `oj_control.hpp`/`oj_model.hpp` 均已门面化

