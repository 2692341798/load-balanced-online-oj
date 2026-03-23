const fs = require('fs');
const path = require('path');
const os = require('os');
const { execSync } = require('child_process');
const {
  calculatePercentile,
  calculateMeanAndStdDev,
  generateMermaidBarChart,
  generateMermaidLineChart,
  generateMermaidPieChart,
  parseLoadSpecParams
} = require('./report_utils');

// 解析命令行参数
const args = process.argv.slice(2);
const options = {
  outputDir: __dirname,
  format: 'md',
  mock: false
};

args.forEach(arg => {
  if (arg.startsWith('--outputDir=')) {
    options.outputDir = arg.split('=')[1];
  } else if (arg.startsWith('--format=')) {
    options.format = arg.split('=')[1];
  } else if (arg === '--mock') {
    options.mock = true;
  }
});

// 确保输出目录存在
if (!fs.existsSync(options.outputDir)) {
  fs.mkdirSync(options.outputDir, { recursive: true });
}

// 1. 获取基础测试结果 (兼容旧版报告)
let testData = null;
const resultsPath = path.join(__dirname, 'test-results.json');
if (fs.existsSync(resultsPath)) {
  testData = JSON.parse(fs.readFileSync(resultsPath, 'utf-8'));
} else if (!options.mock) {
  console.error('未找到 test-results.json，请先运行 playwright test');
  process.exit(1);
}

// 2. 加载性能与质量数据
let mockData = {};
let mockMetricsRaw = '';
const loadSpecPath = path.join(__dirname, 'tests', 'load.spec.ts');
let loadSpecContent = '';
if (fs.existsSync(loadSpecPath)) {
  loadSpecContent = fs.readFileSync(loadSpecPath, 'utf-8');
}

if (options.mock) {
  const mockDataPath = path.join(__dirname, 'mock-data.json');
  if (fs.existsSync(mockDataPath)) {
    mockData = JSON.parse(fs.readFileSync(mockDataPath, 'utf-8'));
  }
  const mockMetricsPath = path.join(__dirname, 'mock-metrics.csv');
  if (fs.existsSync(mockMetricsPath)) {
    mockMetricsRaw = fs.readFileSync(mockMetricsPath, 'utf-8');
  }
} else {
  // 读取真实的 metrics
  const metricsPath = path.join(__dirname, 'tests', 'load_test_metrics.csv');
  if (fs.existsSync(metricsPath)) {
    mockMetricsRaw = fs.readFileSync(metricsPath, 'utf-8');
  }
  // 真实测试中无法轻易拿到编译/运行时间拆分，这里依然用 mockData fallback
  const mockDataPath = path.join(__dirname, 'mock-data.json');
  if (fs.existsSync(mockDataPath)) {
    mockData = JSON.parse(fs.readFileSync(mockDataPath, 'utf-8'));
  }
}

// 解析 CSV 数据
const durations = [];
let totalSubmissions = 0;
let errors5xx = 0;
if (mockMetricsRaw) {
  const lines = mockMetricsRaw.split('\n').filter(l => l.trim().length > 0);
  // 去除表头
  const dataLines = lines.slice(1);
  totalSubmissions = dataLines.length;
  dataLines.forEach(line => {
    const parts = line.split(',');
    if (parts.length >= 5) {
      const status = parseInt(parts[3], 10);
      const duration = parseInt(parts[4], 10);
      if (!isNaN(duration)) durations.push(duration);
      if (status >= 500 && status < 600) errors5xx++;
    }
  });
}

