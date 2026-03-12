import { Link, useNavigate } from 'react-router-dom'
import { Button } from '@/components/ui/button'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '@/components/ui/card'
import { ArrowRight, Code2, Trophy, Zap } from 'lucide-react'
import { useEffect, useState } from 'react'
import api from '@/lib/axios'
import { type Problem, type ProblemListResponse } from '@/types/problem'
import { type Contest, type ContestListResponse } from '@/types/contest'

export default function Home() {
  const [hotProblems, setHotProblems] = useState<Problem[]>([])
  const [upcomingContests, setUpcomingContests] = useState<Contest[]>([])
  const [isLoading, setIsLoading] = useState(true)
  const navigate = useNavigate()

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [problemsRes, contestsRes] = await Promise.all([
          api.get<ProblemListResponse>('/problems?page=1&page_size=5'),
          api.get<ContestListResponse>('/contests?status=upcoming&size=3'),
        ])
        setHotProblems(problemsRes.data.data)
        setUpcomingContests(contestsRes.data.data)
      } catch (error) {
        console.error('Failed to fetch home data:', error)
      } finally {
        setIsLoading(false)
      }
    }
    fetchData()
  }, [])

  return (
    <div className="flex flex-col gap-10 pb-10">
      {/* Hero Section */}
      <section className="relative overflow-hidden bg-background py-20 md:py-32">
        <div className="mx-auto w-full max-w-[1200px] px-4 relative z-10 flex flex-col items-center gap-8 text-center md:flex-row md:text-left lg:gap-16">
          <div className="flex-1 space-y-6">
            <h1 className="text-4xl font-extrabold tracking-tight lg:text-6xl">
              探索代码的 <br className="hidden md:block" />
              <span className="bg-gradient-to-r from-blue-600 to-cyan-500 bg-clip-text text-transparent">
                无限可能
              </span>
            </h1>
            <p className="max-w-[600px] text-lg text-muted-foreground md:text-xl">
              加入我们的在线编程社区，提升你的算法技能，为技术面试做好准备。这里有海量题目等你来挑战。
            </p>
            <div className="flex flex-col gap-4 sm:flex-row sm:justify-center md:justify-start">
              <Button size="lg" asChild className="gap-2">
                <Link to="/problems">
                  开始刷题 <ArrowRight className="h-4 w-4" />
                </Link>
              </Button>
              <Button size="lg" variant="outline" asChild>
                <Link to="/contest">参加竞赛</Link>
              </Button>
            </div>
          </div>
          
          <div className="flex-1 w-full max-w-[600px] perspective-1000">
             <div className="relative rounded-xl border bg-card p-6 shadow-2xl transition-transform hover:scale-[1.02]">
                <div className="absolute top-0 left-0 h-full w-full bg-gradient-to-br from-blue-500/5 to-purple-500/5"></div>
                <div className="flex items-center gap-2 border-b pb-4 mb-4">
                  <div className="flex gap-1.5">
                    <div className="h-3 w-3 rounded-full bg-red-500"></div>
                    <div className="h-3 w-3 rounded-full bg-yellow-500"></div>
                    <div className="h-3 w-3 rounded-full bg-green-500"></div>
                  </div>
                  <span className="text-xs text-muted-foreground ml-2">Solution.cpp</span>
                </div>
                <pre className="overflow-x-auto text-sm font-mono leading-relaxed">
                  <code className="text-foreground">
<span className="text-purple-500">class</span> <span className="text-yellow-500">Solution</span> {'{'}
{'\n'}  <span className="text-purple-500">public</span>:
{'\n'}    <span className="text-blue-500">vector</span>&lt;<span className="text-blue-500">int</span>&gt; <span className="text-yellow-500">twoSum</span>(<span className="text-blue-500">vector</span>&lt;<span className="text-blue-500">int</span>&gt;& nums, <span className="text-blue-500">int</span> target) {'{'}
{'\n'}      <span className="text-gray-500">// 寻找数组中和为目标值的两个数</span>
{'\n'}      <span className="text-blue-500">unordered_map</span>&lt;<span className="text-blue-500">int</span>, <span className="text-blue-500">int</span>&gt; map;
{'\n'}      <span className="text-purple-500">for</span> (<span className="text-blue-500">int</span> i = 0; i &lt; nums.size(); i++) {'{'}
{'\n'}        <span className="text-purple-500">if</span> (map.count(target - nums[i])) {'{'}
{'\n'}          <span className="text-purple-500">return</span> {'{'}map[target - nums[i]], i{'}'};
{'\n'}        {'}'}
{'\n'}        map[nums[i]] = i;
{'\n'}      {'}'}
{'\n'}      <span className="text-purple-500">return</span> {'{}'};
{'\n'}    {'}'}
{'\n'}{'}'};
                  </code>
                </pre>
             </div>
          </div>
        </div>
      </section>

      {/* Features Grid */}
      <section className="mx-auto w-full max-w-[1200px] px-4 grid gap-8 md:grid-cols-2 lg:grid-cols-3">
        <Card>
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <Zap className="h-5 w-5 text-yellow-500" />
              热门题目
            </CardTitle>
            <CardDescription>最受欢迎的算法挑战</CardDescription>
          </CardHeader>
          <CardContent>
            {isLoading ? (
               <div className="space-y-2">
                 {[1, 2, 3].map((i) => (
                   <div key={i} className="h-10 animate-pulse rounded bg-muted"></div>
                 ))}
               </div>
            ) : (
              <ul className="space-y-2">
                {hotProblems.length > 0 ? (
                  hotProblems.map((problem) => (
                    <li key={problem.number}>
                      <Link
                        to={`/problem/${problem.number}`}
                        className="group flex items-center justify-between rounded-md p-2 hover:bg-muted"
                      >
                        <span className="font-medium group-hover:text-primary">
                          {problem.number}. {problem.title}
                        </span>
                        <span className={`text-xs px-2 py-0.5 rounded-full ${
                          problem.star === '简单' ? 'bg-green-100 text-green-700 dark:bg-green-900/30 dark:text-green-400' :
                          problem.star === '中等' ? 'bg-yellow-100 text-yellow-700 dark:bg-yellow-900/30 dark:text-yellow-400' :
                          'bg-red-100 text-red-700 dark:bg-red-900/30 dark:text-red-400'
                        }`}>
                          {problem.star}
                        </span>
                      </Link>
                    </li>
                  ))
                ) : (
                  <p className="text-sm text-muted-foreground">暂无热门题目</p>
                )}
              </ul>
            )}
            <Button variant="link" className="mt-2 w-full px-0" asChild>
              <Link to="/problems">查看更多题目</Link>
            </Button>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <Trophy className="h-5 w-5 text-purple-500" />
              近期竞赛
            </CardTitle>
            <CardDescription>即将开始或正在进行的比赛</CardDescription>
          </CardHeader>
          <CardContent>
             {isLoading ? (
               <div className="space-y-2">
                 {[1, 2].map((i) => (
                   <div key={i} className="h-16 animate-pulse rounded bg-muted"></div>
                 ))}
               </div>
            ) : (
              <ul className="space-y-4">
                {upcomingContests.length > 0 ? (
                  upcomingContests.map((contest, index) => (
                    <li key={index} className="rounded-md border p-3">
                      <div className="font-medium">{contest.name}</div>
                      <div className="mt-1 flex justify-between text-xs text-muted-foreground">
                         <span>{contest.source}</span>
                         <span>{new Date(contest.start_time).toLocaleDateString()}</span>
                      </div>
                      <Button size="sm" variant="secondary" className="mt-2 w-full h-7 text-xs" asChild>
                        <a href={contest.link} target="_blank" rel="noreferrer">
                          查看详情
                        </a>
                      </Button>
                    </li>
                  ))
                ) : (
                  <p className="text-sm text-muted-foreground">近期暂无竞赛安排</p>
                )}
              </ul>
            )}
            <Button variant="link" className="mt-2 w-full px-0" asChild>
              <Link to="/contest">查看所有竞赛</Link>
            </Button>
          </CardContent>
        </Card>
        
        <Card>
           <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <Code2 className="h-5 w-5 text-blue-500" />
              每日一题
            </CardTitle>
            <CardDescription>保持手感，每天进步一点点</CardDescription>
          </CardHeader>
          <CardContent className="flex flex-col items-center justify-center py-6 text-center">
            <div className="mb-4 rounded-full bg-blue-100 p-4 dark:bg-blue-900/20">
              <Code2 className="h-8 w-8 text-blue-600 dark:text-blue-400" />
            </div>
            <h3 className="mb-2 text-lg font-semibold">今日推荐</h3>
            <p className="mb-6 text-sm text-muted-foreground">
              随机抽取一道适合你的题目，开始今天的挑战吧！
            </p>
            <Button className="w-full" onClick={() => {
                // Ideally fetch a random problem ID and navigate
                // For now just navigate to problems
                navigate('/problems')
            }}>
              开始挑战
            </Button>
          </CardContent>
        </Card>
      </section>
    </div>
  )
}
