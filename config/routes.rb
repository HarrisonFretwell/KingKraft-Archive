Rails.application.routes.draw do
  resources :inputs
  devise_for :users
  get '/', to: 'sensors#index'
  get '/register-device', to: 'sensors#register'
  post '/add-cycle', to: 'sensors#add_cycle'
  post '/input', to: 'sensors#input'
  resources :sensors
end
