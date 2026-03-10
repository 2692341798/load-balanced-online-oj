import axios from 'axios'

const api = axios.create({
  baseURL: '/api',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json',
  },
})

api.interceptors.response.use(
  (response) => response,
  (error) => {
    // Handle global errors (e.g. 401, 500)
    if (error.response) {
      if (error.response.status === 401) {
        // Redirect to login or refresh token
        console.warn('Unauthorized, please login.')
      }
    }
    return Promise.reject(error)
  }
)

export default api
