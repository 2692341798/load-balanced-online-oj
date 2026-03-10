import { Loader2 } from 'lucide-react'
import { cn } from '@/lib/utils'

export interface LoadingProps extends React.HTMLAttributes<HTMLDivElement> {
  size?: number
}

export function Loading({ size = 24, className, ...props }: LoadingProps) {
  return (
    <div className={cn('flex justify-center items-center', className)} {...props}>
      <Loader2 className="animate-spin" size={size} />
    </div>
  )
}
