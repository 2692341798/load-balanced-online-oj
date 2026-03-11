import { Navigate, Outlet, useLocation } from 'react-router-dom'
import { useEffect, useState } from 'react'
import { useAuth } from '@/store/auth'
import { Loading } from '@/components/ui/loading'
import api from '@/lib/axios'
import type { ProfileResponse } from '@/types/user'

export const AdminRoute = () => {
  const { isAuthenticated, isLoading: authLoading } = useAuth()
  const [isAdmin, setIsAdmin] = useState<boolean | null>(null)
  const [checkingRole, setCheckingRole] = useState(false)
  const location = useLocation()

  useEffect(() => {
    const checkAdminRole = async () => {
      if (isAuthenticated) {
        setCheckingRole(true)
        try {
          const res = await api.get<ProfileResponse>('/profile')
          if (res.data.status === 0 && res.data.role === 1) {
            setIsAdmin(true)
          } else {
            setIsAdmin(false)
          }
        } catch (error) {
          setIsAdmin(false)
        } finally {
          setCheckingRole(false)
        }
      }
    }

    if (isAuthenticated) {
      checkAdminRole()
    }
  }, [isAuthenticated])

  if (authLoading || (isAuthenticated && checkingRole && isAdmin === null)) {
    return (
      <div className="flex h-[calc(100vh-4rem)] items-center justify-center">
        <Loading />
      </div>
    )
  }

  if (!isAuthenticated) {
    return <Navigate to="/admin/login" state={{ from: location }} replace />
  }

  if (isAdmin === false) {
    return <div className="container py-10 text-center text-destructive">Access Denied: You are not an administrator.</div>
  }

  return <Outlet />
}
