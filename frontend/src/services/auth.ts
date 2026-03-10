import api from '../lib/axios'
import { type UserResponse, type LoginRequest, type RegisterRequest, type AuthResponse } from '../types/user'

export const authService = {
  async getCurrentUser(): Promise<UserResponse> {
    const response = await api.get<UserResponse>('/user')
    return response.data
  },

  async login(data: LoginRequest): Promise<AuthResponse> {
    const response = await api.post<AuthResponse>('/login', data)
    return response.data
  },

  async register(data: RegisterRequest): Promise<AuthResponse> {
    const response = await api.post<AuthResponse>('/register', data)
    return response.data
  },

  async logout(): Promise<AuthResponse> {
    const response = await api.get<AuthResponse>('/logout')
    return response.data
  },
}
