Rails.application.routes.draw do
  get '/', to: 'sensors#index'
  get '/register-device', to: 'sensors#register'
  post '/add-cycle', to: 'sensors#add_cycle'
  resources :sensors
end
