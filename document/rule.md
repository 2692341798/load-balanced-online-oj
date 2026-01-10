# 项目开发规则（AI协作版）

## 1. 代码开发前准备
- **必须完整阅读核心文档**：
  * 架构文档 [`/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/architecture.md`](file:///Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/architecture.md)
  * 数据库设计 [`/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/database.md`](file:///Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/database.md)
  * API规范 [`/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/api_reference.md`](file:///Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/api_reference.md)
  * 前端样式指南 [`/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/frontend_style_guide.md`](file:///Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/frontend_style_guide.md)
- **理解系统架构**：主服务器+编译服务器集群的分布式架构，负载均衡机制
- **熟悉技术栈**：C++11、MySQL、httplib、CTemplate、深色主题前端

## 2. 开发约束条件
- **严格遵循规范**：
  * API接口规范（状态码、请求格式、认证方式）
  * 前端样式指南（CSS变量、响应式设计、组件规范）
  * 架构设计原则（模块化、负载均衡、会话管理）
- **保持技术一致性**：使用项目现有库和工具，不引入新依赖

## 3. 开发流程
1. **文档先行**：功能开发前先创建设计文档
2. **小步迭代**：单功能开发，及时测试验证
3. **测试驱动**：功能完成后立即编写单元测试
4. **质量保证**：所有测试通过后才提交代码
5. **日志更新**：同步更新开发日志 [`/Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/comprehensive_dev_log.md`](file:///Users/huangqijun/Documents/毕业设计/load-balanced-online-oj/document/comprehensive_dev_log.md) 并推送到GitHub

## 4. 代码质量要求
- **命名规范**：全拼命名，见名知意（循环变量i除外）
- **函数设计**：单一职责，长度控制在20行内
- **注释标准**：解释"为什么"而非"做什么"
- **重构节奏**：测试绿灯后才允许推送代码

## 5. 性能优化原则
- **资源管理**：减少对象创建，及时释放资源
- **计算优化**：避免重复计算，缓存可复用结果
- **并发处理**：合理使用并行，同步操作精简
- **系统性能**：响应时间<100ms，评测<5s，并发100+

## 6. 安全设计规范
- **用户认证**：SHA256密码哈希，24小时会话过期
- **输入验证**：参数长度、格式、SQL注入防护
- **代码执行**：沙箱机制，资源限制，降权运行
- **错误处理**：友好提示，不暴露内部信息

## 7. 前端开发规范
- **样式系统**：CSS变量统一管理，深色主题优先
- **响应式设计**：移动端优先，断点768px/1024px
- **组件化**：卡片、按钮、表单标准化
- **可访问性**：WCAG 2.1 AA标准，键盘导航支持

## 8. 架构维护要求
- **负载均衡**：智能选择最优编译服务器
- **故障处理**：自动重试、服务降级、故障转移
- **监控指标**：成功率>95%，响应时间<2.5s，可用性>99%
- **扩展性**：支持水平扩展，无状态设计

## 9. 文档维护标准
- **实时更新**：功能变更同步更新对应文档
- **版本管理**：每次重大更新记录版本历史
- **测试报告**：每个功能都要有验证报告
- **问题追踪**：bug修复需要详细记录根因和解决方案

## 10. 协作开发准则
- **代码审查**：遵循项目现有代码风格和模式
- **增量开发**：避免大规模重构，小步快跑
- **兼容性**：保持向后兼容，平滑升级
- **知识共享**：复杂逻辑要有详细注释和文档说明

## 11. 技术债务管理
- **定期清理**：识别并解决技术债务
- **优先级排序**：按影响程度排序处理
- **渐进式改进**：避免一次性大规模重构
- **文档记录**：技术债务清单和解决计划

## 12. 质量保证机制
- **代码审查**：所有代码变更需要审查
- **自动化测试**：单元测试、集成测试全覆盖
- **性能监控**：关键指标实时监控
- **错误追踪**：完整的错误日志和追踪机制

---

**文档版本**: v0.2.6  
**最后更新时间**: 2026-01-10  
**维护团队**: 在线评测系统开发团队  
**适用范围**: AI辅助开发场景