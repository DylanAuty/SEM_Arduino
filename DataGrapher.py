# DataGrapher.py
# Python script to stream live data from the serial port, and graph it
# Incoming data should be a .csv format.
# time, BMS_V_1, BMS_V_2, BMS_V_3, BMS_V_4, BMS_V_5, BMS_V_6, BMS_V_7, BMS_V_8, BMS_V_9, heading, speed, latitude, longitude, throttle_voltage
# 
# Dylan Auty, 2/7/2016

import serial
import numpy
import matplotlib.pyplot as plt
from time import sleep

from drawnow import *

timeArr = [0]
vTotalArr = [0]

def main(): 
    inData = serial.Serial('/dev/ttyACM0')
    plt.ion()
    BMSArr = [0] * 9
    sleep(0.2)  # Wait a bit for serial connection to stabilise

    while(1):
        while (inData.inWaiting() == 0):
            pass
        dataLine = inData.readline()
        dataArr = dataLine.split(",")  # Read in and split on comma delimiters
        if(len(dataArr) != 15): # Sanity check string
            continue
        
        # Sum the voltages from the inputs
        vTotal = 0.0
        try:
            for i in range(1, 9):
                vTotal += float(dataArr[i]);
            vTotalArr.append(vTotal)
        
            # Record and append timestamp
            timestamp = float(dataArr[0])
            print(dataLine)
            timeArr.append(timestamp)
        except:
            pass

        # Plot the figure and wait a tiny bit to stop drawnow crashing
        drawnow(drawFig)
        plt.pause(0.000001)
        
        if(float(timestamp - timeArr[0]) >= 15000): # 15 seconds max on the graph at once
            timeArr.pop(0)
            vTotalArr.pop(0)
        


def drawFig():
    plt.title('Supercapacitor Voltage/Time')
    plt.grid(True)
    plt.xlabel('Time (ms)')
    plt.ylabel('Supercapacitor Voltage (V)')
    plt.plot(timeArr, vTotalArr, 'rx-')
    plt.xlim(timeArr[0], (timeArr[-1]+200))
    plt.ylim(0, 50)

if __name__ == '__main__':
    main()



