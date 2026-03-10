import { useEffect, useState } from 'react'
import api from '@/lib/axios'
import { type ProfileResponse } from '@/types/user'
import { type Submission, type SubmissionListResponse } from '@/types/submission'
import { Loading } from '@/components/ui/loading'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card'
import { Avatar, AvatarFallback, AvatarImage } from '@/components/ui/avatar'
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs'
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'
import { Badge } from '@/components/ui/badge'
import { Separator } from '@/components/ui/separator'

export default function Profile() {
  const [profile, setProfile] = useState<ProfileResponse | null>(null)
  const [submissions, setSubmissions] = useState<Submission[]>([])
  const [isLoading, setIsLoading] = useState(true)

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [profileRes, submissionsRes] = await Promise.all([
          api.get<ProfileResponse>('/profile'),
          api.get<SubmissionListResponse>('/submissions?page=1&page_size=10'),
        ])
        setProfile(profileRes.data)
        setSubmissions(submissionsRes.data.data)
      } catch (error) {
        console.error('Failed to fetch profile data:', error)
      } finally {
        setIsLoading(false)
      }
    }
    fetchData()
  }, [])

  if (isLoading) {
    return (
      <div className="flex h-[50vh] items-center justify-center">
        <Loading />
      </div>
    )
  }

  if (!profile) {
    return <div>Failed to load profile.</div>
  }

  return (
    <div className="container py-10">
      <div className="grid gap-6 md:grid-cols-[300px_1fr]">
        <div className="space-y-6">
          <Card>
            <CardHeader className="text-center">
              <div className="mx-auto mb-4">
                <Avatar className="h-24 w-24">
                  <AvatarImage src={profile.avatar} alt={profile.username} />
                  <AvatarFallback className="text-2xl">
                    {profile.username.charAt(0).toUpperCase()}
                  </AvatarFallback>
                </Avatar>
              </div>
              <CardTitle>{profile.nickname || profile.username}</CardTitle>
              <CardDescription>@{profile.username}</CardDescription>
            </CardHeader>
            <CardContent>
              <div className="space-y-4 text-sm">
                <div className="grid grid-cols-2 gap-1">
                  <span className="text-muted-foreground">邮箱:</span>
                  <span className="truncate">{profile.email}</span>
                </div>
                <div className="grid grid-cols-2 gap-1">
                  <span className="text-muted-foreground">加入时间:</span>
                  <span>{new Date(profile.created_at).toLocaleDateString()}</span>
                </div>
                <div className="grid grid-cols-2 gap-1">
                  <span className="text-muted-foreground">角色:</span>
                  <span>{profile.role === 1 ? '管理员' : '普通用户'}</span>
                </div>
              </div>
              <Separator className="my-4" />
              <div className="space-y-2">
                <h4 className="font-medium">做题统计</h4>
                <div className="grid grid-cols-3 gap-2 text-center text-xs">
                  <div className="rounded bg-green-100 p-2 dark:bg-green-900/30">
                    <div className="font-bold text-green-700 dark:text-green-400">
                      {profile.stats['简单'] || 0}
                    </div>
                    <div className="text-muted-foreground">简单</div>
                  </div>
                  <div className="rounded bg-yellow-100 p-2 dark:bg-yellow-900/30">
                    <div className="font-bold text-yellow-700 dark:text-yellow-400">
                      {profile.stats['中等'] || 0}
                    </div>
                    <div className="text-muted-foreground">中等</div>
                  </div>
                  <div className="rounded bg-red-100 p-2 dark:bg-red-900/30">
                    <div className="font-bold text-red-700 dark:text-red-400">
                      {profile.stats['困难'] || 0}
                    </div>
                    <div className="text-muted-foreground">困难</div>
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>
        </div>

        <div className="space-y-6">
          <Tabs defaultValue="submissions">
            <TabsList>
              <TabsTrigger value="submissions">最近提交</TabsTrigger>
              <TabsTrigger value="activity">活动记录</TabsTrigger>
            </TabsList>
            <TabsContent value="submissions" className="space-y-4">
              <Card>
                <CardHeader>
                  <CardTitle>提交记录</CardTitle>
                  <CardDescription>最近的 10 次提交</CardDescription>
                </CardHeader>
                <CardContent>
                  <Table>
                    <TableHeader>
                      <TableRow>
                        <TableHead>题目</TableHead>
                        <TableHead>结果</TableHead>
                        <TableHead>语言</TableHead>
                        <TableHead>耗时</TableHead>
                        <TableHead>内存</TableHead>
                        <TableHead className="text-right">提交时间</TableHead>
                      </TableRow>
                    </TableHeader>
                    <TableBody>
                      {submissions.length > 0 ? (
                        submissions.map((sub) => (
                          <TableRow key={sub.id}>
                            <TableCell className="font-medium">
                              #{sub.question_id} {sub.question_title}
                            </TableCell>
                            <TableCell>
                              <Badge
                                variant={sub.result === '0' ? 'secondary' : 'destructive'}
                                className={
                                  sub.result === '0'
                                    ? 'bg-green-100 text-green-700 hover:bg-green-100/80 dark:bg-green-900/30 dark:text-green-400'
                                    : 'bg-red-100 text-red-700 hover:bg-red-100/80 dark:bg-red-900/30 dark:text-red-400'
                                }
                              >
                                {sub.result === '0' ? '通过' : '未通过'}
                              </Badge>
                            </TableCell>
                            <TableCell>{sub.language}</TableCell>
                            <TableCell>{sub.cpu_time}ms</TableCell>
                            <TableCell>{Math.round(sub.mem_usage / 1024)}MB</TableCell>
                            <TableCell className="text-right text-muted-foreground">
                              {new Date(sub.created_at).toLocaleString()}
                            </TableCell>
                          </TableRow>
                        ))
                      ) : (
                        <TableRow>
                          <TableCell colSpan={6} className="h-24 text-center">
                            暂无提交记录
                          </TableCell>
                        </TableRow>
                      )}
                    </TableBody>
                  </Table>
                </CardContent>
              </Card>
            </TabsContent>
            <TabsContent value="activity">
              <Card>
                <CardHeader>
                  <CardTitle>活动记录</CardTitle>
                </CardHeader>
                <CardContent>
                  <div className="flex h-40 items-center justify-center text-muted-foreground">
                    暂无活动数据
                  </div>
                </CardContent>
              </Card>
            </TabsContent>
          </Tabs>
        </div>
      </div>
    </div>
  )
}
