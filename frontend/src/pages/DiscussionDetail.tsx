import { useEffect, useState } from 'react'
import { useParams } from 'react-router-dom'
import ReactMarkdown from 'react-markdown'
import rehypeHighlight from 'rehype-highlight'
import remarkGfm from 'remark-gfm'
import { Avatar, AvatarFallback, AvatarImage } from '@/components/ui/avatar'
import { Button } from '@/components/ui/button'
import { Card, CardHeader } from '@/components/ui/card'
import { Textarea } from '@/components/ui/textarea'
import { Loading } from '@/components/ui/loading'
import { Separator } from '@/components/ui/separator'
import api from '@/lib/axios'
import { 
  type Discussion, 
  type DiscussionComment, 
  type InlineComment,
  type DiscussionDetailResponse, 
  type CommentListResponse,
  type InlineCommentListResponse
} from '@/types/discussion'
import { useAuth } from '@/store/auth'
import { useToast } from '@/hooks/use-toast'
import { MessageSquare, ThumbsUp, Quote } from 'lucide-react'

export default function DiscussionDetail() {
  const { id } = useParams<{ id: string }>()
  const [discussion, setDiscussion] = useState<Discussion | null>(null)
  const [comments, setComments] = useState<DiscussionComment[]>([])
  const [inlineComments, setInlineComments] = useState<InlineComment[]>([])
  const [isLoading, setIsLoading] = useState(true)
  const [newComment, setNewComment] = useState('')
  const [isSubmitting, setIsSubmitting] = useState(false)
  
  const { isAuthenticated } = useAuth()
  const { toast } = useToast()

  useEffect(() => {
    const fetchData = async () => {
      if (!id) return
      setIsLoading(true)
      try {
        const [discussionRes, commentsRes, inlineCommentsRes] = await Promise.all([
          api.get<DiscussionDetailResponse>(`/discussion/${id}`),
          api.get<CommentListResponse>(`/article_comments/${id}`),
          api.get<InlineCommentListResponse>(`/inline_comments/${id}`),
        ])
        setDiscussion(discussionRes.data.data)
        setComments(commentsRes.data.data || [])
        setInlineComments(inlineCommentsRes.data.data || [])
      } catch (error) {
        console.error('Failed to fetch discussion data:', error)
        toast({
          variant: 'destructive',
          title: 'Error',
          description: 'Failed to load discussion',
        })
      } finally {
        setIsLoading(false)
      }
    }
    fetchData()
  }, [id, toast])

  const handleSubmitComment = async () => {
    if (!id || !newComment.trim()) return
    setIsSubmitting(true)
    try {
      const response = await api.post('/article_comment/add', {
        post_id: id,
        content: newComment,
      })

      if (response.data.status === 0) {
        toast({
          title: 'Success',
          description: 'Comment posted',
        })
        setNewComment('')
        // Refresh comments
        const commentsRes = await api.get<CommentListResponse>(`/article_comments/${id}`)
        setComments(commentsRes.data.data || [])
      } else {
        toast({
          variant: 'destructive',
          title: 'Error',
          description: response.data.reason || 'Failed to post comment',
        })
      }
    } catch (error) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: 'Failed to post comment',
      })
    } finally {
      setIsSubmitting(false)
    }
  }

  if (isLoading) {
    return (
      <div className="flex h-[50vh] items-center justify-center">
        <Loading />
      </div>
    )
  }

  if (!discussion) {
    return <div className="container py-10 text-center">Discussion not found</div>
  }

  return (
    <div className="container max-w-4xl py-10">
      <div className="mb-6 space-y-4">
        <h1 className="text-3xl font-bold">{discussion.title}</h1>
        <div className="flex items-center gap-4 text-sm text-muted-foreground">
          <div className="flex items-center gap-2">
            <Avatar className="h-8 w-8">
              <AvatarImage src={discussion.avatar} />
              <AvatarFallback>{discussion.author?.charAt(0).toUpperCase()}</AvatarFallback>
            </Avatar>
            <span className="font-medium text-foreground">{discussion.author}</span>
          </div>
          <span>Published on {discussion.date}</span>
          <div className="flex items-center gap-1">
             <ThumbsUp className="h-4 w-4" />
             <span>{discussion.likes || 0}</span>
          </div>
        </div>
      </div>

      <div className="prose prose-neutral dark:prose-invert max-w-none mb-10">
        <ReactMarkdown
          remarkPlugins={[remarkGfm]}
          rehypePlugins={[rehypeHighlight]}
        >
          {discussion.content}
        </ReactMarkdown>
      </div>

      {inlineComments.length > 0 && (
        <>
          <Separator className="my-8" />
          <div className="space-y-6">
            <h3 className="text-xl font-semibold flex items-center gap-2">
              <Quote className="h-5 w-5" />
              Inline Comments ({inlineComments.length})
            </h3>
            <div className="grid gap-4">
              {inlineComments.map((comment) => (
                <Card key={comment.id} className="bg-muted/30">
                  <CardHeader className="p-4 space-y-3">
                    <div className="text-sm italic text-muted-foreground border-l-2 pl-3 py-1 bg-muted/50 rounded-r">
                      "{comment.selected_text}"
                    </div>
                    <div className="flex flex-row items-start gap-3">
                      <Avatar className="h-6 w-6 mt-0.5">
                        <AvatarImage src={comment.avatar} />
                        <AvatarFallback>{comment.username?.charAt(0).toUpperCase()}</AvatarFallback>
                      </Avatar>
                      <div className="flex-1 space-y-1">
                        <div className="flex items-center justify-between">
                          <span className="font-medium text-sm">{comment.username}</span>
                          <span className="text-xs text-muted-foreground">
                            {new Date(comment.created_at).toLocaleString()}
                          </span>
                        </div>
                        <p className="text-sm text-foreground/90">{comment.content}</p>
                      </div>
                    </div>
                  </CardHeader>
                </Card>
              ))}
            </div>
          </div>
        </>
      )}

      <Separator className="my-8" />

      <div className="space-y-8">
        <h3 className="text-xl font-semibold flex items-center gap-2">
          <MessageSquare className="h-5 w-5" />
          Article Comments ({comments.length})
        </h3>

        {/* Comment Form */}
        {isAuthenticated ? (
          <div className="space-y-4">
            <Textarea
              placeholder="Write a comment..."
              value={newComment}
              onChange={(e) => setNewComment(e.target.value)}
              className="min-h-[100px]"
            />
            <div className="flex justify-end">
              <Button onClick={handleSubmitComment} disabled={isSubmitting || !newComment.trim()}>
                {isSubmitting ? 'Posting...' : 'Post Comment'}
              </Button>
            </div>
          </div>
        ) : (
          <div className="rounded-md bg-muted p-4 text-center">
            Please <Button variant="link" className="px-1 h-auto" asChild><a href="/login">login</a></Button> to post comments.
          </div>
        )}

        {/* Comment List */}
        <div className="space-y-4">
          {comments.length > 0 ? (
            comments.map((comment) => (
              <Card key={comment.id}>
                <CardHeader className="p-4 flex flex-row items-start gap-4 space-y-0">
                  <Avatar className="h-8 w-8 mt-1">
                    <AvatarImage src={comment.avatar} />
                    <AvatarFallback>{comment.username?.charAt(0).toUpperCase()}</AvatarFallback>
                  </Avatar>
                  <div className="flex-1 space-y-1">
                    <div className="flex items-center justify-between">
                      <span className="font-semibold text-sm">{comment.username}</span>
                      <span className="text-xs text-muted-foreground">
                        {new Date(comment.created_at).toLocaleString()}
                      </span>
                    </div>
                    <div className="text-sm text-foreground/90 whitespace-pre-wrap">
                      {comment.content}
                    </div>
                  </div>
                </CardHeader>
              </Card>
            ))
          ) : (
            <div className="text-center text-muted-foreground py-8">
              No comments yet. Be the first to comment!
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
