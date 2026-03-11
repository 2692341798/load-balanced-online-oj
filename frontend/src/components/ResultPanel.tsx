import { useState } from 'react';
import { ScrollArea } from '@/components/ui/scroll-area';
import { Button } from '@/components/ui/button';
import { useTranslation } from '@/hooks/useTranslation';
import { AlertTriangle } from 'lucide-react';

interface TestCase {
  name: string;
  pass: boolean;
  input: string;
  output: string;
  expected: string;
}

interface Summary {
  total: number;
  passed: number;
  overall: string;
}

interface ResultData {
  cases?: TestCase[];
  summary?: Summary;
  raw?: string; // Fallback
}

interface ResultPanelProps {
  status: number; // 0: success, others: error
  error?: string; // Reason or stderr
  data?: ResultData; // Parsed stdout
  onResubmit: () => void;
  onBack: () => void;
  isSubmitting: boolean;
}

export default function ResultPanel({ status, error, data, onResubmit, onBack, isSubmitting }: ResultPanelProps) {
  const { t } = useTranslation();
  const [selectedCaseIndex, setSelectedCaseIndex] = useState(0);

  // Determine overall status
  const isCompileError = status !== 0;
  // If status is 0, check summary for passed count
  const isAllPassed = !isCompileError && data?.summary?.overall === 'All Passed';
  
  return (
    <div className="relative z-10 flex h-full min-h-0 flex-col bg-background" data-testid="result-panel">
       {/* Header */}
       <div className="flex flex-col gap-2 border-b bg-muted/30 px-4 py-2 text-sm font-medium sm:flex-row sm:items-center sm:justify-between">
          <div className="flex items-center gap-2">
            <span className={
              isCompileError ? "text-red-500" :
              isAllPassed ? "text-green-500" : "text-red-500"
            }>
              {isCompileError ? t('status.compile_error') : 
               isAllPassed ? t('status.accepted') : t('status.wrong_answer')}
            </span>
          </div>
          <div className="flex gap-2">
             <Button variant="outline" size="sm" onClick={onBack}>{t('action.back_to_problem')}</Button>
             <Button size="sm" onClick={onResubmit} disabled={isSubmitting}>
               {isSubmitting ? t('action.submitting') : t('action.resubmit')}
             </Button>
          </div>
       </div>

       {/* Content */}
       <div className="flex-1 min-h-0 flex flex-col">
         {isCompileError ? (
            <ScrollArea className="flex-1 p-4">
              <div className="space-y-2">
                <div className="flex items-center gap-2 text-red-500 font-medium">
                  <AlertTriangle className="w-4 h-4" />
                  {t('result.stderr')}
                </div>
                <pre className="p-4 bg-red-500/10 text-red-500 rounded-md font-mono text-sm whitespace-pre-wrap">
                  {error || t('status.unknown_error')}
                </pre>
              </div>
            </ScrollArea>
         ) : (
            <>
              {/* Test Case Tabs */}
              <div className="flex gap-2 p-2 border-b overflow-x-auto bg-muted/10">
                 {data?.cases?.map((testCase, index) => (
                    <Button
                      key={index}
                      variant={selectedCaseIndex === index ? "secondary" : "ghost"}
                      size="sm"
                      onClick={() => setSelectedCaseIndex(index)}
                      className={`h-8 gap-2 ${selectedCaseIndex === index ? 'bg-background shadow-sm' : ''}`}
                    >
                      <div className={`w-2 h-2 rounded-full ${testCase.pass ? 'bg-green-500' : 'bg-red-500'}`} />
                      {t('case')} {index + 1}
                    </Button>
                 ))}
              </div>

              {/* Detail View */}
              <ScrollArea className="flex-1 p-4">
                 {data?.cases && data.cases[selectedCaseIndex] && (
                    <div className="space-y-4 font-mono text-sm">
                        <div className="space-y-1">
                            <div className="text-xs text-muted-foreground">{t('result.input')}</div>
                            <div className="p-3 bg-muted/50 rounded-md border">
                                {data.cases[selectedCaseIndex].input}
                            </div>
                        </div>
                        
                        <div className="space-y-1">
                            <div className="text-xs text-muted-foreground">{t('result.output')}</div>
                            <div className={`p-3 rounded-md border ${
                                data.cases[selectedCaseIndex].pass 
                                 ? 'bg-muted/50' 
                                 : 'bg-red-500/10 border-red-500/20'
                            }`}>
                                {data.cases[selectedCaseIndex].output}
                            </div>
                        </div>
                        
                        <div className="space-y-1">
                            <div className="text-xs text-muted-foreground">{t('result.expected')}</div>
                            <div className="p-3 bg-muted/50 rounded-md border">
                                {data.cases[selectedCaseIndex].expected}
                            </div>
                        </div>
                    </div>
                 )}
              </ScrollArea>
            </>
         )}
       </div>
    </div>
  );
}
