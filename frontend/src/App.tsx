import { Routes, Route } from 'react-router-dom'
import { Layout } from '@/components/layout/Layout'
import { ProtectedRoute } from '@/components/layout/ProtectedRoute'
import Home from '@/pages/Home'
import Login from '@/pages/Login'
import Register from '@/pages/Register'
import ProblemList from '@/pages/ProblemList'
import Profile from '@/pages/Profile'
import ProblemDetail from '@/pages/ProblemDetail'
import DiscussionList from '@/pages/DiscussionList'
import DiscussionDetail from '@/pages/DiscussionDetail'
import TrainingList from '@/pages/TrainingList'
import TrainingDetail from '@/pages/TrainingDetail'
import ContestList from '@/pages/ContestList'
import Games from '@/pages/Games'

function App() {
  return (
    <div className="min-h-screen bg-background text-foreground font-sans antialiased">
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
      </Routes>
    </div>
  )
}

export default App
