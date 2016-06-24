# LUTCalc.py
# Computes a LUT for a 103JT Thermistor.
# Maps number 0-1023 to a temp.
# Prints output as a string to terminal, preformatted as a C array assignment 
#   to be entered directly into an arduino ino file.

import sys
# Define system params
IN_V = 5.0  # Potential divider input voltage
RT = 10000  # Fixed resistor value
V_Div = IN_V/1024

VArr = [0] * 1024
LUT = [0] * 1024

threshArr = [
        4.867619804,
        4.767116907,
        4.610894942,
        4.382868428,
        4.068380846,
        3.673740053,
        3.218738867,
        2.738579828,
        2.267908857,
        1.837644678,
        1.46568177,
        1.15709784,
        0.9096858639,
        0.7147754542,
        0.5622614716,
        0.4443158729,
        0.3530302889,
        0.2821732197,
        0.2530594032]

threshTemps = [
        -50.0,
        -40.0,
        -30.0,
        -20.0,
        -10.0,
        0.0,
        10.0,
        20.0,
        30.0,
        40.0,
        50.0,
        60.0,
        70.0,
        80.0,
        90.0,
        100.0,
        110.0,
        120.0,
        125.0]

threshIndex = 0;

for i in range(0, 1024): 
    VArr[i] = i * V_Div

# Iterate over all possible voltages, check against thresholds
for i in range(0, 1024):
    print(i)
    for j in range(0, len(threshTemps)):
        if(j != len(threshTemps)-1 and j != 0):    # Check every threshold
            if((VArr[i] > threshArr[j]) and (VArr[i] <= threshArr[j-1])): # Threshold found
                vDiff = threshArr[j-1] - threshArr[j] # threshArr[j+1] is the lower one
                vFrac = (VArr[i] - threshArr[j])/vDiff
                tDiff = threshTemps[j-1] - threshTemps[j]
                # Calculate the temp
                print("SETTING")
                LUT[i] = threshTemps[j] + (vFrac * tDiff)
                break
        
        else:   # Only here for last case.
            if(j == 0 and VArr[i] > threshArr[j]):  # Check if it fits the highest voltage bracket
                print("Setting -50")
                LUT[i] = -50.0
                break
            elif(j == (len(threshTemps)-1) and VArr[i] <= threshArr[j-1]): # Check if it fits the lowest voltage bracket
                print("Setting 125")
                LUT[i] = 125.0
                break
            

print(LUT)
    




