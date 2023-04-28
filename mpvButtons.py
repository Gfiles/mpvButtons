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
import paho.mqtt.client as mqtt #pip install paho-mqtt

def readConfig():
    settingsFile = os.path.join(cwd, "config.json")
    with open(settingsFile) as json_file:
        data = json.load(json_file)
    return data

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(btnTopic)

def on_message(client, userdata, msg):
    global player
    global btnStates
    global lastMessage
    playVideo = False
    
    message = int(msg.payload)
    print(f"{msg.topic} {message}")
    #print(f"player.poll(): {player.poll()}")
    
    if player.poll() != None:
        playVideo = True
    else:
        if lastMessage != message:
            player.kill()
            playVideo = True
            #print (f"btnStates: {i} and btnled: {btnLed[i]}")
            for i in range(numBtns):
                #print (f"btnStates: {btnStates[i]} and btnled: {btnLed[i]}")
                if btnStates[i] == False and btnLed[i] == 1:
                    msgToPub = f"{i}1"
                    #print(f"Video not Playing {msgToPub}")
                    mqttClient.publish(ledTopic, msgToPub, qos=1)
                    #print("Publish On")
                    btnStates[i] = True
    
    #print(f"playVideo: {playVideo}")
    if playVideo:    
        player = subprocess.Popen([mpvPlayer, "--fullscreen", "--no-osc", medias[message+1]], stdin=subprocess.PIPE)
        if btnLed[message] == 1:
            msgToPub = f"{message}0"
            mqttClient.publish(ledTopic, msgToPub, qos=1)
            #print(f"On_message received {msgToPub}")
            btnStates[message] = False
            #print("Published OFf")
        lastMessage = message
 
def on_publish(client,userdata,result):             #create function for callback
    print("data published")
    pass
            
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
mpvPlayer = config["mpvPlayer"]
mqttServer = config["mqttServer"]
mqttPort = config["mqttPort"]
btnTopic = config["btnTopic"]
ledTopic = config["ledTopic"]
useMqtt = config["useMqtt"]
btnLed = config["btnLed"]

for i in range(len(medias)):
    medias[i] = cwd + medias[i]

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
btnStates = [True for i in range(numBtns)]
print(medias[0])
mpvPlayer = cwd + mpvPlayer
lastMessage = 0
print(mpvPlayer)
#subprocess.Popen([cwd + '\mpv\mpv.exe', "--fullscreen", "--no-osc", "--no-audio", "--loop-playlist", medias[0]], stdin=subprocess.PIPE)
subprocess.Popen([mpvPlayer, "--loop-playlist", "--fullscreen", "--no-osc", medias[0]], stdin=subprocess.PIPE)
player = subprocess.Popen([mpvPlayer, "--fullscreen", "--no-osc", medias[1]], stdin=subprocess.PIPE)
btnStates[0] = False
print("First Run")

if useMqtt:
    mqttClient = mqtt.Client()
    mqttClient.on_connect = on_connect
    mqttClient.on_message = on_message
    mqttClient.on_publish = on_publish 

    mqttClient.connect(mqttServer, mqttPort, 60)
    mqttClient.loop_start()
    mqttClient.publish(ledTopic, "00", qos=1)
    SentMsg = False

print("Ready")
    
while True:
    playVideo = False
    # Check if video stopped playing
    #print(f"player.poll(): {player.poll()}")
    if player.poll() != None:
        playVideo = True
        for i in range(numBtns):
            if not btnStates[i] and btnLed[i]:
                msgToPub = f"{i}1"
                print(f"Video not Playing {msgToPub}")
                if useMqtt:
                    mqttClient.publish(ledTopic, msgToPub, qos=1)
                if uartOn:
                    ser.write(msgToPub.encode())
                btnStates[i] = True
    
    if uartOn:
        #Serial Read
        serialData = ser.readline()
        #print(serialData)
        strData = serialData.decode()
        #print(strData)
        if strData != "" and strData.isnumeric():
            i = int(strData[0])
            if player.poll() == None:
                if lastMessage != i:
                    player.kill()
                    playVideo = True
            
            if playVideo:   
                print("playing Video")
                player = subprocess.Popen([mpvPlayer, "--fullscreen", "--no-osc", medias[i]], stdin=subprocess.PIPE)
                if btnLed[i] == 1:
                    sendStr = f"{i}0" # button numner and 0 for Off
                    ser.write(sendStr.encode())
                    btnStates[i] = False
                lastMessage = i
    
    for i in range(len(keyPress)):
        if keyboard.is_pressed(keyPress[i]):
            if player.poll() == None:
                if lastMessage != i:
                    player.kill()
                    playVideo = True
            
            if playVideo:
                print("playing Video")
                player = subprocess.Popen([mpvPlayer, "--fullscreen", "--no-osc", medias[i+1]], stdin=subprocess.PIPE)
                lastMessage = i
                btnStates[i] = False