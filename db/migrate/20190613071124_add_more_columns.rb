class AddMoreColumns < ActiveRecord::Migration[5.2]
  def change
    add_column :sensors, :address, :text
  end
end
