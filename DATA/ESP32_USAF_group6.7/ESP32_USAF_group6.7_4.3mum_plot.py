#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun May  7 03:22:49 2023

@author: bene
"""

import matplotlib.pyplot as plt
import numpy as np

# Generate some sample data
dPix = 2 # effective pixelsize: 2µm

y1 = np.array((0, # x-direction
        10.643,
        5.435,
        63.525,
        140.6,
        88.668,
        17.95,
        109.708,
        156.231,
        59.364,
        41.861,
        127.056,
        141.631,
        47.617, 
        6.979,
        4)) 
y2 = np.array((1.389, # y-direction
        0,
        2.963,
        62.37,
        108.665,
        89.787,
        50.072,
        61.71,
        99.587,
        66.193,
        37.778,
        94.01,
        130.28,
        29.195,
        4.486,
        0.556))

x = dPix*np.linspace(0, y1.shape[0]-1, y1.shape[0])

# Create the plot
fig, ax = plt.subplots()

# Set the line colors and styles
ax.plot(x, y1, label='Grating X', color='red', linestyle='solid')
ax.plot(x, y2, label='Grating Y', color='blue', linestyle='dashed')

# Set the title and axis labels
ax.set_title('USAF Chart Group 6,7')
ax.set_xlabel('pixelnumbers ')
ax.set_ylabel('Intensity [A.U.]')

# Customize the tick marks and axis scales
ax.set_xticks(np.arange(0, np.max(x)+2, 5))
ax.set_yticks(np.arange(-1, 1.1, 0.5))
ax.set_yscale('linear')


# Add a legend and gridlines
ax.legend()
ax.grid(True)

# Save the plot as a PDF file for publication
plt.savefig('ESP32_USAF_group6.7_4.3mum_y_direction.png', dpi=300, format='pdf', bbox_inches='tight')
