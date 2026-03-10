import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { Gamepad2 } from "lucide-react"

export default function Games() {
  const games = [
    {
      id: 'dino',
      title: 'Dino Run',
      description: '经典的恐龙跑酷游戏，离线也能玩！',
      path: '/games/dino/index.html',
      icon: '🦖'
    },
    {
      id: 'mario',
      title: 'Super Mario',
      description: '超级马里奥兄弟，重温经典冒险。',
      path: '/games/mario/index.html',
      icon: '🍄'
    },
    {
      id: 'sokoban',
      title: 'Sokoban',
      description: '推箱子益智游戏，考验你的逻辑思维。',
      path: '/games/sokoban/index.html',
      icon: '📦'
    }
  ]

  return (
    <div className="container py-10">
      <div className="mb-8">
        <h1 className="text-3xl font-bold tracking-tight flex items-center gap-2">
          <Gamepad2 className="h-8 w-8" />
          游戏中心
        </h1>
        <p className="text-muted-foreground">放松一下，玩个小游戏吧！</p>
      </div>

      <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
        {games.map((game) => (
          <a 
            key={game.id} 
            href={game.path} 
            target="_blank" 
            rel="noopener noreferrer"
            className="block group"
          >
            <Card className="h-full transition-all hover:shadow-lg group-hover:border-primary/50 cursor-pointer">
              <CardHeader>
                <div className="flex justify-between items-start">
                  <CardTitle className="text-xl group-hover:text-primary">
                    {game.title}
                  </CardTitle>
                  <span className="text-2xl">{game.icon}</span>
                </div>
                <CardDescription>
                  {game.description}
                </CardDescription>
              </CardHeader>
              <CardContent>
                <div className="w-full h-32 bg-muted rounded-md flex items-center justify-center text-4xl group-hover:bg-muted/80 transition-colors">
                  {game.icon}
                </div>
              </CardContent>
            </Card>
          </a>
        ))}
      </div>
    </div>
  )
}
