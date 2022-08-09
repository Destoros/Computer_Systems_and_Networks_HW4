import time
import sys
import paho.mqtt.publish as publish

sensor = '/sys/bus/w1/devices/28-00000b813aea/w1_slave'

def readTempSensor(sensorName) :
    """Aus dem Systembus lese ich die Temperatur der DS18B20 aus."""
    f = open(sensorName, 'r')
    lines = f.readlines()
    f.close()
    return lines

def readTempLines(sensorName) :
    lines = readTempSensor(sensorName)
    while lines[0].strip()[-3:] != 'YES':
        time.sleep(0.2)
        lines = readTempSensor(sensorName)
    temperaturStr = lines[1].find('t=')
    if temperaturStr != -1 :
        tempData = lines[1][temperaturStr+2:]
        tempCelsius = float(tempData) / 1000.0
        tempKelvin = 273 + float(tempData) / 1000
        tempFahrenheit = float(tempData) / 1000 * 9.0 / 5.0 + 32.0
        return [tempCelsius, tempKelvin, tempFahrenheit]


MQTT_SERVER = "localhost"
MQTT_PATH = "test_channel"

my_msg = str(time.strftime('%H:%M:%S: ') + str(readTempLines(sensor)[0]) + str(" deg Celsius")).encode('ascii')
publish.single(MQTT_PATH, my_msg, hostname=MQTT_SERVER)