# fetch_and_plot.py

import requests
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from datetime import datetime

# ESP32 web server URL
esp32_ip = "192.168.4.1"
esp32_url = f"http://{esp32_ip}/data"

timestamps = []
temperatures = []
pressures = []
humidities = []


start_time = None

fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 10))
plt.tight_layout(pad=4.0)
fig.suptitle('Sensor Data', fontsize=16)


def update(frame):
    global timestamps, temperatures, pressures, humidities, start_time
    try:
        response = requests.get(esp32_url, timeout=2)
        if response.status_code == 200:
            data = response.json()


            timestamps.clear()
            temperatures.clear()
            pressures.clear()
            humidities.clear()

            for reading in data:
                timestamp_str = reading.get("timestamp", "")

                if isinstance(timestamp_str, str):
                    if timestamp_str.startswith("ms:"):

                        try:
                            ms = float(timestamp_str[3:])
                            timestamp_sec = ms / 1000.0
                        except ValueError:
                            timestamp_sec = 0.0
                    else:

                        try:
                            dt = datetime.strptime(timestamp_str, "%Y-%m-%d %H:%M:%S")
                            timestamp_sec = time.mktime(dt.timetuple())
                            if start_time is None:
                                start_time = timestamp_sec

                            timestamp_sec = timestamp_sec - start_time
                        except ValueError:

                            timestamp_sec = 0.0
                else:

                    try:
                        timestamp_sec = float(timestamp_str) / 1000.0
                    except (ValueError, TypeError):
                        timestamp_sec = 0.0

                timestamps.append(timestamp_sec)

                #append sensor readings, handling None values
                temp = reading.get("temperature", None)
                press = reading.get("pressure", None)
                hum = reading.get("humidity", None)

                temperatures.append(temp if temp is not None else float('nan'))
                pressures.append(press if press is not None else float('nan'))
                humidities.append(hum if hum is not None else float('nan'))

            #clear previous plots
            ax1.cla()
            ax2.cla()
            ax3.cla()

            #plot for temperature
            ax1.plot(timestamps, temperatures, label='Temperature (°C)', color='red', marker='o')
            ax1.set_ylabel('Temperature (°C)')
            ax1.legend(loc='upper left')
            ax1.grid(True)

            #plot for pressure
            ax2.plot(timestamps, pressures, label='Pressure (hPa)', color='blue', marker='o')
            ax2.set_ylabel('Pressure (hPa)')
            ax2.legend(loc='upper left')
            ax2.grid(True)

            #plot for humidity
            ax3.plot(timestamps, humidities, label='Humidity (%)', color='green', marker='o')
            ax3.set_ylabel('Humidity (%)')
            ax3.legend(loc='upper left')
            ax3.grid(True)
            ax3.set_xlabel('Millis/1000')

            for ax in [ax1, ax2, ax3]:
                if timestamps:
                    ax.set_xlim(left=max(0, timestamps[-1] - 20), right=timestamps[-1] + 1)
                ax.tick_params(axis='x', rotation=45)

            plt.tight_layout(pad=4.0)
        else:
            print(f"Failed to fetch data. Status code: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"Error fetching data: {e}")

ani = FuncAnimation(fig, update, interval=5000)

plt.show()
