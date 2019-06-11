Rails.application.routes.draw do
  get '/test', to: 'sensors#test'
  get '/register-device', to: 'sensors#register'
  post '/posty', to: 'sensors#posty'
  post '/add-cycle', to: 'sensors#add_cycle'
  resources :sensors
end
