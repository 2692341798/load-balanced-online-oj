export interface TrainingProblem {
  id: string
  title: string
  difficulty: string
  status?: string
}

export interface TrainingList {
  id: string
  title: string
  description: string
  difficulty: string
  author_name: string
  author_avatar?: string
  problem_count: number
  likes: number
  collections: number
  created_at: string
}

export interface TrainingDetail extends TrainingList {
  description: string
  tags: string // JSON string
  author_id: string
  visibility: string
  problems: TrainingProblem[]
}

export interface TrainingListResponse {
  status: number
  total: number
  data: TrainingList[]
}

export interface TrainingDetailResponse {
  status: number
  data: TrainingDetail
}
