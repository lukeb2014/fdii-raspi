import os
import asyncio
import uuid
import time
import serial
import matplotlib.pyplot as plt
import numpy as np
import math
import threading
import csv

from smbus2 import SMBus, i2c_msg
from bleak import BleakScanner
from bleak import BleakClient
from time import sleep # for sleep function
from smbus2 import SMBus, i2c_msg
from LEDControl import turnLEDColor


btName = 'Haptic Band'
#mode variables
testMode = 1



#LIDAR Global Variables
# I2C channel 1 is connected to the GPIO pins
channel = 1
# LiDAR 
address = 0x62 # default slave address
# write: 0xC4, read: 0xC5; effective addresses according to documentation
# acquisition command
ACQ_COMMAND = 0x00 # write 0x1 to this location to tell the LiDAR to receive a thing
STATUS_CMD = 0x01
WAIT_REG = 0x01 # continuously read this register until the LSB is 0x0
READ_REG = 0x8f # where to read two bytes from
block_size = 0x2 # bytes, or 16 bits


#Initiate Lidar Bus
LidarBus = SMBus(channel)

# create serial object with corresponding COM Port and open it
COM_Port = '/dev/ttyUSB0'
com_obj=serial.Serial(COM_Port)
#RadarMaxRange = 0=5m, 1=10m, 2=30m, 3=100m
radarMaxRange = 1
maxAngle = 5

def system_status():
    msg = i2c_msg.write(address, [STATUS_CMD, 0b10])
    LidarBus.i2c_rdwr(msg)
    while True:
        prep_msg = i2c_msg.write(address, [WAIT_REG])
        LidarBus.i2c_rdwr(prep_msg)
        read_msg = i2c_msg.read(address, 0x1) # control register has 1 byte
        LidarBus.i2c_rdwr(read_msg)
        if (list(read_msg)[0] > 0x0):
            break # we are ready to receive data
    prep_msg = i2c_msg.write(address, [READ_REG])
    LidarBus.i2c_rdwr(prep_msg)
    read_data_msg = i2c_msg.read(address, block_size)
    LidarBus.i2c_rdwr(read_data_msg)
    print(read_data_msg)

def cleanup_pins():
    # shutdown performs necessary operations to keep the Pi safe
    print("[INFO] Shutting down gpio pins")
    #GPIO.cleanup(self.output_channels) # cleanup output channels

def getLIDARdistance():
    # get_distance gets the LiDAR current reading
    msg = i2c_msg.write(address, [ACQ_COMMAND, 0b10]) # the registers are included in the read/write data
    LidarBus.i2c_rdwr(msg) # write to "ask for data" register
    while True:
        prep_msg = i2c_msg.write(address, [WAIT_REG])
        LidarBus.i2c_rdwr(prep_msg)
        read_msg = i2c_msg.read(address, 0x1) # control register has 1 byte
        LidarBus.i2c_rdwr(read_msg)
        if (list(read_msg)[0] > 0x0):
            break # we are ready to receive data
    prep_msg = i2c_msg.write(address, [READ_REG])
    LidarBus.i2c_rdwr(prep_msg)
    read_data_msg = i2c_msg.read(address, block_size)
    LidarBus.i2c_rdwr(read_data_msg)
    return list(read_data_msg)

def initiateRadar():
    com_obj.baudrate=115200
    com_obj.parity=serial.PARITY_EVEN
    com_obj.stopbits=serial.STOPBITS_ONE
    com_obj.bytesize=serial.EIGHTBITS
    
    # connect to sensor and set baudrate 
    payloadlength = (4).to_bytes(4, byteorder='little')
    value = (0).to_bytes(4, byteorder='little')
    header = bytes("INIT", 'utf-8')
    cmd_init = header+payloadlength+value
    com_obj.write(cmd_init)
    
    # get response
    response_init = com_obj.read(9)
    if response_init[8] != 0:
        print('Error during initialisation for K-LD7')
    
    # change max range to x
    value = (radarMaxRange).to_bytes(4, byteorder='little')
    header = bytes("RRAI", 'utf-8')
    cmd_frame = header+payloadlength+value
    com_obj.write(cmd_frame)

    # get response
    response_init = com_obj.read(9)
    if response_init[8] != 0:
        print('Error: Command not acknowledged')
        
def getRadarRange():
    # request next frame data
    payloadlength = (4).to_bytes(4, byteorder='little')
    TDAT = (8).to_bytes(4, byteorder='little')
    header = bytes("GNFD", 'utf-8')
    cmd_frame = header+payloadlength+TDAT
    com_obj.write(cmd_frame)
    
    # get acknowledge
    resp_frame = com_obj.read(9)
    if resp_frame[8] != 0:
        print('Error: Command not acknowledged')
    
    # get data 
    resp_frame = com_obj.read(8)

    # check if target detected by checking payloadlength and then get data
    target_detected = 0
    if resp_frame[4] > 1:
        target_detected = 1
        TDAT_Distance = (np.frombuffer(com_obj.read(2), dtype=np.uint16))*0.0328
        TDAT_Speed = np.frombuffer(com_obj.read(2), dtype=np.int16)/100
        TDAT_Angle =  math.radians(np.frombuffer(com_obj.read(2), dtype=np.int16)/100)
        TDAT_Magnitude = np.frombuffer(com_obj.read(2), dtype=np.uint16)
        
        if abs(TDAT_Angle) <= maxAngle:
            return TDAT_Distance


