export interface Contest {
  name: string
  start_time: string
  end_time: string
  status: 'upcoming' | 'running' | 'ended'
  link: string
  source: string // e.g., "Codeforces"
}

export interface ContestListResponse {
  status: number
  total: number
  data: Contest[]
}
