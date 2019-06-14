class SensorsController < ApplicationController
  before_action :set_sensor, only: [:show, :edit, :update, :destroy]
  require 'date'
  # GET /sensors
  # GET /sensors.json
  def index
    @sensors = Sensor.all
    @sensors_inputs = SensorsInputs.all
  end

  # GET /sensors/1
  # GET /sensors/1.json
  def show
  end

  # GET /sensors/new
  def new
    @sensor = Sensor.new
    @sensor.save
  end

  # GET /sensors/1/edit
  def edit
  end
  # GET /sensors/test

  # POST /add_cycle
  # Adds one to cycle for particular sensor
  def add_cycle
    result = Sensor.where(id: params[:id])
    #If sensor exists in table, then just add one to cycles
    if(result.exists?)
      sensor = result.first
      sensor.cycles = sensor.cycles + 1
      sensor.save
    else
      #If sensor does not exist, add it to table
      sensor = Sensor.new(id: params[:id], cycles: 1)
      sensor.save
    end
  end
  # curl -d "{"id": 12, "inputs": [{"input_id": 1, "value": 1}]}" -H "Content-Type: application/json" -X POST http://localhost:3000/input
  #JSON := {"id": 12, "inputs": [{"input_id": 1, "value": 1},{"input_id": 1, value: 1}]}
  # POST /input
  # Receives JSON input from arduino and parses that input
  # Format of JSON is as follows
  ###{id: sensor_id, inputs: [{input_id: input_id1, value: value1},{input_id: input_id2, value: value2},...]}
  def input
    #Try and find the sensor the request came from
    sensor_id = params[:id]
    sensor_q = Sensor.where(id: sensor_id)
    if(!sensor_q.exists?)
      #If sensor doesn't exist, create new sensor
      sensor = Sensor.new(id: sensor_id)
      sensor.save
    end
    inputs = params[:inputs]
    #Iterate through all inputs received and save them in sensors_inputs
    inputs.each do |input|
      input_id = input[:input_id]
      #Check if input exists
      if(Input.where(id: input_id).exists?)
        puts "fire"
        SensorsInputs.new(sensor_id: sensor_id, input_id: input_id, value: input[:value], time: Time.current).save
      else
        puts "WARNING, tried to write to input id that doesnt exist. Input id: #{input_id}"
      end
    end
  end

  # GET /register-device
  # Creates new sensor and returns it as a JSON
  def register
    sensor = Sensor.new(cycles: 0)
    sensor.save
    puts "Registered Device with id #{sensor.id}"
    render json: {"id": "#{sensor.id}"}
  end
  # POST /sensors
  # POST /sensors.json
  def create
    @sensor = Sensor.new(sensor_params)

    respond_to do |format|
      if @sensor.save
        format.html { redirect_to @sensor, notice: 'Sensor was successfully created.' }
        format.json { render :show, status: :created, location: @sensor }
      else
        format.html { render :new }
        format.json { render json: @sensor.errors, status: :unprocessable_entity }
      end
    end
  end

  # PATCH/PUT /sensors/1
  # PATCH/PUT /sensors/1.json
  def update
    respond_to do |format|
      if @sensor.update(sensor_params)
        format.html { redirect_to @sensor, notice: 'Sensor was successfully updated.' }
        format.json { render :show, status: :ok, location: @sensor }
      else
        format.html { render :edit }
        format.json { render json: @sensor.errors, status: :unprocessable_entity }
      end
    end
  end

  # DELETE /sensors/1
  # DELETE /sensors/1.json
  def destroy
    @sensor.destroy
    respond_to do |format|
      format.html { redirect_to sensors_url, notice: 'Sensor was successfully destroyed.' }
      format.json { head :no_content }
    end
  end

  private
  # Use callbacks to share common setup or constraints between actions.
  def set_sensor
    @sensor = Sensor.find(params[:id])
  end

  # Never trust parameters from the scary internet, only allow the white list through.
  def sensor_params
    params.fetch(:sensor, {})
  end
end