def determinePulseFrequency(lidarRange, RadarRange):
            
    rangeLoc = 1
    if (RadarRange <= 100) and (RadarRange > 90):
        rangeLoc = 1
    elif (RadarRange <= 90) and (RadarRange > 80):
        rangeLoc = 2
    elif (RadarRange <= 80) and (RadarRange >70):
        rangeLoc = 3
    elif (RadarRange <= 70) and (RadarRange > 60):
        rangeLoc = 4
    elif (RadarRange <= 60) and (RadarRange > 50):
        rangeLoc = 5
    elif (RadarRange <= 50) and (RadarRange > 40):
        rangeLoc = 6
    elif (RadarRange <= 40) and (RadarRange > 30):
        rangeLoc = 7
    elif (RadarRange <= 30) and (RadarRange > 20):
        rangeLoc = 8
    elif (RadarRange <= 20) and (RadarRange > 10):
        rangeLoc = 9
    elif (RadarRange <= 10) and (RadarRange > 0):
        rangeLoc = 10
    else:
        rangeLoc = 0
        
    pulseFint = int(2**rangeLoc)
    return pulseFint


async def connectBT():
    async with BleakScanner() as scanner:
        # time search runs for
        await asyncio.sleep(3.0)
        devices = await scanner.get_discovered_devices()
    # HB_add initialized to exists outside the for loop
    HB_add = 0
    for d in devices:
        # set to find device with SimpleSerialSocket as device name
        if d.name == btName:
            HB_add = d.address # devices MAC address
            print(HB_add)
    # HB_add is 0 if the device is not found
    if HB_add !=0:
        client = BleakClient(HB_add)
        # tries to connect 10
        for i in range(10):
            
            if await client.connect():
              print('connect')
              bluetoothConnected = 1
              turnLEDColor("blue")
              break
    else:
        print('Can\'t find device')
        bluetoothConnected = 0
        turnLEDColor("green")
        client = 0
        
    return client

    

async def run():
    #link Bluetooth
    #os.system('sudo systemctl restart bluetooth')
    #os.system('sudo bluetoothctl power on')
    turnLEDColor("red")
    
    bluetoothConnected = 0
    
    try: 
        client = await connectBT()
    except:
        print("Failed initital connection to BT") 
    if client != 0:
        bluetoothConnected = 1
    else:
        bluetoothConnected = 0
    
    
    #Check Lidar Status
    system_status()
    
    #initiate radar
    initiateRadar()
    
    char_uuid = uuid.UUID('0000fff1-0000-1000-8000-00805f9b34fb')
    print("Enter the number of cycles to run system for: ")
    x = int(input())
        
    
    i=0
    radarRange = 0
    #await threadedPeriodicPulse()
    period = 0
    
    
    print("Enter a name for a new log file")
    LogFileName = input()
    localtime = time.asctime( time.localtime(time.time()) )
    LogFileName = LogFileName + "  " + localtime
    with open("./LogFiles/" + LogFileName + ".csv", mode='w') as log:
        log_writer = csv.writer(log)
        log_writer.writerow(['Radar_ft', 'Lidar_ft'])
    
        
        
        while i < x:
            sleep(1)
            
            if bluetoothConnected == 0:
                try:
                    client = 0
                    client = await connectBT()
                except:
                    print("Failed initital connection to BT")
                    turnLEDColor("green")
                if client != 0:
                    bluetoothConnected = 1
                        
            
            #Holds on to old radar range if there is no change
            
            try:
                newRadarRange = getRadarRange()                        
            except:
                print("Radar Module ERROR")
                turnLEDColor("red")
            
            if newRadarRange != None:
                radarRange = newRadarRange
            
            try:
                lidarRangeRet = getLIDARdistance()
            except:
                print("LIDAR Module ERROR")
                rutnLEDColor("red")
            
            
            lidarRange = lidarRangeRet[1]*0.0328
            
            if i == 0:
                radarRange = lidarRange
            
            print("Radar range:")
            print('%.2f'%radarRange)
           
            print("Lidar range:")
            print('%.2f'%lidarRange)
            PulseFreq = determinePulseFrequency(lidarRange, radarRange)
            
            
            
            log_writer.writerow(['%.2f'%radarRange, '%.2f'%lidarRange])
            
            
            if PulseFreq != 0:
                period = 1/PulseFreq
            
            if bluetoothConnected == 1:
                msgOn = [2]
                msg = bytearray(msgOn)
                print("turn on")
                try:
                    await client.write_gatt_char(char_uuid, msg)
                except:
                    print("Lost connection with HB")
                    bluetoothConnected = 0
                    turnLEDColor("green")
                    
                        
                
                msgOff = [0]
                msg = bytearray(msgOff)
                print("turn off")
                try:
                    await client.write_gatt_char(char_uuid, msg)
                except:
                    print("Lost connection with HB")
                    bluetoothConnected = 0
                    turnLEDColor("green")
                    
     
            i += 1
        
        if bluetoothConnected == 1:
            try:
                await client.disconnect()
                print("disconnected from bluetooth")
            except:
                print("failed to disconnect")
        turnLEDColor("")


loop = asyncio.get_event_loop()
loop.run_until_complete(run())