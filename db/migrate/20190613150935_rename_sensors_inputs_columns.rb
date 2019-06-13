class RenameSensorsInputsColumns < ActiveRecord::Migration[5.2]
  def change
    rename_column :sensors_inputs, :sensors_id, :sensor_id
    rename_column :sensors_inputs, :inputs_id, :input_id
  end
end
