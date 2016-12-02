from flask import Flask, render_template
from flask_socketio import SocketIO, emit
from Adafruit_BME280 import *

sensor = BME280(mode=BME280_OSAMPLE_8)

app = Flask(__name__)
app.config['SECRET_KEY'] = 'lightnet'
socketio = SocketIO(app)

@socketio.on('update')
def handle_update():
	print("Received update request\n")
	temp = "{0:0.3f}".format(sensor.read_temperature())
	pascals = "{0:0.2f}".format(sensor.read_pressure())
	humidity = "{0:0.2f}".format(sensor.read_humidity())
	print("Sending update\n")
	emit('new data', (temp, pascals, humidity))

@app.route("/")
def hello():
	return app.send_static_file('main.html')

@app.route("/static/socket.io.js")
def sock():
	return app.send_static_file('socket.io.js')

@app.route("/static/jquery.js")
def jq():
	return app.send_static_file('jquery.js')
if __name__ == '__main__':
	socketio.run(app)


