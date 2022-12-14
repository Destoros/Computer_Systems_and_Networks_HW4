import connexion
import six
import time
import sys

from swagger_server import util

#import time, sys

# Systempfad zum den Sensor, weitere Systempfade könnten über ein Array
# oder weiteren Variablen hier hinzugefügt werden.
# 28-02161f5a48ee müsst ihr durch die eures Sensors ersetzen!
sensor = '/sys/bus/w1/devices/28-00000b813aea/w1_slave'

def readTempSensor(sensorName) :
    """Aus dem Systembus lese ich die Temperatur der DS18B20 aus."""
    f = open(sensorName, 'r')
    lines = f.readlines()
    f.close()
    return lines

def readTempLines(sensorName) :
    lines = readTempSensor(sensorName)
    # Solange nicht die Daten gelesen werden konnten, bin ich hier in einer Endlosschleife
    while lines[0].strip()[-3:] != 'YES':
        time.sleep(0.2)
        lines = readTempSensor(sensorName)
    temperaturStr = lines[1].find('t=')
    # Ich überprüfe ob die Temperatur gefunden wurde.
    if temperaturStr != -1 :
        tempData = lines[1][temperaturStr+2:]
        tempCelsius = float(tempData) / 1000.0
        tempKelvin = 273 + float(tempData) / 1000
        tempFahrenheit = float(tempData) / 1000 * 9.0 / 5.0 + 32.0
        # Rückgabe als Array - [0] tempCelsius => Celsius...
        return [tempCelsius, tempKelvin, tempFahrenheit]


def get_temperature():  # noqa: E501
    """Reads temperature from local sensor and returns value

    Reads temperature from local sensor and returns value # noqa: E501


    :rtype: str
    """
	#print("Temperatur um " + time.strftime('%H:%M:%S') +" drinnen: " + str(readTempLines(sensor)[0]) + " °C")
    return time.strftime('%H:%M:%S') + ": " + str(readTempLines(sensor)[0]) + " deg Celsius"
