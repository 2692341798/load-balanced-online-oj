import { useEffect, useState } from 'react'
import {
  Card,
  CardContent,
} from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { Badge } from '@/components/ui/badge'
import api from '@/lib/axios'
import { type Contest, type ContestListResponse } from '@/types/contest'
import { Calendar, Clock, ExternalLink } from 'lucide-react'

export default function ContestList() {
  const [contests, setContests] = useState<Contest[]>([])
  const [isLoading, setIsLoading] = useState(true)

  useEffect(() => {
    const fetchContests = async () => {
      setIsLoading(true)
      try {
        const response = await api.get<ContestListResponse>('/contests?status=all&size=20')
        setContests(response.data.data || [])
      } catch (error) {
        console.error('Failed to fetch contests:', error)
      } finally {
        setIsLoading(false)
      }
    }
    fetchContests()
  }, [])

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'running':
        return 'bg-green-500 hover:bg-green-600'
      case 'upcoming':
        return 'bg-blue-500 hover:bg-blue-600'
      case 'ended':
        return 'bg-gray-500 hover:bg-gray-600'
      default:
        return 'bg-gray-500'
    }
  }

  const getStatusText = (status: string) => {
    switch (status) {
      case 'running':
        return '进行中'
      case 'upcoming':
        return '即将开始'
      case 'ended':
        return '已结束'
      default:
        return status
    }
  }

  return (
    <div className="container py-10">
      <div className="mb-8">
        <h1 className="text-3xl font-bold tracking-tight">竞赛中心</h1>
        <p className="text-muted-foreground">参与高质量算法竞赛，检验实力</p>
      </div>

      {isLoading ? (
        <div className="space-y-4">
          {[1, 2, 3].map((i) => (
            <div key={i} className="h-24 animate-pulse rounded-lg border bg-muted"></div>
          ))}
        </div>
      ) : contests.length > 0 ? (
        <div className="space-y-4">
          {contests.map((contest, index) => (
            <Card key={index} className="hover:shadow-md transition-shadow">
              <CardContent className="p-6 flex flex-col md:flex-row items-start md:items-center justify-between gap-4">
                <div className="space-y-2">
                  <div className="flex items-center gap-2">
                    <h3 className="text-lg font-semibold">{contest.name}</h3>
                    <Badge className={getStatusColor(contest.status)}>
                      {getStatusText(contest.status)}
                    </Badge>
                  </div>
                  <div className="flex flex-col sm:flex-row gap-2 sm:gap-6 text-sm text-muted-foreground">
                    <div className="flex items-center gap-1">
                      <Calendar className="h-4 w-4" />
                      <span>{new Date(contest.start_time).toLocaleString()}</span>
                    </div>
                    <div className="flex items-center gap-1">
                      <Clock className="h-4 w-4" />
                      <span>时长: {((new Date(contest.end_time).getTime() - new Date(contest.start_time).getTime()) / 3600000).toFixed(1)} 小时</span>
                    </div>
                    <div>
                      来源: {contest.source}
                    </div>
                  </div>
                </div>
                <Button asChild>
                  <a href={contest.link} target="_blank" rel="noreferrer">
                    <ExternalLink className="mr-2 h-4 w-4" />
                    前往参赛
                  </a>
                </Button>
              </CardContent>
            </Card>
          ))}
        </div>
      ) : (
        <div className="text-center py-20 text-muted-foreground">
          暂无竞赛安排
        </div>
      )}
    </div>
  )
}
