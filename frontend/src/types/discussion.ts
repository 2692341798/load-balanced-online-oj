export interface Discussion {
  id: string
  title: string
  content: string // Markdown
  author: string
  avatar?: string
  date: string
  likes?: number
  views?: number
  comments?: number
  isOfficial?: boolean
  question_id?: string
  question_title?: string
}

export interface DiscussionComment {
  id: string
  post_id: string
  content: string
  user_id: string
  username: string
  avatar?: string
  created_at: string
  likes?: number
}

export interface InlineComment {
  id: string
  post_id: string
  content: string
  selected_text: string
  parent_id: string
  user_id: string
  username: string
  avatar?: string
  created_at: string
}

export interface DiscussionListResponse {
  status: number
  total: number
  data: Discussion[]
}

export interface DiscussionDetailResponse {
  status: number
  data: Discussion
}

export interface CommentListResponse {
  status: number
  data: DiscussionComment[]
}

export interface InlineCommentListResponse {
  status: number
  data: InlineComment[]
}

export interface CreateDiscussionRequest {
  title: string
  content: string
  question_id?: string
}

export interface CreateCommentRequest {
  post_id: string
  content: string
}
