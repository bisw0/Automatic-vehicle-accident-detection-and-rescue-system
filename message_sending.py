#SMS Python Code
from boltiot import Sms
from boltiot import Bolt        # importing Bolt from boltiot module

import json
import time
import requests                 # for making HTTP requests

max_temp = 70
#scaled value to temperature for the purpose of simulation
#Engine temperature varies between 80 and 100 degree celcius.

max_vib = 100 #Pulse duration more than 10secs
#In Normal Conditions, the vehicle body vibrations last for 2 to 5 secs and with lesser intensity.

max_roll = 4  #value is proportional to the clockwise angle w.r.t ground.  1 roll = 9 degrees.
min_roll = -4 #value is proportional to the anti-clockwise angle

max_pitch = 4  #value is proportional to the clockwise angle w.r.t ground.  1 pitch = 9 degrees.
min_pitch = -4 #value is proportional to the anti-clockwise angle

# Messaging Configuration
SID = 'AC13b97b36fd7034a37d31aec8db8a3108' 
AUTH_TOKEN = 'ac2eefbad1c1d7fb6fc5861af3364a83' 
FROM_NUMBER = '+14325585238'
TO_NUMBER = '+917411603095'
API_KEY = 'bf3436b3-f7d0-4fce-a192-71a383764b7f'
DEVICE_ID = 'BOLT3263244'

mybolt = Bolt(API_KEY, DEVICE_ID)
sms = Sms(SID, AUTH_TOKEN, TO_NUMBER, FROM_NUMBER)
gps_str = "https://maps.google.com/maps?q="
response = mybolt.serialBegin('9600')

while True: 
    response = mybolt.isOnline()
    if(respon['value']=='online'):
        response = mybolt.serialWrite('GetSensorData')
        sensor_values=response['value'].split(',')#response['value'] = "vibration,roll,pitch,ir1,ir2,temperature"    
        response = mybolt.serialRead('10')
        vibration = int(sensor_values[0])
        roll = int(sensor_values[1])
        pitch = int(sensor_values[2])
        ir = int(sensor_values[0])+ int(sensor_values[0])
        if(vibration > max_vib) and (roll > max_roll or roll < min_roll or pitch > max_pitch or pitch < min_pitch): 
            response = mybolt.serialWrite('GetGPSData')
            response = mybolt.serialRead('10')
            gps_lat_long = response['value']
            sms.send_sms("Accident has Occured!!! \n Send help Immediately to the below location :\n"+gps_str+gps_lat_long)
    time.sleep(1000) #repeat the monitoring after one second