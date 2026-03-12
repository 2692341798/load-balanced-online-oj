import { useEffect, useState } from 'react'
import type { ReactNode, HTMLAttributes } from 'react'
import { useParams } from 'react-router-dom'
import ReactMarkdown from 'react-markdown'
import rehypeHighlight from 'rehype-highlight'
import remarkGfm from 'remark-gfm'
import remarkMath from 'remark-math'
import rehypeKatex from 'rehype-katex'
import {
  ResizableHandle,
  ResizablePanel,
  ResizablePanelGroup,
} from '@/components/ui/resizable'
import { ScrollArea } from '@/components/ui/scroll-area'
import { Button } from '@/components/ui/button'
import { Loading } from '@/components/ui/loading'
import { Badge } from '@/components/ui/badge'
import api from '@/lib/axios'
import { type ProblemDetail, type ProblemDetailResponse } from '@/types/problem'
import { useToast } from '@/hooks/use-toast'
import { Copy, Check, Terminal, Clock, Cpu, Award } from 'lucide-react'
import EditorResultWorkspace, {
  type WorkspaceResult,
} from '@/components/EditorResultWorkspace'

import 'highlight.js/styles/github-dark.css' // Import highlight.js style

// Default code templates
const DEFAULT_CODE_TEMPLATES: Record<string, string> = {
  cpp: `#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int main() {
    // Your code here
    return 0;
}`,
  java: `import java.util.*;
import java.io.*;

public class Main {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        // Your code here
        scanner.close();
    }
}`,
  python: `import sys

if __name__ == "__main__":
    # Your code here
    pass
`
}

// Custom Code Block Component with Copy Button
type CodeBlockProps = HTMLAttributes<HTMLElement> & {
  children?: ReactNode
}

const CodeBlock = ({ className, children, ...props }: CodeBlockProps) => {
  const [copied, setCopied] = useState(false)
  const textContent = String(children).replace(/\n$/, '')

  const onCopy = () => {
    navigator.clipboard.writeText(textContent)
    setCopied(true)
    setTimeout(() => setCopied(false), 2000)
  }

  // Detect if it's an inline code or block code
  const isInline = !String(children).includes('\n') && !className

  if (isInline) {
    return <code className="bg-muted px-1 py-0.5 rounded text-sm font-mono text-primary" {...props}>{children}</code>
  }

  return (
    <div className="relative group my-4 rounded-lg overflow-hidden border bg-muted/50">
      <div className="flex items-center justify-between px-4 py-2 bg-muted/80 border-b">
        <div className="flex items-center gap-2">
            <Terminal className="w-4 h-4 text-muted-foreground" />
            <span className="text-xs font-medium text-muted-foreground">Code / Sample</span>
        </div>
        <Button
          variant="ghost"
          size="icon"
          className="h-6 w-6 text-muted-foreground hover:text-foreground"
          onClick={onCopy}
        >
          {copied ? <Check className="h-3.5 w-3.5" /> : <Copy className="h-3.5 w-3.5" />}
        </Button>
      </div>
      <div className="p-4 overflow-x-auto">
        <code className={className} {...props}>
          {children}
        </code>
      </div>
    </div>
  )
}

