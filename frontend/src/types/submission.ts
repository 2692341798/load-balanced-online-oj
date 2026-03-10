export interface Submission {
  id: string
  user_id: string
  question_id: string
  question_title: string
  result: string // "0" means success
  cpu_time: number
  mem_usage: number
  created_at: string
  content?: string
  language: string
}

export interface SubmissionListResponse {
  status: number
  total: number
  page: number
  page_size: number
  data: Submission[]
}
