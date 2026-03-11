import { useEffect, useState } from 'react'
import { Send } from 'lucide-react'
import CodeEditor from '@/components/CodeEditor'
import ResultPanel from '@/components/ResultPanel'
import { Button } from '@/components/ui/button'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'
import {
  ResizableHandle,
  ResizablePanel,
  ResizablePanelGroup,
} from '@/components/ui/resizable'

interface TestCaseResult {
  name: string
  pass: boolean
  input: string
  output: string
  expected: string
}

interface JudgeResultData {
  cases?: TestCaseResult[]
  summary?: {
    total: number
    passed: number
    overall: string
  }
  raw?: string
}

export interface WorkspaceResult {
  status: number
  error?: string
  data?: JudgeResultData
}

interface EditorResultWorkspaceProps {
  language: string
  code: string
  isSubmitting: boolean
  result: WorkspaceResult | null
  onLanguageChange: (language: string) => void
  onCodeChange: (code: string) => void
  onSubmit: () => void
  onClearResult: () => void
}

export default function EditorResultWorkspace({
  language,
  code,
  isSubmitting,
  result,
  onLanguageChange,
  onCodeChange,
  onSubmit,
  onClearResult,
}: EditorResultWorkspaceProps) {
  const [viewportWidth, setViewportWidth] = useState(() =>
    typeof window === 'undefined' ? 1280 : window.innerWidth
  )
  const isTabletViewport = viewportWidth < 1024
  const isMobileViewport = viewportWidth < 768
  const editorDefaultSize = result ? (isMobileViewport ? 58 : isTabletViewport ? 62 : 70) : 100
  const editorMinSize = result ? (isMobileViewport ? 40 : isTabletViewport ? 34 : 25) : 25
  const resultDefaultSize = 100 - editorDefaultSize
  const resultMinSize = isMobileViewport ? 28 : isTabletViewport ? 24 : 20

  useEffect(() => {
    const handleResize = () => {
      setViewportWidth(window.innerWidth)
    }
    window.addEventListener('resize', handleResize)
    return () => window.removeEventListener('resize', handleResize)
  }, [])

  return (
    <div className="flex h-full min-h-0 flex-col overflow-hidden rounded-lg border bg-card">
      <div className="flex flex-col gap-3 border-b bg-muted/20 px-4 py-2 sm:flex-row sm:flex-wrap sm:items-center sm:justify-between">
        <div className="flex items-center gap-4">
          <span className="text-sm font-medium text-muted-foreground">Language:</span>
          <Select value={language} onValueChange={onLanguageChange}>
            <SelectTrigger className="h-8 w-[140px] bg-background">
              <SelectValue placeholder="Select Language" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="cpp">C++</SelectItem>
              <SelectItem value="java">Java</SelectItem>
              <SelectItem value="python">Python</SelectItem>
            </SelectContent>
          </Select>
        </div>
        <div className="flex gap-2 self-start sm:self-auto">
          <Button size="sm" onClick={onSubmit} disabled={isSubmitting} className="h-8">
            <Send className="mr-2 h-3.5 w-3.5" />
            {isSubmitting ? 'Submitting...' : 'Submit'}
          </Button>
        </div>
      </div>

      <div className="flex-1 min-h-0">
        <ResizablePanelGroup direction="vertical">
          <ResizablePanel defaultSize={editorDefaultSize} minSize={editorMinSize} className="min-h-0">
            <div className="relative h-full min-h-0 overflow-hidden" data-testid="editor-panel">
              <CodeEditor
                language={language}
                value={code}
                onChange={(value) => onCodeChange(value || '')}
                fontSize={14}
              />
            </div>
          </ResizablePanel>
          {result && (
            <>
              <ResizableHandle withHandle />
              <ResizablePanel
                defaultSize={resultDefaultSize}
                minSize={resultMinSize}
                className="min-h-0"
              >
                <ResultPanel
                  status={result.status}
                  error={result.error}
                  data={result.data}
                  onResubmit={onSubmit}
                  onBack={onClearResult}
                  isSubmitting={isSubmitting}
                />
              </ResizablePanel>
            </>
          )}
        </ResizablePanelGroup>
      </div>
    </div>
  )
}
