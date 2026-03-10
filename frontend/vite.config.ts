import path from "path"
import react from "@vitejs/plugin-react"
import { defineConfig } from "vite"

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "./src"),
    },
  },
  server: {
    proxy: {
      '/api': {
        target: 'http://localhost:8094',
        changeOrigin: true,
        secure: false,
      },
      '/judge': {
        target: 'http://localhost:8094',
        changeOrigin: true,
        secure: false,
      },
      '/uploads': {
        target: 'http://localhost:8094',
        changeOrigin: true,
        secure: false,
      },
    },
  },
  build: {
    rollupOptions: {
      output: {
        manualChunks: {
          'vendor': ['react', 'react-dom', 'react-router-dom', 'zustand', 'axios'],
          'ui': ['@radix-ui/react-avatar', '@radix-ui/react-dialog', '@radix-ui/react-dropdown-menu', '@radix-ui/react-label', '@radix-ui/react-scroll-area', '@radix-ui/react-select', '@radix-ui/react-separator', '@radix-ui/react-slot', '@radix-ui/react-tabs', '@radix-ui/react-toast', 'lucide-react', 'class-variance-authority', 'clsx', 'tailwind-merge'],
          'markdown': ['react-markdown', 'rehype-highlight', 'remark-gfm', 'highlight.js'],
          'monaco': ['@monaco-editor/react'],
        }
      }
    },
    chunkSizeWarningLimit: 1000,
  }
})
