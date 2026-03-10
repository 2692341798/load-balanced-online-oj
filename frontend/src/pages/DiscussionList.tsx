import { useEffect, useState } from 'react'
import { Link, useSearchParams } from 'react-router-dom'
import {
  Card,
  CardContent,
  CardFooter,
  CardHeader,
  CardTitle,
} from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { Input } from '@/components/ui/input'
import { Avatar, AvatarFallback, AvatarImage } from '@/components/ui/avatar'
import { Badge } from '@/components/ui/badge'
import api from '@/lib/axios'
import { type Discussion, type DiscussionListResponse } from '@/types/discussion'
import { MessageSquare, Plus, Search, ThumbsUp } from 'lucide-react'
import { useAuth } from '@/store/auth'
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from '@/components/ui/dialog'
import { Label } from '@/components/ui/label'
import { Textarea } from '@/components/ui/textarea' // Need to add textarea component
import { useToast } from '@/hooks/use-toast'

export default function DiscussionList() {
  const [discussions, setDiscussions] = useState<Discussion[]>([])
  const [isLoading, setIsLoading] = useState(true)
  const [searchParams, setSearchParams] = useSearchParams()
  const { isAuthenticated } = useAuth()
  const { toast } = useToast()
  
  // New Discussion Form State
  const [newTitle, setNewTitle] = useState('')
  const [newContent, setNewContent] = useState('')
  const [isCreating, setIsCreating] = useState(false)
  const [isDialogOpen, setIsDialogOpen] = useState(false)

  const search = searchParams.get('search') || ''

  useEffect(() => {
    const fetchDiscussions = async () => {
      setIsLoading(true)
      try {
        const questionId = searchParams.get('question_id')
        let response
        
        if (questionId) {
          response = await api.get<DiscussionListResponse>(`/discussions/question/${questionId}`)
        } else {
          // Construct query params
          const params = new URLSearchParams()
          if (search) params.append('search', search)
          response = await api.get<DiscussionListResponse>(`/discussions?${params.toString()}`)
        }

        setDiscussions(response.data.data || [])
      } catch (error) {
        console.error('Failed to fetch discussions:', error)
      } finally {
        setIsLoading(false)
      }
    }
    fetchDiscussions()
  }, [search, searchParams])

  const handleSearch = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault()
    const formData = new FormData(e.currentTarget)
    const newSearch = formData.get('search') as string
    setSearchParams({ search: newSearch })
  }

  const handleCreateDiscussion = async () => {
    if (!newTitle.trim() || !newContent.trim()) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: 'Title and content are required',
      })
      return
    }

    setIsCreating(true)
    try {
      const response = await api.post('/discussion', {
        title: newTitle,
        content: newContent,
      })

      if (response.data.status === 0) {
        toast({
          title: 'Success',
          description: 'Discussion created successfully',
        })
        setIsDialogOpen(false)
        setNewTitle('')
        setNewContent('')
        // Refresh list
        const listRes = await api.get<DiscussionListResponse>('/discussions')
        setDiscussions(listRes.data.data || [])
      } else {
        toast({
          variant: 'destructive',
          title: 'Error',
          description: response.data.reason || 'Failed to create discussion',
        })
      }
    } catch (error) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: 'Failed to create discussion',
      })
    } finally {
      setIsCreating(false)
    }
  }

  return (
    <div className="container py-10">
      <div className="mb-8 flex flex-col gap-4 md:flex-row md:items-center md:justify-between">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">社区讨论</h1>
          <p className="text-muted-foreground">分享观点，交流心得</p>
        </div>
        <div className="flex flex-col gap-2 sm:flex-row">
          <form onSubmit={handleSearch} className="flex gap-2">
            <Input
              name="search"
              placeholder="搜索讨论..."
              defaultValue={search}
              className="w-full sm:w-[200px]"
            />
            <Button type="submit" size="icon" variant="secondary">
              <Search className="h-4 w-4" />
            </Button>
          </form>
          
          {isAuthenticated && (
            <Dialog open={isDialogOpen} onOpenChange={setIsDialogOpen}>
              <DialogTrigger asChild>
                <Button>
                  <Plus className="mr-2 h-4 w-4" />
                  发布讨论
                </Button>
              </DialogTrigger>
              <DialogContent className="sm:max-w-[600px]">
                <DialogHeader>
                  <DialogTitle>发布新讨论</DialogTitle>
                  <DialogDescription>
                    分享你的算法解题思路或遇到的问题。
                  </DialogDescription>
                </DialogHeader>
                <div className="grid gap-4 py-4">
                  <div className="grid gap-2">
                    <Label htmlFor="title">标题</Label>
                    <Input
                      id="title"
                      value={newTitle}
                      onChange={(e) => setNewTitle(e.target.value)}
                      placeholder="请输入标题"
                    />
                  </div>
                  <div className="grid gap-2">
                    <Label htmlFor="content">内容 (Markdown)</Label>
                    <Textarea
                      id="content"
                      value={newContent}
                      onChange={(e) => setNewContent(e.target.value)}
                      placeholder="请输入内容，支持 Markdown 格式"
                      className="min-h-[200px]"
                    />
                  </div>
                </div>
                <DialogFooter>
                  <Button type="submit" onClick={handleCreateDiscussion} disabled={isCreating}>
                    {isCreating ? '发布中...' : '发布'}
                  </Button>
                </DialogFooter>
              </DialogContent>
            </Dialog>
          )}
        </div>
      </div>

      {isLoading ? (
        <div className="space-y-4">
          {[1, 2, 3].map((i) => (
            <div key={i} className="h-32 animate-pulse rounded-lg border bg-muted"></div>
          ))}
        </div>
      ) : discussions.length > 0 ? (
        <div className="grid gap-4">
          {discussions.map((discussion) => (
            <Card key={discussion.id} className="hover:bg-muted/50 transition-colors">
              <CardHeader className="pb-2">
                <div className="flex justify-between items-start">
                  <div className="space-y-1">
                    <CardTitle className="text-xl">
                      <Link to={`/discussion/${discussion.id}`} className="hover:underline">
                        {discussion.title}
                      </Link>
                    </CardTitle>
                    <div className="flex items-center gap-2 text-sm text-muted-foreground">
                      <Avatar className="h-6 w-6">
                        <AvatarImage src={discussion.avatar} />
                        <AvatarFallback>{discussion.author?.charAt(0).toUpperCase()}</AvatarFallback>
                      </Avatar>
                      <span>{discussion.author}</span>
                      <span>•</span>
                      <span>{discussion.date}</span>
                    </div>
                  </div>
                  {discussion.question_id && (
                    <Badge variant="outline">
                      Problem #{discussion.question_id}
                    </Badge>
                  )}
                </div>
              </CardHeader>
              <CardContent>
                <p className="line-clamp-2 text-muted-foreground text-sm">
                  {discussion.content ? discussion.content.replace(/[#*`]/g, '') : ''}
                </p>
              </CardContent>
              <CardFooter className="pt-0 text-sm text-muted-foreground gap-4">
                <div className="flex items-center gap-1">
                  <ThumbsUp className="h-4 w-4" />
                  <span>{discussion.likes || 0}</span>
                </div>
                <div className="flex items-center gap-1">
                  <MessageSquare className="h-4 w-4" />
                  <span>{discussion.comments || 0}</span>
                </div>
              </CardFooter>
            </Card>
          ))}
        </div>
      ) : (
        <div className="text-center py-20 text-muted-foreground">
          暂无讨论内容
        </div>
      )}
    </div>
  )
}