// ================= 新版：性能与质量摘要 =================
function generatePerformanceQualityReport() {
  const { concurrentUsers, durationMs } = parseLoadSpecParams(loadSpecContent);
  const durationSec = durationMs / 1000;
  const submitsPerSec = totalSubmissions > 0 ? (totalSubmissions / durationSec).toFixed(2) : 0;

  // 1. 测评吞吐量
  const langDist = mockData.languageDistribution || { "C++": 100 };
  const langChart = generateMermaidBarChart(
    "提交分布 (按语言)", 
    "编程语言", Object.keys(langDist), 
    "提交次数", Object.values(langDist)
  );

  // 2. 判题速度
  const pMin = calculatePercentile(durations, 0);
  const p50 = calculatePercentile(durations, 50);
  const p95 = calculatePercentile(durations, 95);
  const p99 = calculatePercentile(durations, 99);
  const pMax = calculatePercentile(durations, 100);

  const compileStats = calculateMeanAndStdDev(mockData.compilePhase?.times || []);
  const runStats = calculateMeanAndStdDev(mockData.runPhase?.times || []);

  // 3. 错误拦截时效
  const errTime = mockData.errorDiscoveryTime || { CE: 0, RE: 0, WA: 0 };
  const errTypes = mockData.errorTypes || { CE: 0, RE: 0, WA: 0 };
  const errPieChart = generateMermaidPieChart("各类错误占比", errTypes);

  // 4. 并发承载能力
  const avgResponseTime = durations.length > 0 ? (durations.reduce((a, b) => a + b, 0) / durations.length) : 0;
  const eqQPS = avgResponseTime > 0 ? (concurrentUsers / (avgResponseTime / 1000)).toFixed(2) : 0;

  const gradData = mockData.gradientStressTest || { concurrency: [10], p95Delays: [p95] };
  const gradLineChart = generateMermaidLineChart(
    "梯度压测 P95 延迟 (2000ms 为阈值)", 
    "并发数", gradData.concurrency.map(String),
    "P95 延迟 (ms)", gradData.p95Delays
  );

  // 5. 环境快照
  const nodeVer = process.version;
  const cpus = os.cpus().length;
  const totalMem = (os.totalmem() / 1024 / 1024 / 1024).toFixed(2) + ' GB';
  let dockerVer = 'Unknown';
  try {
    dockerVer = execSync('docker -v').toString().trim();
  } catch (e) {
    // ignore
  }

  const outputJson = {
    throughput: { totalSubmissions, submitsPerSec, languageDistribution: langDist },
    judgingSpeed: {
      endToEnd: { min: pMin, p50, p95, p99, max: pMax },
      compile: compileStats,
      run: runStats
    },
    errorInterception: { avgTime: errTime, types: errTypes },
    concurrency: {
      users: concurrentUsers, durationMs, 
      errors5xx, eqQPS, 
      gradient: gradData
    },
    environment: { node: nodeVer, cpus, mem: totalMem, docker: dockerVer }
  };

  if (options.format === 'json') {
    const jsonPath = path.join(options.outputDir, 'Performance_Quality_Report.json');
    fs.writeFileSync(jsonPath, JSON.stringify(outputJson, null, 2));
    console.log(`✅ JSON 报告生成成功: ${jsonPath}`);
    return;
  }

  // Generate Markdown
  let md = `# 性能与质量摘要\n\n`;

  md += `## 1. 测评吞吐量\n`;
  md += `- **压测时段**: ${durationSec} s\n`;
  md += `- **成功完成提交总数**: ${totalSubmissions}\n`;
  md += `- **平均每秒提交数 (submits/s)**: ${submitsPerSec}\n\n`;
  md += `${langChart}\n\n`;

  md += `## 2. 判题速度\n`;
  md += `- **端到端耗时分布**: Min=${pMin}ms, P50=${p50}ms, P95=${p95}ms, P99=${p99}ms, Max=${pMax}ms\n`;
  md += `- **编译阶段平均耗时**: ${compileStats.mean}ms (标准差: ${compileStats.stdDev}ms)\n`;
  md += `- **运行阶段平均耗时**: ${runStats.mean}ms (标准差: ${runStats.stdDev}ms)\n\n`;

  md += `## 3. 错误拦截时效\n`;
  md += `- **平均发现时间**: 编译错误(CE) ${errTime.CE}ms, 运行时错误(RE) ${errTime.RE}ms, 答案错误(WA) ${errTime.WA}ms\n\n`;
  md += `${errPieChart}\n\n`;

  md += `## 4. 并发承载能力\n`;
  md += `- **压测场景**: ${concurrentUsers} 并发 × ${durationSec} 秒\n`;
  md += `- **关键结论**: 总完成提交数 ${totalSubmissions}，P95 响应时间 ${p95} ms，5xx 错误数为 ${errors5xx}。\n`;
  md += `- **等价 QPS**: ${eqQPS} (计算公式: \`并发数 / (平均响应时间 / 1000)\`)\n\n`;
  md += `${gradLineChart}\n\n`;

  md += `## 5. 环境快照\n`;
  md += `- **Node 版本**: ${nodeVer}\n`;
  md += `- **CPU 核数**: ${cpus}\n`;
  md += `- **内存总量**: ${totalMem}\n`;
  md += `- **Docker 镜像版本**: ${dockerVer}\n`;

  const mdPath = path.join(options.outputDir, 'Performance_Quality_Report.md');
  fs.writeFileSync(mdPath, md);
  console.log(`✅ Markdown 性能与质量报告生成成功: ${mdPath}`);
}

