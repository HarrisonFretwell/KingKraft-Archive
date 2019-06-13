class CreateSensorInput < ActiveRecord::Migration[5.2]
  def change
    create_table :sensors_inputs do |t|
      t.belongs_to :sensors, index: true
      t.belongs_to :inputs, index: true
      t.float :value
    end
  end
end
