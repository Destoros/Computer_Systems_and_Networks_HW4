#!/usr/bin/python
import paho.mqtt.publish as publish
import time

MQTT_SERVER = "192.168.0.179"
MQTT_PATH = "test_channel"


while(1):
    sensor = '/sys/bus/w1/devices/28-00000b807cfa/w1_slave' #change to our device id
    f = open(sensor, 'r')
    lines = f.readlines()
    f.close()
    lines = lines[1]
    lines = lines[29:]
    temp = int(lines)/1000
    
    publish.single(MQTT_PATH, "Current Temperature [" + str(temp) + "] Grad C" , hostname=MQTT_SERVER)
    time.sleep(5)
