#!/usr/bin/env python
# coding: utf-8

# To work in directories
import os

# To get the time
import time

# To handeĺe the telegram-bot
import telegram

# To handle warnings
import warnings

# To do linear algebra
import numpy as np

# To convert datetimes
from datetime import datetime

# To create plots
import matplotlib.pyplot as plt

# To connect to InfluxDB
from influxdb_client import InfluxDBClient
from influxdb_client.client.warnings import MissingPivotFunction

# To suppress some warings
warnings.simplefilter("ignore", MissingPivotFunction)

# API-Token to use your telegram-bot
telegram_token = 'TELEGRAM_TOKEN'

# ChatID to your prefered (group-)chat
telegram_chat_id = 'TELEGRAM_CHAT_ID'

# Connection to InfluxDB
host = 'INFLUXDB_IP:INFLUXDB_PORT'
org = 'INFLUXDB_ORG'
influx_token = 'INFLUXDB_TOKEN'

# Path to store the image
path_image = 'IMAGEPATH.png'

# Create a connection to the InfluxDB instance
client = InfluxDBClient(url=host, token=influx_token, org=org)

# Restrictions for the data
start = '-1440m'
group = '1m'

# Query to get the data
query = ' from(bucket: "keller")  |> range(start: {})  |> filter(fn: (r) => r["_measurement"] == "Keller_sensor")  |> filter(fn: (r) => r["_field"] == "in_hum" or r["_field"] == "in_temp" or r["_field"] == "in_water" or r["_field"] == "out_hum" or r["_field"] == "out_temp" or r["_field"] == "out_water" or r["_field"] == "ventilation")  |> aggregateWindow(every: {}, fn: mean, createEmpty: false)  |> yield(name: "mean")'.format(start, group)

# Get the data from InfluxDB
df = client.query_api().query_data_frame(query)
df = df.pivot(index='_time',columns='_field', values='_value')

# Create the plots for the daily update
fig, axes = plt.subplots(nrows=4, ncols=1, figsize=(12,15))

df[['in_water', 'out_water']].plot(ax=axes[0], title='Absolute Lufteuchtigkeit', style='.', color = ['darkorange', 'blue'])
df[['in_hum', 'out_hum']].plot(ax=axes[1], title='Relative Luftfeuchtigkeit', style='.', color = ['darkorange', 'blue'])
df[['in_temp', 'out_temp']].plot(ax=axes[2], title='Temperatur', style='.', color = ['darkorange', 'blue'])
df[['ventilation']].plot(ax=axes[3], title='Belüftung: An/Aus', style='.', color = ['darkorange', 'blue'])

legend = ['Innen', 'Außen']
for ax in axes[:-1]:
    ax.legend(legend)
    ax.grid()
    ax.set_xlabel('')
    
axes[0].set_ylabel('g/m³')
axes[1].set_ylabel('%')
axes[2].set_ylabel('°C')

axes[3].legend(['Belüftung'])
axes[3].grid()
axes[3].set_ylabel('An/Aus')
axes[3].set_ylim([-0.25,1.25])
    
plt.tight_layout()
plt.savefig(path_image)

# Open the image and send it
bot.send_photo(telegram_chat_id, photo=open(path_image, 'rb'))