import numpy as np
import datetime
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
from array import array
import os
import sys
from scipy.optimize import curve_fit


def find_time(fname):
    """
    Extracts the start time of the gain measurement from its corresponding .sett file
    
    :param fname: Gain measurement settings file (*.sett) with full absolute path
    """

    with open(fname) as file:
        lines = file.readlines()
        for il in lines:
            if 'Start' in il:
                words = il.split()

                hour = words[2].split(':')[0]
                minute = words[2].split(':')[1]
                print (words[2])
                return 60*int(hour)+int(minute)



def draw_gain_run(_times, _adcs, _times_flat, _adcs_flat, _runs, _barcode):
    
    fig = plt.figure()
    ax = fig.add_subplot(111)    
    colors = ['black', 'b', 'r', 'g', 'c', 'm', 'y', 'k', 'w']
    #plt.rc('text', usetex=True)
    #plt.rc('font', family='serif')
    for i in range(len(_times)):
        ax.scatter(_times[i], _adcs[i], s=10, c=colors[i], marker="o", label=_runs[i])
    plt.legend(loc='upper left')
    plt.legend(frameon=False)
    ax.set_xlabel('time [min]')
    ax.set_ylabel('$\mu$ (ADC)')
    
    # fit curve to combined data
    if(True):
        init_vals = [40, 0.03, 60, 320]
        xmin = 30
        xmax = -1
        _times_flat_sliced = _times_flat[_times_flat>xmin]
        _adcs_flat_sliced = _adcs_flat[_times_flat>xmin]
        popt, pcov = curve_fit(chargup_func, _times_flat_sliced, _adcs_flat_sliced, p0=init_vals, bounds=(0, [1000,1,1000,1000]))
        plt.plot(_times_flat_sliced, chargup_func(_times_flat_sliced, *popt), 'r-', label='fit')
        x = np.mean(_times_flat)*4/3
        y = (np.amax(_adcs_flat)-np.amin(_adcs_flat))/3 + np.amin(_adcs_flat)
        print('Putting legend to: ({}, {})'.format(x,y))
        ax.text(x,y,'A/(1+exp(-B*(t-t0))) + A0\n  A={:.1f}\n  B={:.3f}\n  A0={:.1f}\n  Gain={:.1f}'.format(popt[0],popt[1],popt[3],chargup_func(10000, *popt)))
    ax.xaxis.set_major_locator(MultipleLocator(60))
    ax.xaxis.set_minor_locator(MultipleLocator(10))
    ax.yaxis.set_major_locator(MultipleLocator(50))
    ax.yaxis.set_minor_locator(MultipleLocator(5))
    #ax.grid(b=True, which='major', color='b', linestyle='--')
    ax.grid(b=True, which='minor', axis='y', color='g', linestyle='--')
    ax.grid(b=True, which='major', color='black', linestyle='-')

    plt.title(_barcode)
    plt.savefig('/home/vargyas/Desktop/{}_chargup.png'.format(_barcode))


def concatenate_data(_dirname, _start_times, _runs):
    """
    Concatenate individual gain mean data files into a single object
    
    :param _dirname: Gain measurement directory (full absolute path)
    :param _start_times: List of start times of each run in minutes
    :param _runs: List of runs
    """
    
    # load data files
    _data = []
    _data_flat = np.array([])
    for irun in _runs:
        tmp_arr =  np.loadtxt(_dirname+'/Run{}_mean.txt'.format(irun)) 
        _data.append(tmp_arr)
        _data_flat = np.append(_data_flat, tmp_arr)
    # fill corresponding time
    _time = []
    _time_flat = []
    for i in range(len(_data)):
        _time.append(array('f'))
        for j in range(len(_data[i])):
            _time[i].append(_start_times[i]+j*4)
            _time_flat = np.append(_time_flat, _start_times[i]+j*4)


    return _time, _data, _time_flat, _data_flat
    
def chargup_func(x, A, B, X0, C):
    return A/(1+np.exp(-B*(x-X0))) + C

####################
### MAIN PROGRAM ###
####################    
if len(sys.argv)<3:
    print("Usage: python3 draw_gain_runs.py [barcode=O2-G1-005] [list of runs] [time of filling before first measurement in minutes]")
    
##############################################################################
# barcode: barcode of the foil, e.g. O2-G1-005
# runs=[]: list of runs of a given foil, e.g. 278, 279, 280
# fill_time: duration of the 50l/h filling before the gain measurement in mins
# start_times=[]: starting time of each measurement
##############################################################################

name,barcode,*runs,fill_time = sys.argv

dirname = '/media/gridserv/gainScan/data/'
dirname = dirname+barcode
start_times = []

for irun in runs:
    start_times.append(find_time(dirname+'/GemQa2Run{}.sett'.format(irun)))

# RPI's clock is off, only difference is accurate...
# ...and add offset from filling
#print(start_times)
start_t = []
for i in range(len(start_times)):
    start_t.append( start_times[i] - start_times[0] + int(fill_time))
#print(start_t)

# create one 2D array with time and ADC data
times, ADCs, times_flat, ADCs_flat = concatenate_data(dirname, start_t, runs)
# draw the results
print(runs)
draw_gain_run(times, ADCs, times_flat, ADCs_flat, runs, barcode)    

print(len(times[0]), len(ADCs[0]))
