import { create } from 'zustand'
import { authService } from '../services/auth'
import type { User, LoginRequest, RegisterRequest } from '../types/user'

interface AuthState {
  user: User | null
  isAuthenticated: boolean
  isLoading: boolean
  checkAuth: () => Promise<void>
  login: (data: LoginRequest) => Promise<void>
  register: (data: RegisterRequest) => Promise<void>
  logout: () => Promise<void>
}

export const useAuth = create<AuthState>((set, get) => ({
  user: null,
  isAuthenticated: false,
  isLoading: true,
  checkAuth: async () => {
    try {
      set({ isLoading: true })
      const data = await authService.getCurrentUser()
      if (data.status === 0 && data.username) {
        set({
          user: {
            username: data.username,
            email: data.email || '',
            avatar: data.avatar,
            nickname: data.nickname,
          },
          isAuthenticated: true,
        })
      } else {
        set({ user: null, isAuthenticated: false })
      }
    } catch (error) {
      set({ user: null, isAuthenticated: false })
    } finally {
      set({ isLoading: false })
    }
  },
  login: async (data: LoginRequest) => {
    const res = await authService.login(data)
    if (res.status !== 0) {
      throw new Error(res.reason || 'Login failed')
    }
    await get().checkAuth()
  },
  register: async (data: RegisterRequest) => {
    const res = await authService.register(data)
    if (res.status !== 0) {
      throw new Error(res.reason || 'Registration failed')
    }
    // After register, user might need to login or is auto-logged in.
    // Based on API docs, register just returns success. User usually needs to login.
    // But let's check. Assuming user needs to login.
  },
  logout: async () => {
    try {
      await authService.logout()
      set({ user: null, isAuthenticated: false })
    } catch (error) {
      console.error('Logout failed', error)
    }
  },
}))
