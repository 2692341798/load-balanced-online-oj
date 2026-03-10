import { useEffect, useState } from 'react'
import { useParams, Link } from 'react-router-dom'
import {
  Card,
  CardContent,
  CardHeader,
  CardTitle,
} from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import { Loading } from '@/components/ui/loading'
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'
import { Button } from '@/components/ui/button'
import api from '@/lib/axios'
import { type TrainingDetail, type TrainingDetailResponse } from '@/types/training'
import { CheckCircle2, Circle } from 'lucide-react'

export default function TrainingDetail() {
  const { id } = useParams<{ id: string }>()
  const [training, setTraining] = useState<TrainingDetail | null>(null)
  const [isLoading, setIsLoading] = useState(true)

  useEffect(() => {
    const fetchDetail = async () => {
      if (!id) return
      setIsLoading(true)
      try {
        const response = await api.get<TrainingDetailResponse>(`/training/${id}`)
        setTraining(response.data.data)
      } catch (error) {
        console.error('Failed to fetch training detail:', error)
      } finally {
        setIsLoading(false)
      }
    }
    fetchDetail()
  }, [id])

  if (isLoading) {
    return (
      <div className="flex h-[50vh] items-center justify-center">
        <Loading />
      </div>
    )
  }

  if (!training) {
    return <div className="container py-10 text-center">Training list not found</div>
  }

  return (
    <div className="container py-10">
      <div className="mb-8 space-y-4">
        <div className="flex items-center gap-4">
          <h1 className="text-3xl font-bold">{training.title}</h1>
          <Badge variant={
            training.difficulty === '简单' ? 'secondary' :
            training.difficulty === '中等' ? 'default' : 'destructive'
          }>
            {training.difficulty}
          </Badge>
        </div>
        <p className="text-muted-foreground max-w-2xl">{training.description}</p>
        
        <div className="flex gap-2">
          {(() => {
            try {
              const tags = JSON.parse(training.tags)
              return Array.isArray(tags) ? tags.map((tag: string) => (
                <Badge key={tag} variant="outline">{tag}</Badge>
              )) : null
            } catch (e) {
              return null
            }
          })()}
        </div>
      </div>

      <Card>
        <CardHeader>
          <CardTitle>题目列表 ({training.problems.length})</CardTitle>
        </CardHeader>
        <CardContent>
          <Table>
            <TableHeader>
              <TableRow>
                <TableHead className="w-[50px]"></TableHead>
                <TableHead>标题</TableHead>
                <TableHead>难度</TableHead>
                <TableHead className="text-right">操作</TableHead>
              </TableRow>
            </TableHeader>
            <TableBody>
              {training.problems.map((problem) => (
                <TableRow key={problem.id}>
                  <TableCell>
                    {problem.status === 'Solved' ? (
                      <CheckCircle2 className="h-4 w-4 text-green-500" />
                    ) : (
                      <Circle className="h-4 w-4 text-muted-foreground" />
                    )}
                  </TableCell>
                  <TableCell className="font-medium">
                    {problem.id}. {problem.title}
                  </TableCell>
                  <TableCell>
                    <Badge variant={
                        problem.difficulty === '简单' ? 'secondary' :
                        problem.difficulty === '中等' ? 'default' : 'destructive'
                      }
                      className={
                        problem.difficulty === '简单' ? 'bg-green-100 text-green-700 hover:bg-green-100/80 dark:bg-green-900/30 dark:text-green-400' :
                        problem.difficulty === '中等' ? 'bg-yellow-100 text-yellow-700 hover:bg-yellow-100/80 dark:bg-yellow-900/30 dark:text-yellow-400' :
                        'bg-red-100 text-red-700 hover:bg-red-100/80 dark:bg-red-900/30 dark:text-red-400'
                      }
                    >
                      {problem.difficulty}
                    </Badge>
                  </TableCell>
                  <TableCell className="text-right">
                    <Button size="sm" variant="ghost" asChild>
                      <Link to={`/problem/${problem.id}`}>
                        做题
                      </Link>
                    </Button>
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </CardContent>
      </Card>
    </div>
  )
}
