# Requires pip install matplotlib && pip install pyqt5

import matplotlib.pyplot as plt
import csv
import pathlib

FILE_DIR = pathlib.Path(__file__).parent.resolve()
def plot_file(file_name: str, title: str):
    t = []
    x = []
    vx = []
    
    plt.figure()
    with open(file_name,'r') as csvfile:
        lines = csv.reader(csvfile, delimiter=',')
        next(lines) # Skip first line
        for row in lines:
            t.append(float(row[0]))
            x.append(float(row[1]))
            vx.append(float(row[2]))

    plt.plot(t, x, color = 'g', linestyle = 'dashed',
            marker = 'o',label = title)
    
    plt.xticks(rotation = 25)
    plt.xlabel('Time (s)')
    plt.ylabel('Position (m)')
    plt.title(title, fontsize = 20)
    plt.grid()
    plt.legend()

plot_file(FILE_DIR/"spring.csv", 'Spring Position')
plot_file(FILE_DIR/"spring_rk45.csv", 'Spring Position')
plt.show()