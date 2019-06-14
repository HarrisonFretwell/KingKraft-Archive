class AddTimeColumnToSensorsInputs < ActiveRecord::Migration[5.2]
  def change
    add_column :sensors_inputs, :time, :timestamptz
  end
end
