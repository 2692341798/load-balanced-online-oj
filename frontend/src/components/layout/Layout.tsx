import { Navbar } from './Navbar'
import { Footer } from './Footer'
import { Toaster } from '@/components/ui/toaster'
import { Outlet } from 'react-router-dom'
import { useEffect } from 'react'
import { useAuth } from '@/store/auth'
import { Loading } from '@/components/ui/loading'

export function Layout() {
  const { checkAuth, isLoading } = useAuth()

  useEffect(() => {
    checkAuth()
  }, [checkAuth])

  if (isLoading) {
    return (
      <div className="flex h-screen items-center justify-center">
        <Loading size={48} />
      </div>
    )
  }

  return (
    <div className="relative flex min-h-screen flex-col">
      <Navbar />
      <main className="flex-1">
        <Outlet />
      </main>
      <Footer />
      <Toaster />
    </div>
  )
}
