import { useEffect, useState } from 'react'
import { Link, useSearchParams } from 'react-router-dom'
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'
import { Button } from '@/components/ui/button'
import { Input } from '@/components/ui/input'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'
import { Badge } from '@/components/ui/badge'
import { Loading } from '@/components/ui/loading'
import api from '@/lib/axios'
import { type Problem, type ProblemListResponse } from '@/types/problem'
import { ChevronLeft, ChevronRight, ChevronsLeft, ChevronsRight, Search } from 'lucide-react'

export default function ProblemList() {
  const [problems, setProblems] = useState<Problem[]>([])
  const [total, setTotal] = useState(0)
  const [isLoading, setIsLoading] = useState(true)
  const [jumpPage, setJumpPage] = useState('')
  const [searchParams, setSearchParams] = useSearchParams()

  const page = parseInt(searchParams.get('page') || '1')
  const pageSize = 20
  const search = searchParams.get('search') || ''
  const difficulty = searchParams.get('difficulty') || 'all'

  useEffect(() => {
    const fetchProblems = async () => {
      setIsLoading(true)
      try {
        // Construct query params
        const params = new URLSearchParams()
        params.append('page', page.toString())
        params.append('page_size', pageSize.toString())
        if (search) params.append('search', search) // Assuming API supports search
        if (difficulty !== 'all') params.append('difficulty', difficulty) // Assuming API supports difficulty

        const response = await api.get<ProblemListResponse>(`/problems?${params.toString()}`)
        setProblems(response.data.data)
        setTotal(response.data.total)
      } catch (error) {
        console.error('Failed to fetch problems:', error)
      } finally {
        setIsLoading(false)
      }
    }
    fetchProblems()
  }, [page, search, difficulty])

  const totalPages = Math.ceil(total / pageSize)

  const handleSearch = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault()
    const formData = new FormData(e.currentTarget)
    const newSearch = formData.get('search') as string
    setSearchParams({ page: '1', search: newSearch, difficulty })
  }

  const handleDifficultyChange = (value: string) => {
    setSearchParams({ page: '1', search, difficulty: value })
  }

  const handlePageChange = (newPage: number) => {
    setSearchParams({ page: newPage.toString(), search, difficulty })
    setJumpPage('')
    window.scrollTo({ top: 0, behavior: 'smooth' })
  }

  const handleJump = () => {
     const p = parseInt(jumpPage, 10)
     if (!isNaN(p) && p >= 1 && p <= totalPages) {
       handlePageChange(p)
     }
   }


  return (
    <div className="container py-10">
      <div className="mb-8 flex flex-col gap-4 md:flex-row md:items-center md:justify-between">
        <div>
          <h1 className="text-3xl font-bold tracking-tight">题目列表</h1>
          <p className="text-muted-foreground">浏览并挑战海量算法题目</p>
        </div>
        <div className="flex flex-col gap-2 sm:flex-row">
          <form onSubmit={handleSearch} className="flex gap-2">
            <Input
              name="search"
              placeholder="搜索题目..."
              defaultValue={search}
              className="w-full sm:w-[200px]"
            />
            <Button type="submit" size="icon" variant="secondary">
              <Search className="h-4 w-4" />
            </Button>
          </form>
          <Select value={difficulty} onValueChange={handleDifficultyChange}>
            <SelectTrigger className="w-full sm:w-[120px]">
              <SelectValue placeholder="难度" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="all">所有难度</SelectItem>
              <SelectItem value="简单">简单</SelectItem>
              <SelectItem value="中等">中等</SelectItem>
              <SelectItem value="困难">困难</SelectItem>
            </SelectContent>
          </Select>
        </div>
      </div>

      <div className="rounded-md border bg-card text-card-foreground shadow-sm">
        <Table>
          <TableHeader>
            <TableRow>
              <TableHead className="w-[100px]">编号</TableHead>
              <TableHead>标题</TableHead>
              <TableHead className="w-[100px]">难度</TableHead>
              <TableHead className="w-[100px] text-right">操作</TableHead>
            </TableRow>
          </TableHeader>
          <TableBody>
            {isLoading ? (
              <TableRow>
                <TableCell colSpan={4} className="h-24 text-center">
                  <Loading className="mx-auto" />
                </TableCell>
              </TableRow>
            ) : problems.length > 0 ? (
              problems.map((problem) => (
                <TableRow key={problem.number}>
                  <TableCell className="font-medium">#{problem.number}</TableCell>
                  <TableCell>
                    <Link
                      to={`/problem/${problem.number}`}
                      className="hover:underline hover:text-primary transition-colors font-medium"
                    >
                      {problem.title}
                    </Link>
                  </TableCell>
                  <TableCell>
                    <Badge
                      variant={
                        problem.star === '简单'
                          ? 'secondary'
                          : problem.star === '中等'
                          ? 'default'
                          : 'destructive'
                      }
                      className={
                        problem.star === '简单'
                          ? 'bg-green-100 text-green-700 hover:bg-green-100/80 dark:bg-green-900/30 dark:text-green-400'
                          : problem.star === '中等'
                          ? 'bg-yellow-100 text-yellow-700 hover:bg-yellow-100/80 dark:bg-yellow-900/30 dark:text-yellow-400'
                          : 'bg-red-100 text-red-700 hover:bg-red-100/80 dark:bg-red-900/30 dark:text-red-400'
                      }
                    >
                      {problem.star}
                    </Badge>
                  </TableCell>
                  <TableCell className="text-right">
                    <Button asChild size="sm" variant="outline">
                      <Link to={`/problem/${problem.number}`}>做题</Link>
                    </Button>
                  </TableCell>
                </TableRow>
              ))
            ) : (
              <TableRow>
                <TableCell colSpan={4} className="h-24 text-center">
                  暂无题目
                </TableCell>
              </TableRow>
            )}
          </TableBody>
        </Table>
      </div>

      {/* Pagination */}
      {totalPages > 1 && (
        <div className="mt-8 flex flex-col sm:flex-row items-center justify-center gap-4">
          <div className="flex items-center gap-2">
            <Button
              variant="outline"
              size="icon"
              onClick={() => handlePageChange(1)}
              disabled={page <= 1 || isLoading}
              title="首页"
            >
              <ChevronsLeft className="h-4 w-4" />
            </Button>
            <Button
              variant="outline"
              size="sm"
              onClick={() => handlePageChange(page - 1)}
              disabled={page <= 1 || isLoading}
              className="gap-1"
            >
              <ChevronLeft className="h-4 w-4" />
              上一页
            </Button>
            <span className="text-sm text-muted-foreground min-w-[100px] text-center">
              第 {page} 页 / 共 {Math.max(1, totalPages)} 页
            </span>
            <Button
              variant="outline"
              size="sm"
              onClick={() => handlePageChange(page + 1)}
              disabled={page >= totalPages || isLoading}
              className="gap-1"
            >
              下一页
              <ChevronRight className="h-4 w-4" />
            </Button>
            <Button
              variant="outline"
              size="icon"
              onClick={() => handlePageChange(totalPages)}
              disabled={page >= totalPages || isLoading}
              title="尾页"
            >
              <ChevronsRight className="h-4 w-4" />
            </Button>
          </div>

          <div className="flex items-center gap-2">
            <span className="text-sm text-muted-foreground whitespace-nowrap">前往</span>
            <Input
              type="number"
              min={1}
              max={totalPages}
              value={jumpPage}
              onChange={(e) => setJumpPage(e.target.value)}
              onKeyDown={(e) => e.key === 'Enter' && handleJump()}
              className="w-16 h-8 text-center"
              placeholder={String(page)}
            />
            <span className="text-sm text-muted-foreground whitespace-nowrap">页</span>
            <Button 
              variant="outline" 
              size="sm" 
              onClick={handleJump}
              disabled={!jumpPage || parseInt(jumpPage) < 1 || parseInt(jumpPage) > totalPages}
            >
              跳转
            </Button>
          </div>
        </div>
      )}
    </div>
  )
}
