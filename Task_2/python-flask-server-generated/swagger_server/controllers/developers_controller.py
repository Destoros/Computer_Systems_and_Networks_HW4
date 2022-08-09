import connexion
import six

from swagger_server.models.temperature_item import TemperatureItem  # noqa: E501
from swagger_server import util


def read_temperature():  # noqa: E501
    """returns temperature reading of sensor on raspi

    You can get an actual reading of the current temperature from the sensor connected to the raspi  # noqa: E501


    :rtype: TemperatureItem
    """
    sensor = '/sys/bus/w1/devices/28-00000b807cfa/w1_slave' #change to our device id
    """Aus dem Systembus lese ich die Temperatur der DS18B20 aus."""
    f = open(sensor, 'r')
    lines = f.readlines()
    f.close()
    lines = lines[1]
    lines = lines[29:]
    temp = int(lines)/1000

    return temp

