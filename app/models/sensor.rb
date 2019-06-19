class Sensor < ApplicationRecord
  def get_bed_data(period = 24.hours, resolution = 10.minutes)
    puts "Start time: #{Time.now}"
    records = SensorsInputs.where(sensor_id: id, time: (DateTime.now - 24.hours)..(DateTime.now))
    a = DateTime.now - period
    b = DateTime.now
    #This will be bugged as I should look at the last reading before the interval
    most_recent = records.first
    data = []
    count = records.where(time: a..(DateTime.now - 24.hours)).count
    begin
      a += resolution
      current_records = SensorsInputs.where(time: ((DateTime.now - 24.hours)..a))
      if(current_records.count > count)
        count = current_records.count
        most_recent = current_records.order("time").last
      end
      data << [a,most_recent.value]
    end while a < b
    return data
  end

  def current_status
    return SensorsInputs.where(sensor_id: id).order("time").last.value
  end
end
