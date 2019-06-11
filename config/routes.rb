Rails.application.routes.draw do
  get '/register-device', to: 'sensors#register'
  post '/add-cycle', to: 'sensors#add_cycle'
  resources :sensors
end
