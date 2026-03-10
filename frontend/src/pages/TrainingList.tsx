import { useEffect, useState } from 'react'
import { Link, useSearchParams } from 'react-router-dom'
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import api from '@/lib/axios'
import { type TrainingList, type TrainingListResponse } from '@/types/training'
import { BookOpen, Star, User } from 'lucide-react'

export default function TrainingList() {
  const [trainings, setTrainings] = useState<TrainingList[]>([])
  const [isLoading, setIsLoading] = useState(true)
  const [searchParams] = useSearchParams()

  const page = parseInt(searchParams.get('page') || '1')

  useEffect(() => {
    const fetchTrainings = async () => {
      setIsLoading(true)
      try {
        const response = await api.get<TrainingListResponse>(`/training/list?page=${page}`)
        setTrainings(response.data.data || [])
      } catch (error) {
        console.error('Failed to fetch training lists:', error)
      } finally {
        setIsLoading(false)
      }
    }
    fetchTrainings()
  }, [page])

  return (
    <div className="container py-10">
      <div className="mb-8">
        <h1 className="text-3xl font-bold tracking-tight">题单广场</h1>
        <p className="text-muted-foreground">精选算法题单，系统化训练</p>
      </div>

      {isLoading ? (
        <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
          {[1, 2, 3, 4, 5, 6].map((i) => (
            <div key={i} className="h-48 animate-pulse rounded-lg border bg-muted"></div>
          ))}
        </div>
      ) : trainings.length > 0 ? (
        <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
          {trainings.map((training) => (
            <Link key={training.id} to={`/training/${training.id}`} className="group">
              <Card className="h-full transition-all hover:shadow-md group-hover:border-primary/50">
                <CardHeader>
                  <div className="flex justify-between items-start">
                    <CardTitle className="line-clamp-1 group-hover:text-primary">
                      {training.title}
                    </CardTitle>
                    <Badge variant={
                      training.difficulty === '简单' ? 'secondary' :
                      training.difficulty === '中等' ? 'default' : 'destructive'
                    }>
                      {training.difficulty}
                    </Badge>
                  </div>
                  <CardDescription className="flex items-center gap-1">
                    <User className="h-3 w-3" />
                    {training.author_name}
                  </CardDescription>
                </CardHeader>
                <CardContent>
                  <div className="flex items-center justify-between text-sm text-muted-foreground">
                    <div className="flex items-center gap-1">
                      <BookOpen className="h-4 w-4" />
                      <span>{training.problem_count} 题</span>
                    </div>
                    <div className="flex items-center gap-1">
                      <Star className="h-4 w-4" />
                      <span>{training.collections} 收藏</span>
                    </div>
                  </div>
                </CardContent>
              </Card>
            </Link>
          ))}
        </div>
      ) : (
        <div className="text-center py-20 text-muted-foreground">
          暂无题单
        </div>
      )}
    </div>
  )
}
