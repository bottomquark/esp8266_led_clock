#!/usr/bin/python

from flask import Flask, render_template, request, flash, redirect
import paho.mqtt.client as mqtt
import sys
from datetime import datetime
import sqlite3
import os

targetdir="/var/www/temps/"
db_filename = os.path.dirname(os.path.realpath(__file__)) + "/settings.db"

app = Flask(__name__)
mqttc = mqtt.Client( protocol='MQTTv31')

messages = {}
colorvalues = [("Stunde", "hourhand","FF0000"),
	    ("Minute", "minutehand", "33EE00"),
	    ("Sekunde", "secondhand", "FFEE00"),
	    ("5Minuten", "fiveminutesections", "000045"),
	    ("Viertelstunden", "quartersections", "005C4A")]

user_brightness = 255 
min_brightness = 7
brightness_override = False
prev_brightness = 0 
lastOutsideTemp = 0

@app.route('/')
def index():

  global brightness_override
  global min_brightness
  global user_brightness

  for idx,element in enumerate(colorvalues):
    hexvalue = request.args.get(element[1])
    if hexvalue:
      #replace with user selection
      temp = list(colorvalues[idx])
      temp[2] = hexvalue
      colorvalues[idx] = tuple(temp)

      #convert and publish to MQTT
      rgbvalue = hex_to_rgb(hexvalue)
      rgbvalue = ",".join(map(str,rgbvalue))
      mqttc.publish("clock/in/leds/" + element[1], rgbvalue)

  u_b = request.args.get("brightness")
  if u_b:
    user_brightness = int(u_b)
    mqttc.publish("clock/in/leds/brightness", str(user_brightness))
  
  b_o = request.args.get("brightness_override")
  if b_o == "override":
    brightness_override = True
  elif user_brightness:
    brightness_override = False 
  
  m_b = request.args.get("min_brightness")
  if m_b:
    min_brightness = int(m_b)

  #store all values to the DB
  with sqlite3.connect(db_filename) as conn:
    for e in colorvalues:
      print("inserting into db: " + str((e[1], e[2])))
      conn.execute("insert or replace into settings (key, value) values(?, ?)", (e[1], e[2],))

    print("inserting into db: " + str(("user_brightness", str(user_brightness),)))
    conn.execute("insert or replace into settings (key, value) values(?, ?)", ("user_brightness", str(user_brightness),))
    print("inserting into db: " + str(("brightness_override", str(brightness_override),)))
    conn.execute("insert or replace into settings (key, value) values(?, ?)", ("brightness_override", str(brightness_override),))
    print("inserting into db: " + str(("min_brightness", str(min_brightness),)))
    conn.execute("insert or replace into settings (key, value) values(?, ?)", ("min_brightness", str(min_brightness),))
    conn.commit()

  if len(request.args) > 0:
    return redirect(request.path)

  return render_template('clocktemplate.html', colorvalues=colorvalues, messages=messages, brightness=user_brightness, brightness_override=brightness_override, min_brightness=min_brightness)

def publish_all_colors():
  for element in colorvalues:
    rgbvalue = hex_to_rgb(element[2])
    rgbvalue = ",".join(map(str,rgbvalue))
    mqttc.publish("clock/in/leds/" + element[1], rgbvalue)

def on_message(mqttc, obj, msg):

  global brightness_override
  global lastOutsideTemp

  split = msg.topic.split('/')
  subtopic = split[0]
  value = str(msg.payload)

  messages[msg.topic] = value
  if subtopic == "motion":
    node = split[1]
    today = datetime.today()
    line = '{:%Y-%m-%d_%H:%M:%S} {}'.format(today, value)
    with open(targetdir + 'motion_' + node + '.txt', 'a') as the_file:
      the_file.write(line + '\n')

  if subtopic == "temps":
    node = split[1]
    unit = split[2]
    if node == "node4" and unit == "temp" and value != "nan":
      lastOutsideTemp = value

    today = datetime.today()
    line = '{:%Y-%m-%d_%H:%M:%S} {}'.format(today, value)
    with open(targetdir + unit + '_' + node + '.txt', 'a') as the_file:
      the_file.write(line + '\n')

  if msg.topic == "clock/time/out":
    today = datetime.today()

    #check if the time is correct and set it if not so
    rec_dt = datetime.strptime(value, "%Y-%m-%d_%H:%M:%S")
    dt_diff = rec_dt - today
    if dt_diff.total_seconds() > 2 or dt_diff.total_seconds() < -2:
      print("{:%Y-%m-%d_%H:%M:%S}".format(today) + ": republishing time. Time diff: " + str(dt_diff.total_seconds()))
      mqttc.publish("clock/in/time", "{:%Y-%m-%d_%H:%M:%S}".format(today))
      publish_all_colors()

  elif msg.topic == "temps/clock/temp":
