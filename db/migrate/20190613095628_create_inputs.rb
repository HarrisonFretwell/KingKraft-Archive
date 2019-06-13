class CreateInputs < ActiveRecord::Migration[5.2]
  def change
    create_table :inputs do |t|
      t.integer :input_id, :primary_key
      t.string :name, null: false
    end
  end
end
