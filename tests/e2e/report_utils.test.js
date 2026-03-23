const {
  calculatePercentile,
  calculateMeanAndStdDev,
  generateMermaidBarChart,
  generateMermaidLineChart,
  generateMermaidPieChart,
  parseLoadSpecParams
} = require('./report_utils');

describe('report_utils', () => {
  describe('calculatePercentile', () => {
    const data = [10, 20, 30, 40, 50];

    test('should return 0 for empty array', () => {
      expect(calculatePercentile([], 50)).toBe(0);
      expect(calculatePercentile(null, 50)).toBe(0);
    });

    test('should return max if upper >= sorted.length', () => {
      expect(calculatePercentile([10], 50)).toBe(10);
      expect(calculatePercentile([10, 20], 99.9)).toBe(20);
    });

    test('should calculate P0 (min)', () => {
      expect(calculatePercentile(data, 0)).toBe(10);
    });

    test('should calculate P100 (max)', () => {
      expect(calculatePercentile(data, 100)).toBe(50);
    });

    test('should calculate P50 (median)', () => {
      expect(calculatePercentile(data, 50)).toBe(30);
    });

    test('should trigger weight correctly', () => {
      // index for p=50 is 2. lower=2, upper=2, weight=0. 
      // let's test a case where lower != upper and weight is non-zero
      // p=25 in [10, 20, 30, 40, 50]
      // index = 0.25 * 4 = 1. lower=1, upper=1. Still integer.
      // p=10 in [10, 20, 30, 40, 50]
      // index = 0.10 * 4 = 0.4. lower=0, upper=1, weight=0.4
      // val = 10*(1-0.4) + 20*0.4 = 6 + 8 = 14
      expect(calculatePercentile(data, 10)).toBe(14);
    });

    test('should calculate P95', () => {
      // Index for P95: 0.95 * 4 = 3.8 -> 0.2*40 + 0.8*50 = 8 + 40 = 48
      expect(calculatePercentile(data, 95)).toBe(48);
    });

    test('should return max if p >= 100', () => {
      expect(calculatePercentile(data, 105)).toBe(50);
    });

    test('should calculate P99 (upper bound)', () => {
      expect(calculatePercentile(data, 99)).toBe(50);
      
      // Force upper >= sorted.length when p < 100 
      // Actually index = p/100 * (len-1) 
      // upper = ceil(index). If len=2, index = 0.99 * 1 = 0.99 -> upper = 1.
      // Not possible to get upper >= 2 unless index > 1.
      // So line 18 "if (upper >= sorted.length)" is technically a defensive check
      // against float precision errors, and we might not hit it strictly.
    });
  });

  describe('calculateMeanAndStdDev', () => {
    test('should handle empty array', () => {
      expect(calculateMeanAndStdDev([])).toEqual({ mean: 0, stdDev: 0 });
      expect(calculateMeanAndStdDev(null)).toEqual({ mean: 0, stdDev: 0 });
    });

    test('should calculate mean and stdDev correctly', () => {
      // [2, 4, 4, 4, 5, 5, 7, 9] mean: 5, stdDev: ~2
      const data = [2, 4, 4, 4, 5, 5, 7, 9];
      const result = calculateMeanAndStdDev(data);
      expect(result.mean).toBe(5);
      expect(result.stdDev).toBe(2);
    });
  });

  describe('Mermaid chart generators', () => {
    test('generateMermaidBarChart', () => {
      const result = generateMermaidBarChart('Test', 'X', ['A', 'B'], 'Y', [10, 20]);
      expect(result).toContain('title "Test"');
      expect(result).toContain('x-axis "X" ["A", "B"]');
      expect(result).toContain('y-axis "Y"');
      expect(result).toContain('bar [10, 20]');
    });

    test('generateMermaidLineChart', () => {
      const result = generateMermaidLineChart('Line Test', 'X', ['10', '20'], 'Y', [100, 200]);
      expect(result).toContain('title "Line Test"');
      expect(result).toContain('x-axis "X" ["10", "20"]');
      expect(result).toContain('line [100, 200]');
    });

    test('generateMermaidPieChart', () => {
      const result = generateMermaidPieChart('Pie Test', { 'A': 10, 'B': 20 });
      expect(result).toContain('pie title Pie Test');
      expect(result).toContain('"A" : 10');
      expect(result).toContain('"B" : 20');
    });
  });

  describe('parseLoadSpecParams', () => {
    test('should parse correctly from content', () => {
      const content = `
        const TARGET_URL = 'http://localhost';
        const CONCURRENT_USERS = 20; // test
        const DURATION_MS = 60000;
      `;
      const result = parseLoadSpecParams(content);
      expect(result.concurrentUsers).toBe(20);
      expect(result.durationMs).toBe(60000);
    });

    test('should fallback to defaults if not found', () => {
      const content = `const some_other_var = 10;`;
      const result = parseLoadSpecParams(content);
      expect(result.concurrentUsers).toBe(10);
      expect(result.durationMs).toBe(30000);
    });
    
    test('should fallback if partially found', () => {
      const content = `const CONCURRENT_USERS = 25;`;
      const result = parseLoadSpecParams(content);
      expect(result.concurrentUsers).toBe(25);
      expect(result.durationMs).toBe(30000);
      
      const content2 = `const DURATION_MS = 40000;`;
      const result2 = parseLoadSpecParams(content2);
      expect(result2.concurrentUsers).toBe(10);
      expect(result2.durationMs).toBe(40000);
    });
  });
});
