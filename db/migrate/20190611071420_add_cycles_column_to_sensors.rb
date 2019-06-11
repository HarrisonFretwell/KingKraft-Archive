class AddCyclesColumnToSensors < ActiveRecord::Migration[5.2]
  def change
    add_column :sensors, :cycles, :int
    add_column :sensors, :id, :primary_key
  end
end
