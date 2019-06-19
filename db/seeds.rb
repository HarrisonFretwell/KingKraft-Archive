# This file should contain all the record creation needed to seed the database with its default values.
# The data can then be loaded with the rails db:seed command (or created alongside the database with db:setup).
#
# Examples:
#
#   movies = Movie.create([{ name: 'Star Wars' }, { name: 'Lord of the Rings' }])
#   Character.create(name: 'Luke', movie: movies.first)
Input.new(id: 1, name: 'Accelerometer').save!;
Input.new(id: 2, name: 'Ultrasound').save!;
Input.new(id: 3, name: 'Pressure').save!;
Input.new(id: 4, name: 'GPS').save!;
