import { useState } from 'react';
import { ScrollArea } from '@/components/ui/scroll-area';
// import { Button } from '@/components/ui/button';
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
}

export default function ResultPanel({ status, error, data }: ResultPanelProps) {
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
              {/* Test Case Selection Grid */}
              <div className="p-4 border-b bg-muted/5">
                 <div className="mb-3 text-xs font-medium text-muted-foreground uppercase tracking-wider">{t('result.test_cases')}</div>
                 <div className="grid grid-cols-8 sm:grid-cols-10 md:grid-cols-12 gap-2">
                    {data?.cases?.map((testCase, index) => (
                       <button
                         key={index}
                         onClick={() => setSelectedCaseIndex(index)}
                         className={`
                           relative group flex h-8 w-full items-center justify-center rounded-md border transition-all duration-200
                           ${testCase.pass 
                             ? 'bg-green-500/10 border-green-500/20 text-green-600 hover:bg-green-500/20 hover:border-green-500/30' 
                             : 'bg-red-500/10 border-red-500/20 text-red-600 hover:bg-red-500/20 hover:border-red-500/30'
                           }
                           ${selectedCaseIndex === index ? 'ring-2 ring-offset-2 ring-primary scale-105 shadow-sm' : ''}
                         `}
                         title={`Case ${index + 1}: ${testCase.pass ? 'Passed' : 'Failed'}`}
                       >
                         <span className="text-xs font-bold font-mono">{index + 1}</span>
                         {/* Status Indicator Dot */}
                         <span className={`absolute -top-1 -right-1 flex h-2.5 w-2.5 items-center justify-center`}>
                           <span className={`animate-ping absolute inline-flex h-full w-full rounded-full opacity-75 ${testCase.pass ? 'bg-green-400' : 'bg-red-400'} ${selectedCaseIndex === index ? 'block' : 'hidden'}`}></span>
                           <span className={`relative inline-flex rounded-full h-2 w-2 ${testCase.pass ? 'bg-green-500' : 'bg-red-500'}`}></span>
                         </span>
                       </button>
                    ))}
                 </div>
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
