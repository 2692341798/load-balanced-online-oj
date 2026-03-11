import { useEffect, useState } from 'react'
import { useNavigate, useParams } from 'react-router-dom'
import { Button } from '@/components/ui/button'
import { Input } from '@/components/ui/input'
import { Label } from '@/components/ui/label'

import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { useToast } from '@/hooks/use-toast'
import api from '@/lib/axios'
import Editor from '@monaco-editor/react'

interface QuestionForm {
  title: string
  star: string
  status: string
  cpu_limit: number
  mem_limit: number
  description: string
  tail: string // JSON test cases
}

export default function AdminQuestionEditor() {
  const { id } = useParams()
  const navigate = useNavigate()
  const { toast } = useToast()
  const [isLoading, setIsLoading] = useState(false)
  const [formData, setFormData] = useState<QuestionForm>({
    title: '',
    star: '简单',
    status: '1',
    cpu_limit: 1,
    mem_limit: 30000,
    description: '',
    tail: '[\n  { "input": "1 2", "expect": "3" }\n]'
  })

  useEffect(() => {
    if (id) {
      fetchQuestion(id)
    }
  }, [id])

  const fetchQuestion = async (questionId: string) => {
    setIsLoading(true)
    try {
      // Since we don't have a direct get-one-admin API, we fetch all and find
      // Optimized approach would be adding GET /api/admin/question/{id}
      const res = await api.get('/admin/questions')
      if (res.data.status === 0) {
        const question = res.data.data.find((q: any) => q.number == questionId)
        if (question) {
          setFormData({
            title: question.title,
            star: question.star,
            status: question.status.toString(),
            cpu_limit: question.cpu_limit,
            mem_limit: question.mem_limit,
            description: question.description,
            tail: question.tail
          })
        }
      }
    } catch (error) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: 'Failed to load question',
      })
    } finally {
      setIsLoading(false)
    }
  }

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setIsLoading(true)

    try {
      // Validate JSON
      try {
        JSON.parse(formData.tail)
      } catch (e) {
        throw new Error('Invalid JSON in Test Cases')
      }

      const payload = {
        ...formData,
        status: parseInt(formData.status),
        cpu_limit: parseInt(formData.cpu_limit.toString()),
        mem_limit: parseInt(formData.mem_limit.toString())
      }

      const url = id ? `/admin/question/update/${id}` : '/admin/question'
      const res = await api.post(url, payload)

      if (res.data.status === 0) {
        toast({
          title: 'Success',
          description: 'Question saved successfully',
        })
        navigate('/admin/dashboard')
      } else {
        throw new Error(res.data.reason)
      }
    } catch (error: any) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: error.message || 'Failed to save question',
      })
    } finally {
      setIsLoading(false)
    }
  }

  return (
    <div className="container py-8 max-w-4xl">
      <div className="flex items-center justify-between mb-6">
        <h1 className="text-2xl font-bold">{id ? 'Edit Question' : 'New Question'}</h1>
        <Button variant="outline" onClick={() => navigate('/admin/dashboard')}>
          Cancel
        </Button>
      </div>

      <Card>
        <CardHeader>
          <CardTitle>Question Details</CardTitle>
        </CardHeader>
        <CardContent>
          <form onSubmit={handleSubmit} className="space-y-6">
            <div className="grid gap-4 md:grid-cols-2">
              <div className="space-y-2">
                <Label htmlFor="title">Title</Label>
                <Input
                  id="title"
                  value={formData.title}
                  onChange={(e) => setFormData({ ...formData, title: e.target.value })}
                  required
                />
              </div>
              <div className="space-y-2">
                <Label htmlFor="star">Difficulty</Label>
                <Select
                  value={formData.star}
                  onValueChange={(val) => setFormData({ ...formData, star: val })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="简单">Easy</SelectItem>
                    <SelectItem value="中等">Medium</SelectItem>
                    <SelectItem value="困难">Hard</SelectItem>
                  </SelectContent>
                </Select>
              </div>
            </div>

            <div className="grid gap-4 md:grid-cols-3">
              <div className="space-y-2">
                <Label htmlFor="status">Status</Label>
                <Select
                  value={formData.status}
                  onValueChange={(val) => setFormData({ ...formData, status: val })}
                >
                  <SelectTrigger>
                    <SelectValue />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="1">Visible</SelectItem>
                    <SelectItem value="0">Hidden</SelectItem>
                  </SelectContent>
                </Select>
              </div>
              <div className="space-y-2">
                <Label htmlFor="cpu">CPU Limit (s)</Label>
                <Input
                  id="cpu"
                  type="number"
                  value={formData.cpu_limit}
                  onChange={(e) => setFormData({ ...formData, cpu_limit: parseInt(e.target.value) })}
                  required
                />
              </div>
              <div className="space-y-2">
                <Label htmlFor="mem">Memory Limit (KB)</Label>
                <Input
                  id="mem"
                  type="number"
                  value={formData.mem_limit}
                  onChange={(e) => setFormData({ ...formData, mem_limit: parseInt(e.target.value) })}
                  required
                />
              </div>
            </div>

            <div className="space-y-2">
              <Label htmlFor="description">Description (Markdown)</Label>
              <div className="h-[400px] border rounded-md overflow-hidden">
                <Editor
                  height="100%"
                  defaultLanguage="markdown"
                  theme="vs-dark"
                  value={formData.description}
                  onChange={(val) => setFormData({ ...formData, description: val || '' })}
                  options={{ minimap: { enabled: false } }}
                />
              </div>
            </div>

            <div className="space-y-2">
              <Label htmlFor="tail">Test Cases (JSON)</Label>
              <div className="h-[200px] border rounded-md overflow-hidden">
                <Editor
                  height="100%"
                  defaultLanguage="json"
                  theme="vs-dark"
                  value={formData.tail}
                  onChange={(val) => setFormData({ ...formData, tail: val || '' })}
                  options={{ minimap: { enabled: false } }}
                />
              </div>
              <p className="text-sm text-muted-foreground">
                Format: <code>[{`{"input": "...", "expect": "..."}`}]</code>
              </p>
            </div>

            <div className="flex justify-end gap-4">
              <Button type="submit" disabled={isLoading}>
                {isLoading ? 'Saving...' : 'Save Question'}
              </Button>
            </div>
          </form>
        </CardContent>
      </Card>
    </div>
  )
}