#    today = datetime.today()
#    if float(value) < -50.:
#      return
#    line = '{:%Y-%m-%d_%H:%M:%S} {}'.format(today, value)
#    with open(targetdir + 'temp_clock.txt', 'a') as the_file:
#      the_file.write(line + '\n')
#
    publish_temps(value)

  elif msg.topic == "clock/light":
    if not brightness_override:
      publish_brightness(int(value))

    node = split[1]
    today = datetime.today()
    line = '{:%Y-%m-%d_%H:%M:%S} {}'.format(today, value)
    with open(targetdir + 'brightness_clock.txt', 'a') as the_file:
      the_file.write(line + '\n')


def publish_temps(clockTemp):

  global lastOutsideTemp
#  with open(targetdir + "temp_node4.txt", "rb") as f:
#    for line in f:
#      pass
#    splitline = line.split()
#    if len(splitline) == 2:
#      lastOutsideTemp = splitline[1]

  clkTmp = float(clockTemp)

  mqttc.publish("clock/in/display", "{0:.1f}".format(round(clkTmp,2)) + " " + str(lastOutsideTemp) )
  
def publish_brightness(lightSensorValue):

  global prev_brightness
  global min_brightness
  global user_brightness

  #linear dimming
  # LED value for "fully on"
  LED_MAX = 255
  DISPLAY_MAX = 255
  # LED value for "still visible"
  LED_MIN = min_brightness
  DISPLAY_MIN = 0
  # max value from LDR (fully dark). min value for LDR is assumed 0 (bright)
  LDR_MAX = 960
  led_brightness = int(LED_MAX - (int(lightSensorValue)*(LED_MAX-LED_MIN)/LDR_MAX))
  display_brightness = int(DISPLAY_MAX - (int(lightSensorValue)*(DISPLAY_MAX-DISPLAY_MIN)/LDR_MAX))


  if led_brightness < LED_MIN:
    led_brightness = LED_MIN
  if display_brightness < DISPLAY_MIN:
    display_brightness = DISPLAY_MIN

  mqttc.publish("clock/in/leds/brightness", str(led_brightness))

  # if brightness decreases suddenly, adjust the LEDs on next cycle 
  #if prev_brightness - led_brightness < 20:
  mqttc.publish("clock/in/display/brightness", str(display_brightness))

  user_brightness = led_brightness 
  prev_brightness = led_brightness


#http://stackoverflow.com/a/214657
def hex_to_rgb(value):
  """Return (red, green, blue) for the color given as #rrggbb."""
  value = value.lstrip('#')
  lv = len(value)
  return tuple(int(value[i:i + lv // 3], 16) for i in range(0, lv, lv // 3))

def on_connect(client, userdata, flags, rc):
  mqttc.subscribe("#")

def setupDb():
  db_is_new = not os.path.exists(db_filename)

  with sqlite3.connect(db_filename) as conn:
    if db_is_new:
      conn.execute("create table settings(key text primary key, value text);")

def restoreFromDb():
  global brightness_override
  global min_brightness 
  global user_brightness
  global colorvalues

  with sqlite3.connect(db_filename) as con:
    cur = con.cursor()    
    cur.execute('select value from settings where key = ?', ("user_brightness",))
    data = cur.fetchone()
    if data:
      try:
        user_brightness = int(data[0])
        print("restored user_brightness: " + str(user_brightness))
      except:
        print("user_brightness is no int but " + str(data[0]))

    cur.execute('select value from settings where key = ?', ("brightness_override",))
    data = cur.fetchone()
    if data:
      brightness_override = data[0] == "True"
      print("restored brightness_override: " + str(brightness_override))

    cur.execute('select value from settings where key = ?', ("min_brightness",))
    data = cur.fetchone()
    if data:
      try:
        min_brightness = int(data[0])
        print("restored min_brightness: " + str(min_brightness))
      except:
        print("min_brightness is no int but " + str(data[0]))
    
    for idx,element in enumerate(colorvalues):
      cur.execute("select value from settings where key = ?", (element[1],))
      data = cur.fetchone()
      if data:
	colorvalues[idx] = (element[0], element[1], data[0])
        print("restored colorvalues: " + str((element[0], element[1], data[0])))

if __name__ == '__main__':
  setupDb()
  restoreFromDb()
  mqttc.on_message = on_message
  mqttc.on_connect = on_connect
  mqttc.connect("127.0.0.1", 1883, 60)

  mqttc.loop_start()

  #app.run(debug=True, host='0.0.0.0')
  app.run(debug=False, host='0.0.0.0')

# vim:ai:cin:sts=2 sw=2 ft=python
