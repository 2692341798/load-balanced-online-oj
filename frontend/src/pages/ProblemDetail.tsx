import { useEffect, useState } from 'react'
import { useParams } from 'react-router-dom'
import Editor from '@monaco-editor/react'
import ReactMarkdown from 'react-markdown'
import rehypeHighlight from 'rehype-highlight'
import remarkGfm from 'remark-gfm'
import {
  ResizableHandle,
  ResizablePanel,
  ResizablePanelGroup,
} from '@/components/ui/resizable'
import { ScrollArea } from '@/components/ui/scroll-area'
import { Button } from '@/components/ui/button'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'
import { Loading } from '@/components/ui/loading'
import { Badge } from '@/components/ui/badge'
import api from '@/lib/axios'
import { type ProblemDetail, type ProblemDetailResponse } from '@/types/problem'
import { useToast } from '@/hooks/use-toast'
import { Play, Send } from 'lucide-react'

import 'highlight.js/styles/github-dark.css' // Import highlight.js style

export default function ProblemDetail() {
  const { id } = useParams<{ id: string }>()
  const [problem, setProblem] = useState<ProblemDetail | null>(null)
  const [isLoading, setIsLoading] = useState(true)
  const [code, setCode] = useState<string>('// Write your code here\n')
  const [language, setLanguage] = useState('cpp')
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [result, setResult] = useState<any>(null)
  const { toast } = useToast()

  useEffect(() => {
    const fetchProblem = async () => {
      if (!id) return
      setIsLoading(true)
      try {
        const response = await api.get<ProblemDetailResponse>(`/question/${id}`)
        setProblem(response.data.data)
        // Set default code template based on language if needed
      } catch (error) {
        console.error('Failed to fetch problem:', error)
        toast({
          variant: 'destructive',
          title: 'Error',
          description: 'Failed to load problem details',
        })
      } finally {
        setIsLoading(false)
      }
    }
    fetchProblem()
  }, [id, toast])

  const handleSubmit = async () => {
    if (!id || !code) return
    setIsSubmitting(true)
    setResult(null)
    try {
      const response = await api.post(`/judge/${id}`, {
        code,
        language,
        input: '', // Optional input for custom test
      })
      
      const data = response.data
      if (data.status === 0) {
         // Parse stdout JSON if possible, or just show raw
         try {
             const resultData = JSON.parse(data.stdout)
             setResult(resultData)
             toast({
               title: 'Submission Complete',
               description: resultData.summary?.overall || 'Check results',
             })
         } catch (e) {
             setResult({ raw: data.stdout })
         }
      } else {
        toast({
          variant: 'destructive',
          title: 'Submission Failed',
          description: data.reason || 'Unknown error',
        })
      }
    } catch (error: any) {
      toast({
        variant: 'destructive',
        title: 'Error',
        description: error.message || 'Submission failed',
      })
    } finally {
      setIsSubmitting(false)
    }
  }

  if (isLoading) {
    return (
      <div className="flex h-screen items-center justify-center">
        <Loading />
      </div>
    )
  }

  if (!problem) {
    return <div>Problem not found</div>
  }

  return (
    <div className="h-[calc(100vh-3.5rem)] w-full overflow-hidden">
      <ResizablePanelGroup orientation="horizontal">
        {/* Left Panel: Problem Description */}
        <ResizablePanel defaultSize={40} minSize={30}>
          <ScrollArea className="h-full">
            <div className="p-6">
              <div className="mb-4 space-y-2">
                <h1 className="text-2xl font-bold">
                  {problem.number}. {problem.title}
                </h1>
                <div className="flex gap-2">
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
                  <Badge variant="outline">
                    Time: {problem.cpu_limit}s
                  </Badge>
                  <Badge variant="outline">
                    Memory: {Math.round((problem.mem_limit || 0) / 1024)}MB
                  </Badge>
                </div>
              </div>
              
              <div className="prose prose-sm dark:prose-invert max-w-none">
                <ReactMarkdown
                  remarkPlugins={[remarkGfm]}
                  rehypePlugins={[rehypeHighlight]}
                >
                  {problem.desc}
                </ReactMarkdown>
              </div>
            </div>
          </ScrollArea>
        </ResizablePanel>

        <ResizableHandle withHandle />

        {/* Right Panel: Editor */}
        <ResizablePanel defaultSize={60} minSize={30}>
           <div className="flex h-full flex-col">
             <div className="flex items-center justify-between border-b p-2">
               <Select value={language} onValueChange={setLanguage}>
                 <SelectTrigger className="w-[120px] h-8">
                   <SelectValue placeholder="Language" />
                 </SelectTrigger>
                 <SelectContent>
                   <SelectItem value="cpp">C++</SelectItem>
                   <SelectItem value="java">Java</SelectItem>
                   <SelectItem value="python">Python</SelectItem>
                 </SelectContent>
               </Select>
               
               <div className="flex gap-2">
                 <Button size="sm" variant="secondary" onClick={() => {/* Run Test */}}>
                   <Play className="mr-2 h-4 w-4" />
                   Run
                 </Button>
                 <Button size="sm" onClick={handleSubmit} disabled={isSubmitting}>
                   <Send className="mr-2 h-4 w-4" />
                   {isSubmitting ? 'Submitting...' : 'Submit'}
                 </Button>
               </div>
             </div>
             
             <div className="flex-1 overflow-hidden">
               <Editor
                 height="100%"
                 language={language === 'c++' ? 'cpp' : language} // map language for Monaco
                 theme="vs-dark"
                 value={code}
                 onChange={(value) => setCode(value || '')}
                 options={{
                   minimap: { enabled: false },
                   fontSize: 14,
                   scrollBeyondLastLine: false,
                   automaticLayout: true,
                 }}
               />
             </div>
             
             {/* Result Panel (Optional - can be collapsible) */}
             {result && (
               <div className="h-[200px] border-t bg-muted/50 p-4 overflow-auto">
                 <h3 className="font-semibold mb-2">Result</h3>
                 {result.summary ? (
                    <div>
                      <div className={`text-lg font-bold ${result.summary.overall === 'All Passed' ? 'text-green-600' : 'text-red-600'}`}>
                        {result.summary.overall}
                      </div>
                      <div className="text-sm text-muted-foreground">
                        Passed: {result.summary.passed} / {result.summary.total}
                      </div>
                      <div className="mt-2 space-y-1">
                        {result.cases?.map((testCase: any, index: number) => (
                          <div key={index} className="text-xs flex gap-2">
                             <span className={testCase.pass ? 'text-green-500' : 'text-red-500'}>
                               {testCase.name}: {testCase.pass ? 'Passed' : 'Failed'}
                             </span>
                             {!testCase.pass && (
                               <span className="text-muted-foreground">
                                 (Expected: {testCase.expected}, Got: {testCase.output})
                               </span>
                             )}
                          </div>
                        ))}
                      </div>
                    </div>
                 ) : (
                    <pre className="text-xs">{result.raw || JSON.stringify(result, null, 2)}</pre>
                 )}
               </div>
             )}
           </div>
        </ResizablePanel>
      </ResizablePanelGroup>
    </div>
  )
}
