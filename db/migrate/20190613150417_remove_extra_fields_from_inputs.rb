class RemoveExtraFieldsFromInputs < ActiveRecord::Migration[5.2]
  def change
    def up
      remove_column :inputs, :input_id
      remove_column :inputs, :primary_key
    end

    def down
      remove_column :inputs, :input_id, :int
      remove_column :inputs, :primary_key, :int
    end
  end
end
