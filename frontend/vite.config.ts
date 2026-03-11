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
    chunkSizeWarningLimit: 1000,
  }
})
