export interface Problem {
  number: string
  title: string
  star: string // Difficulty: "简单" | "中等" | "困难"
  cpu_limit?: number
  mem_limit?: number
  status?: string // "Solved" or empty
}

export interface ProblemDetail extends Problem {
  desc: string
  header: string // For compatibility if exists
  tail: string // Test cases (JSON string) - usually hidden from user?
}

export interface ProblemListResponse {
  status: number
  total: number
  page: number
  data: Problem[]
}

export interface ProblemDetailResponse {
  status: number
  data: ProblemDetail
}
