class Sensor < ApplicationRecord
  def get_24
    return SensorsInputs.where(time: (Time.now - 24.hours)..Time.now, sensor_id: id)
  end
end
