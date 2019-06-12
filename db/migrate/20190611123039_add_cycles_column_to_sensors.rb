class AddCyclesColumnToSensors < ActiveRecord::Migration[5.2]
  def change
    add_column :sensors, :cycles, :int
  end
end