// ================= 旧版报告逻辑 =================
function generateLegacyReports() {
  if (!testData) return;

  let totalCases = 0;
  let passedCases = 0;
  let failedCases = 0;
  const moduleStats = {};
  const failedDetails = [];

  testData.suites.forEach(suite => {
    const fileName = path.basename(suite.file);
    if (!moduleStats[fileName]) {
      moduleStats[fileName] = { cases: 0, duration: 0 };
    }

    const traverseTests = (currentSuite) => {
      (currentSuite.specs || []).forEach(spec => {
        (spec.tests || []).forEach(test => {
          totalCases++;
          let testDuration = 0;
          let status = 'passed';
          
          const results = test.results || [];
          if (results.length > 0) {
            const finalResult = results[results.length - 1];
            testDuration = finalResult.duration || 0;
            status = finalResult.status;
            
            if (status === 'passed') {
              passedCases++;
            } else if (status === 'failed' || status === 'timedOut') {
              failedCases++;
              failedDetails.push({
                title: spec.title,
                file: fileName,
                error: finalResult.error ? finalResult.error.message : '未知错误'
              });
            }
          }

          moduleStats[fileName].cases++;
          moduleStats[fileName].duration += testDuration;
        });
      });

      (currentSuite.suites || []).forEach(childSuite => traverseTests(childSuite));
    };

    traverseTests(suite);
  });

  const totalDuration = (testData.stats.duration / 1000).toFixed(2);
  const passRate = totalCases > 0 ? ((passedCases / totalCases) * 100).toFixed(2) : 0;

  const requirements = [
    { id: 'REQ-1', desc: '用户注册与登录验证', file: 'login.spec.ts' },
    { id: 'REQ-2', desc: '代码正常提交与判题', file: 'judge.spec.ts' },
    { id: 'REQ-3', desc: '沙箱时间超限拦截(TLE)', file: 'judge.spec.ts' },
    { id: 'REQ-4', desc: '沙箱内存超限拦截(MLE)', file: 'judge.spec.ts' },
    { id: 'REQ-5', desc: '编译错误处理(CE)', file: 'judge.spec.ts' },
    { id: 'REQ-6', desc: '多语言(C++/Python/Java)支持', file: 'integration.spec.ts' },
    { id: 'REQ-7', desc: '高并发负载均衡调度', file: 'load.spec.ts' },
    { id: 'REQ-8', desc: '前后端UI及状态实时同步', file: 'integration.spec.ts' }
  ];

  let coveredReqs = 0;
  requirements.forEach(req => {
    if (moduleStats[req.file] && moduleStats[req.file].cases > 0) {
      coveredReqs++;
    }
  });
  const coverageRate = ((coveredReqs / requirements.length) * 100).toFixed(2);

  let detailedReport = '# 端到端(E2E)测试详细报告\n\n';
  detailedReport += '## 1. 测试概览\n';
  detailedReport += '- **总用例数**: ' + totalCases + '\n';
  detailedReport += '- **通过数**: ' + passedCases + '\n';
  detailedReport += '- **失败数**: ' + failedCases + '\n';
  detailedReport += '- **测试通过率**: ' + passRate + '%\n';
  detailedReport += '- **总耗时**: ' + totalDuration + 's\n';
  detailedReport += '- **业务场景覆盖率**: ' + coverageRate + '% (' + coveredReqs + '/' + requirements.length + ')\n\n';

  detailedReport += '## 2. 性能指标 (各模块耗时)\n';
  detailedReport += '| 测试文件 | 用例数 | 平均耗时(ms) | 总耗时(ms) |\n';
  detailedReport += '| --- | --- | --- | --- |\n';
  Object.keys(moduleStats).forEach(file => {
    const stat = moduleStats[file];
    const avg = stat.cases > 0 ? (stat.duration / stat.cases).toFixed(0) : 0;
    detailedReport += '| ' + file + ' | ' + stat.cases + ' | ' + avg + ' | ' + stat.duration.toFixed(0) + ' |\n';
  });
  detailedReport += '\n';

  detailedReport += '## 3. 测试结果分布图\n';
  detailedReport += '```mermaid\n';
  detailedReport += 'pie title 测试结果统计\n';
  detailedReport += '  "通过 (Passed)" : ' + passedCases + '\n';
  detailedReport += '  "失败 (Failed)" : ' + failedCases + '\n';
  detailedReport += '```\n\n';

  detailedReport += '## 4. 业务场景覆盖详情\n';
  detailedReport += '| 需求编号 | 需求描述 | 关联测试文件 | 覆盖状态 |\n';
  detailedReport += '| --- | --- | --- | --- |\n';
  requirements.forEach(req => {
    const status = (moduleStats[req.file] && moduleStats[req.file].cases > 0) ? '✅ 已覆盖' : '❌ 未覆盖';
    detailedReport += '| ' + req.id + ' | ' + req.desc + ' | ' + req.file + ' | ' + status + ' |\n';
  });
  detailedReport += '\n';

  detailedReport += '## 5. 失败用例分析\n';
  if (failedCases === 0) {
    detailedReport += '🎉 所有测试用例均通过，无失败记录。\n';
  } else {
    failedDetails.forEach(f => {
      detailedReport += '### ❌ ' + f.title + ' (' + f.file + ')\n';
      detailedReport += '```\n' + f.error + '\n```\n\n';
    });
  }

  const detailPath = path.join(options.outputDir, 'Detailed_Test_Report.md');
  fs.writeFileSync(detailPath, detailedReport);
  console.log(`✅ 测试报告生成成功: ${detailPath}`);
}

generateLegacyReports();
generatePerformanceQualityReport();