export default function ProblemDetail() {
  const { id } = useParams<{ id: string }>()
  const [problem, setProblem] = useState<ProblemDetail | null>(null)
  const [isLoading, setIsLoading] = useState(true)
  const [language, setLanguage] = useState('cpp')
  const [code, setCode] = useState<string>(DEFAULT_CODE_TEMPLATES['cpp'])
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [result, setResult] = useState<WorkspaceResult | null>(null)
  const [viewportWidth, setViewportWidth] = useState(() =>
    typeof window === 'undefined' ? 1280 : window.innerWidth
  )
  const { toast } = useToast()
  const isDesktopViewport = viewportWidth >= 1024
  const isMobileViewport = viewportWidth < 768
  const mainLayoutDirection = isDesktopViewport ? 'horizontal' : 'vertical'
  const descriptionDefaultSize = isDesktopViewport ? 50 : isMobileViewport ? 54 : 56
  const descriptionMinSize = isDesktopViewport ? 28 : isMobileViewport ? 34 : 32
  const workspaceDefaultSize = 100 - descriptionDefaultSize
  const workspaceMinSize = isDesktopViewport ? 32 : isMobileViewport ? 34 : 30

  const handleLanguageChange = (newLanguage: string) => {
    setLanguage(newLanguage)
    const isDefaultCode = Object.values(DEFAULT_CODE_TEMPLATES).includes(code) || code.trim() === '' || code === '// Write your code here\n'
    
    if (isDefaultCode) {
      setCode(DEFAULT_CODE_TEMPLATES[newLanguage] || '')
    }
  }

  useEffect(() => {
    const fetchProblem = async () => {
      if (!id) return
      setIsLoading(true)
      try {
        const response = await api.get<ProblemDetailResponse>(`/question/${id}`)
        setProblem(response.data.data)
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

  useEffect(() => {
    const handleResize = () => {
      setViewportWidth(window.innerWidth)
    }
    window.addEventListener('resize', handleResize)
    return () => window.removeEventListener('resize', handleResize)
  }, [])

  const handleSubmit = async () => {
    if (!id || !code) return
    setIsSubmitting(true)
    setResult(null)
    try {
      const response = await api.post(`/judge/${id}`, {
        code,
        language,
        input: '',
      }, {
        baseURL: '/'
      })
      
      const data = response.data
      
      // Construct result object for ResultPanel
       // Prefer stderr for detailed error messages (e.g. compile errors), fallback to reason
       const panelData: WorkspaceResult = {
           status: data.status,
           error: data.stderr || data.reason
       }

      if (data.status === 0) {
         try {
             // Check if stdout is already an object (sometimes axios parses it if content-type is json)
             // But backend says it returns SerializeJson(stdout_json) string inside the json.
             // data.stdout is a string.
             const resultData = typeof data.stdout === 'string' ? JSON.parse(data.stdout) : data.stdout
             panelData.data = resultData
             
             // Check if there is a compile error inside the successful request (status 0 but result says error?)
             // Backend logic: "if (resp_val["status"].asInt() != 0) *out_json = res->body;"
             // So if compile error, data.status might be non-zero (from compile server) passed through?
             // oj_control.hpp: "if (resp_val["status"].asInt() != 0) { *out_json = res->body; return; }"
             // So if compile error, the response body from compile server is returned directly.
             // Compile server response: { status: 1, reason: "...", stdout: "...", stderr: "..." }
             // So data.status will be 1.
             
         } catch {
             panelData.error = "Failed to parse result details"
             panelData.status = -1 
         }
      }
      
      setResult(panelData)
      
    } catch (error: unknown) {
      setResult({
          status: -1,
          error: error instanceof Error ? error.message : 'Submission failed'
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
    <div className="w-full bg-background">
      <div className="mx-auto h-[calc(100vh-3.5rem)] min-h-0 w-full max-w-[1600px] overflow-hidden p-3 md:p-4">
        <ResizablePanelGroup direction={mainLayoutDirection} className="h-full w-full gap-3">
          <ResizablePanel
            defaultSize={descriptionDefaultSize}
            minSize={descriptionMinSize}
            className="min-h-0 min-w-0"
          >
            <div
              className="h-full min-h-0 min-w-0 overflow-hidden rounded-lg border bg-card"
              data-testid="problem-description-panel"
            >
              <ScrollArea className="h-full min-w-0">
                <div className="min-w-[400px] p-6">
                  <div className="space-y-6">
                    <div className="space-y-4 border-b pb-4">
                      <h1 className="flex min-w-0 flex-wrap items-start gap-2 text-3xl font-bold tracking-tight text-primary">
                        <span className="shrink-0 font-mono text-2xl text-muted-foreground">#{problem.number}</span>
                        <span className="min-w-0 break-words">{problem.title}</span>
                      </h1>
                      <div className="flex flex-wrap items-center gap-4 text-sm md:gap-6">
                        <div className="flex items-center gap-2">
                          <Award className="h-4 w-4 text-muted-foreground" />
                          <Badge
                            variant={
                              problem.star === '简单'
                                ? 'secondary'
                                : problem.star === '中等'
                                ? 'default'
                                : 'destructive'
                            }
                            className={`${
                              problem.star === '简单'
                                ? 'bg-green-500/15 text-green-700 hover:bg-green-500/25 dark:text-green-400'
                                : problem.star === '中等'
                                ? 'bg-yellow-500/15 text-yellow-700 hover:bg-yellow-500/25 dark:text-yellow-400'
                                : 'bg-red-500/15 text-red-700 hover:bg-red-500/25 dark:text-red-400'
                            }`}
                          >
                            {problem.star}
                          </Badge>
                        </div>
                        <div className="flex items-center gap-2 text-muted-foreground">
                          <Clock className="h-4 w-4" />
                          <span>Time: {problem.cpu_limit}s</span>
                        </div>
                        <div className="flex items-center gap-2 text-muted-foreground">
                          <Cpu className="h-4 w-4" />
                          <span>Memory: {Math.round((problem.mem_limit || 0) / 1024)}MB</span>
                        </div>
                      </div>
                    </div>

                    <div className="prose prose-slate max-w-none overflow-x-auto break-words dark:prose-invert prose-headings:font-bold prose-headings:tracking-tight prose-h1:mt-8 prose-h1:mb-4 prose-h1:border-b prose-h1:pb-2 prose-h1:text-2xl prose-h2:mt-8 prose-h2:mb-4 prose-h2:border-l-4 prose-h2:border-primary/70 prose-h2:pl-4 prose-h2:text-xl prose-h3:mt-6 prose-h3:mb-3 prose-h3:text-lg prose-p:mb-4 prose-p:leading-7 prose-pre:border-none prose-pre:bg-transparent prose-pre:p-0 prose-pre:shadow-none prose-code:font-mono prose-code:text-sm">
                      <ReactMarkdown
                        remarkPlugins={[remarkGfm, remarkMath]}
                        rehypePlugins={[rehypeHighlight, rehypeKatex]}
                        components={{
                          code: CodeBlock
                        }}
                      >
                        {problem.desc}
                      </ReactMarkdown>
                    </div>
                  </div>
                </div>
              </ScrollArea>
            </div>
          </ResizablePanel>

          <ResizableHandle withHandle data-testid="main-layout-handle" />

          <ResizablePanel
            defaultSize={workspaceDefaultSize}
            minSize={workspaceMinSize}
            className="min-h-0 min-w-0"
          >
            <div className="h-full min-h-0 min-w-0 overflow-hidden" data-testid="composite-workspace">
              <EditorResultWorkspace
                language={language}
                code={code}
                isSubmitting={isSubmitting}
                result={result}
                onLanguageChange={handleLanguageChange}
                onCodeChange={setCode}
                onSubmit={handleSubmit}
                onClearResult={() => setResult(null)}
              />
            </div>
          </ResizablePanel>
        </ResizablePanelGroup>
      </div>
    </div>
  )
}
