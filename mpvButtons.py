#!/usr/bin/env python3

# Import socket module 
import json
import keyboard #pip install keyboard
import serial #pip install pyserial
import serial.tools.list_ports
import time
import os
import sys
import subprocess

def readConfig():
    settingsFile = os.path.join(cwd, "config.json")
    with open(settingsFile) as json_file:
        data = json.load(json_file)
    return data

# Get the current working 
# directory (CWD)
try:
    this_file = __file__
except NameError:
    this_file = sys.argv[0]
this_file = os.path.abspath(this_file)
if getattr(sys, 'frozen', False):
    cwd = os.path.dirname(sys.executable)
else:
    cwd = os.path.dirname(this_file)
    
print("Current working directory:", cwd)

# Read Config File
config = readConfig()
uartPort = config["uartPort"]
uartSpeed = config["uartSpeed"]
uartOn = config["uartOn"]
medias = config["medias"]
numBtns = config["numBtns"]
keyPress = config["keyPress"]

for i in range(len(medias)):
    medias[i] = cwd + "\\" + medias[i]

# Get COM Ports
if uartOn:
    comlist = serial.tools.list_ports.comports()
    comPortList = []
    if len(comlist) > 0:
        jsonPort = False
        for element in comlist:
            comPortList.append(element.device)
        if len(comlist) == 1:
            uartPort = comPortList[0]
        else:
            for item in comPortList:
                if uartPort == item:
                    jsonPort = True
                    break
            if not jsonPort:
                uartPort = comPortList[0]
        print(uartPort)
    ser = serial.Serial(uartPort, uartSpeed, timeout=1)
    ser.write("n1".encode())
    time.sleep(2)

#Variables
btnState = 0
print(medias[0])
#subprocess.Popen([cwd + '\mpv\mpv.exe', "--fullscreen", "--no-osc", "--no-audio", "--loop-playlist", medias[0]], stdin=subprocess.PIPE)
subprocess.Popen([cwd + '\mpv\mpv.exe', "--loop-playlist", "--fullscreen", "--no-osc", medias[0]], stdin=subprocess.PIPE)
player = subprocess.Popen([cwd + '\mpv\mpv.exe', "--fullscreen", "--no-osc", medias[1]], stdin=subprocess.PIPE)
print("Ready")
    
while 1:
    if uartOn:
        #Serial Read
        serialData = ser.readline()
        #print(serialData)
        strData = serialData.decode()
        #print(strData)
        if player.poll() == 0 and btnState == 0:
                sendStr = f"n{i+1}"
                ser.write(sendStr.encode())
                print(f"Stop Video")
                btnState = 1
        
        if strData != "" and strData[0].isnumeric():
            j = int(strData[0])
            if player.poll() == 0 and j == 1:
                print("playing Video")
                player = subprocess.Popen(['mpv\mpv.exe', "--fullscreen", "--no-osc", medias[j]], stdin=subprocess.PIPE)
                sendStr = f"f{j+1}"
                ser.write(sendStr.encode())
                btnState = 0
    else:
        if keyboard.is_pressed(keyPress) and player.poll() == 0:
            print("playing Video")
            player = subprocess.Popen([cwd + '\mpv\mpv.exe', "--fullscreen", "--no-osc", medias[1]], stdin=subprocess.PIPE)
