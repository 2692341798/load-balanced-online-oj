import { Routes, Route } from 'react-router-dom'
import { Layout } from '@/components/layout/Layout'
import { ProtectedRoute } from '@/components/layout/ProtectedRoute'
import { AdminRoute } from '@/components/layout/AdminRoute'
import { Suspense, lazy } from 'react'
import { Loading } from '@/components/ui/loading'

// Eager load critical pages
import Home from '@/pages/Home'
import Login from '@/pages/Login'
import Register from '@/pages/Register'

// Lazy load other pages
const ProblemList = lazy(() => import('@/pages/ProblemList'))
const Profile = lazy(() => import('@/pages/Profile'))
const ProblemDetail = lazy(() => import('@/pages/ProblemDetail'))
const DiscussionList = lazy(() => import('@/pages/DiscussionList'))
const DiscussionDetail = lazy(() => import('@/pages/DiscussionDetail'))
const TrainingList = lazy(() => import('@/pages/TrainingList'))
const TrainingDetail = lazy(() => import('@/pages/TrainingDetail'))
const ContestList = lazy(() => import('@/pages/ContestList'))
const Games = lazy(() => import('@/pages/Games'))

// Admin pages
const AdminLogin = lazy(() => import('@/pages/admin/AdminLogin'))
const AdminDashboard = lazy(() => import('@/pages/admin/AdminDashboard'))
const AdminQuestionEditor = lazy(() => import('@/pages/admin/AdminQuestionEditor'))

function App() {
  return (
    <div className="min-h-screen bg-background text-foreground font-sans antialiased">
      <Suspense fallback={<div className="flex h-screen items-center justify-center"><Loading /></div>}>
        <Routes>
          <Route element={<Layout />}>
            <Route path="/" element={<Home />} />
            <Route path="/login" element={<Login />} />
            <Route path="/register" element={<Register />} />
            
            {/* Protected Routes */}
            <Route element={<ProtectedRoute />}>
              <Route path="/problems" element={<ProblemList />} />
              <Route path="/problem/:id" element={<ProblemDetail />} />
              <Route path="/training" element={<TrainingList />} />
              <Route path="/training/:id" element={<TrainingDetail />} />
              <Route path="/contest" element={<ContestList />} />
              <Route path="/discussion" element={<DiscussionList />} />
              <Route path="/discussion/:id" element={<DiscussionDetail />} />
              <Route path="/profile" element={<Profile />} />
              <Route path="/games" element={<Games />} />
            </Route>
          </Route>

          {/* Admin Routes (No Main Layout) */}
          <Route path="/admin/login" element={<AdminLogin />} />
          <Route element={<AdminRoute />}>
            <Route path="/admin" element={<AdminDashboard />} />
            <Route path="/admin/dashboard" element={<AdminDashboard />} />
            <Route path="/admin/index.html" element={<AdminDashboard />} /> {/* Legacy Redirect */}
            <Route path="/admin/question/new" element={<AdminQuestionEditor />} />
            <Route path="/admin/question/:id" element={<AdminQuestionEditor />} />
          </Route>
        </Routes>
      </Suspense>
    </div>
  )
}

export default App
