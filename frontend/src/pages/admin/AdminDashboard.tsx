import { useEffect, useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import { Button } from '@/components/ui/button'
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'
import { Badge } from '@/components/ui/badge'
import api from '@/lib/axios'
import { useToast } from '@/hooks/use-toast'
import { Plus, LogOut, Edit, Trash2 } from 'lucide-react'
import { useAuth } from '@/store/auth'

interface Question {
  number: string
  title: string
  star: string
  status: number
  cpu_limit: number
  mem_limit: number
}

interface AdminQuestionsResponse {
  status: number
  data: Question[]
}

export default function AdminDashboard() {
  const [questions, setQuestions] = useState<Question[]>([])
  const [isLoading, setIsLoading] = useState(true)
  const { logout } = useAuth()
  const navigate = useNavigate()
  const { toast } = useToast()

  const fetchQuestions = async () => {
    setIsLoading(true)
    try {
      const res = await api.get<AdminQuestionsResponse>('/admin/questions')
      if (res.data.status === 0) {
        setQuestions(res.data.data)
      }
    } catch (error) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: 'Failed to fetch questions',
      })
    } finally {
      setIsLoading(false)
    }
  }

  useEffect(() => {
    fetchQuestions()
  }, [])

  const handleDelete = async (id: string) => {
    if (!confirm('Are you sure you want to delete this question?')) return

    try {
      const res = await api.post(`/admin/question/delete/${id}`)
      if (res.data.status === 0) {
        toast({
          title: 'Success',
          description: 'Question deleted successfully',
        })
        fetchQuestions()
      } else {
        toast({
          variant: 'destructive',
          title: 'Error',
          description: res.data.reason || 'Failed to delete question',
        })
      }
    } catch (error) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: 'Failed to delete question',
      })
    }
  }

  const handleLogout = async () => {
    await logout()
    navigate('/admin/login')
  }

  return (
    <div className="min-h-screen bg-background">
      <header className="border-b bg-muted/40">
        <div className="container flex h-16 items-center justify-between py-4">
          <h1 className="text-xl font-bold">Admin Dashboard</h1>
          <div className="flex gap-2">
            <Link to="/admin/question/new">
              <Button>
                <Plus className="mr-2 h-4 w-4" /> Add Question
              </Button>
            </Link>
            <Button variant="destructive" onClick={handleLogout}>
              <LogOut className="mr-2 h-4 w-4" /> Logout
            </Button>
          </div>
        </div>
      </header>

      <main className="container py-8">
        <div className="rounded-md border bg-card">
          <Table>
            <TableHeader>
              <TableRow>
                <TableHead>ID</TableHead>
                <TableHead>Title</TableHead>
                <TableHead>Difficulty</TableHead>
                <TableHead>Status</TableHead>
                <TableHead>CPU Limit</TableHead>
                <TableHead>Mem Limit</TableHead>
                <TableHead className="text-right">Actions</TableHead>
              </TableRow>
            </TableHeader>
            <TableBody>
              {isLoading ? (
                <TableRow>
                  <TableCell colSpan={7} className="text-center h-24">
                    Loading...
                  </TableCell>
                </TableRow>
              ) : questions.length === 0 ? (
                <TableRow>
                  <TableCell colSpan={7} className="text-center h-24">
                    No questions found.
                  </TableCell>
                </TableRow>
              ) : (
                questions.map((q) => (
                  <TableRow key={q.number}>
                    <TableCell>{q.number}</TableCell>
                    <TableCell className="font-medium">{q.title}</TableCell>
                    <TableCell>
                      <Badge variant={
                        q.star === '简单' ? 'secondary' :
                        q.star === '中等' ? 'default' : 'destructive'
                      }>
                        {q.star}
                      </Badge>
                    </TableCell>
                    <TableCell>
                      <Badge variant={q.status === 1 ? 'outline' : 'secondary'} className={q.status === 1 ? 'text-green-500 border-green-500' : ''}>
                        {q.status === 1 ? 'Visible' : 'Hidden'}
                      </Badge>
                    </TableCell>
                    <TableCell>{q.cpu_limit}s</TableCell>
                    <TableCell>{q.mem_limit}KB</TableCell>
                    <TableCell className="text-right">
                      <div className="flex justify-end gap-2">
                        <Link to={`/admin/question/${q.number}`}>
                          <Button size="icon" variant="ghost">
                            <Edit className="h-4 w-4" />
                          </Button>
                        </Link>
                        <Button size="icon" variant="ghost" className="text-destructive" onClick={() => handleDelete(q.number)}>
                          <Trash2 className="h-4 w-4" />
                        </Button>
                      </div>
                    </TableCell>
                  </TableRow>
                ))
              )}
            </TableBody>
          </Table>
        </div>
      </main>
    </div>
  )
}
