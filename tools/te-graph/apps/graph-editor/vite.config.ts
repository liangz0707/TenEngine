import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    host: true,   // 监听 0.0.0.0，便于本机浏览器和 Tauri 连接
    port: 5174,
    strictPort: true,
  },
})
