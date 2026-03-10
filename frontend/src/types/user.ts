export interface User {
  username: string
  email: string
  nickname?: string
  avatar?: string
  role?: number // 0: User, 1: Admin
}

export interface AuthResponse {
  status: number
  reason?: string
}

export interface UserResponse extends AuthResponse {
  username?: string
  email?: string
  avatar?: string
  nickname?: string
}

export interface ProfileResponse extends AuthResponse {
  username: string
  email: string
  nickname?: string
  phone?: string
  avatar?: string
  role: number
  created_at: string
  stats: {
    [key: string]: number // "简单": 10, etc.
  }
}

export interface LoginRequest {
  username: string
  password: string
}

export interface RegisterRequest {
  username: string
  password: string
  email: string
  nickname?: string
  phone?: string
}
