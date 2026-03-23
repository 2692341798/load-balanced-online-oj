/**
 * 计算给定数组中特定百分位数的值
 * @param {number[]} arr - 数字数组
 * @param {number} p - 百分位 (0-100)
 * @returns {number} 对应百分位数的值
 */
function calculatePercentile(arr, p) {
  if (!arr || arr.length === 0) return 0;
  if (p <= 0) return Math.min(...arr);
  if (p >= 100) return Math.max(...arr);

  const sorted = [...arr].sort((a, b) => a - b);
  const index = (p / 100) * (sorted.length - 1);
  const lower = Math.floor(index);
  const upper = Math.ceil(index);
  const weight = index - lower;

  if (upper >= sorted.length) /* istanbul ignore next */ return sorted[lower];
  return Math.round(sorted[lower] * (1 - weight) + sorted[upper] * weight);
}

/**
 * 计算给定数组的平均值和标准差
 * @param {number[]} arr - 数字数组
 * @returns {{mean: number, stdDev: number}}
 */
function calculateMeanAndStdDev(arr) {
  if (!arr || arr.length === 0) return { mean: 0, stdDev: 0 };
  const mean = arr.reduce((sum, val) => sum + val, 0) / arr.length;
  const variance = arr.reduce((sum, val) => sum + Math.pow(val - mean, 2), 0) / arr.length;
  return {
    mean: Math.round(mean),
    stdDev: Math.round(Math.sqrt(variance))
  };
}

/**
 * 生成 Mermaid 柱状图 (xychart-beta)
 * @param {string} title 
 * @param {string} xAxisName 
 * @param {string[]} xData 
 * @param {string} yAxisName 
 * @param {number[]} yData 
 * @returns {string}
 */
function generateMermaidBarChart(title, xAxisName, xData, yAxisName, yData) {
  return `\`\`\`mermaid
xychart-beta
    title "${title}"
    x-axis "${xAxisName}" [${xData.map(d => `"${d}"`).join(', ')}]
    y-axis "${yAxisName}"
    bar [${yData.join(', ')}]
\`\`\``;
}

/**
 * 生成 Mermaid 折线图 (xychart-beta)
 * @param {string} title 
 * @param {string} xAxisName 
 * @param {string[]} xData 
 * @param {string} yAxisName 
 * @param {number[]} yData 
 * @returns {string}
 */
function generateMermaidLineChart(title, xAxisName, xData, yAxisName, yData) {
  return `\`\`\`mermaid
xychart-beta
    title "${title}"
    x-axis "${xAxisName}" [${xData.map(d => `"${d}"`).join(', ')}]
    y-axis "${yAxisName}"
    line [${yData.join(', ')}]
\`\`\``;
}

/**
 * 生成 Mermaid 饼图
 * @param {string} title 
 * @param {Object} data - { label: value }
 * @returns {string}
 */
function generateMermaidPieChart(title, data) {
  let pie = `\`\`\`mermaid\npie title ${title}\n`;
  for (const [key, val] of Object.entries(data)) {
    pie += `    "${key}" : ${val}\n`;
  }
  pie += `\`\`\``;
  return pie;
}

/**
 * 从 load.spec.ts 文件内容中解析参数
 * @param {string} fileContent 
 * @returns {{concurrentUsers: number, durationMs: number}}
 */
function parseLoadSpecParams(fileContent) {
  const usersMatch = fileContent.match(/const\s+CONCURRENT_USERS\s*=\s*(\d+)/);
  const durationMatch = fileContent.match(/const\s+DURATION_MS\s*=\s*(\d+)/);

  return {
    concurrentUsers: usersMatch ? parseInt(usersMatch[1], 10) : 10,
    durationMs: durationMatch ? parseInt(durationMatch[1], 10) : 30000
  };
}

module.exports = {
  calculatePercentile,
  calculateMeanAndStdDev,
  generateMermaidBarChart,
  generateMermaidLineChart,
  generateMermaidPieChart,
  parseLoadSpecParams
};
