#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed May 31 08:49:49 2023

@author: bene
"""
import numpy as np
import matplotlib.pyplot as plt
import tifffile as tif
import serial
import time
import serial.tools.list_ports
from PIL import Image
import base64
import io
import serial.tools.list_ports

def connect_to_usb_device(manufacturer):
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if port.manufacturer == manufacturer:
            try:
                ser = serial.Serial(port.device, baudrate=2000000, timeout=1)
                print(f"Connected to device: {port.description}")
                return ser
            except serial.SerialException:
                print(f"Failed to connect to device: {port.description}")
    print("No matching USB device found.")
    return None

# Specify the manufacturer to connect to
manufacturer = 'Espressif'

# Connect to the USB device
serialdevice = connect_to_usb_device(manufacturer)

# Use the serial_connection object to interact with the device
if serialdevice:
    
    
    message = ""
    imageString = ""
    while True:
      try:
          #read image and decode
          imageB64 = serialdevice.readline()
          #imageB64 = str(imageB64).split("+++++")[-1].split("----")[0]
          image = np.array(Image.open(io.BytesIO(base64.b64decode(imageB64.decode()))))
          
          #plt.imshow(image), plt.show()
          #tif.imsave("test_stack_esp32.tif", image, append=True)
      except Exception as e:
        print(e)

        