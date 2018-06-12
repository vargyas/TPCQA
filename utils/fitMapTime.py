###################################################################
### This macro reads the output of generateMap.py, a preprocessed
### of the gain scan output and processes it further, fitting the
### the ADC values with a Gaussian to find the mean. It saves the
### numpy array output to a text file and a root file into a tree.
###
### Author: Vargyas, Marton
### Email: mvargyas@cern.ch
### 2016
###################################################################

# to run on all data (depreciated):
# for f in data/*root; do tmpName=${f##*/}; python fitMap.py ${tmpName/.root/} ; done

# set the extensions of all figures
EXT = ["png","pdf","eps"]

import numpy as np
import sys
import time
from scipy.stats import norm
from scipy.stats import chisquare
import matplotlib.pyplot as plt

#import matplotlib.cm as cm

# to simplify things, I rather save result as ROOT tree as well
#import ROOT
#from ROOT import TTree, TFile, TH1D
#from root_numpy import array2tree

def findADCIndex(x):
    """
    Find the ADC index from x value
    """

    if x >= 0 and x < 64:
        return 0
    elif x >= 64 and x < 112:
        return 1
    elif x >= 112 and x < 112+64:
        return 2
    elif x >= 112+64 and x < 112+64+48:
        return 3

colors = [601, 634, 417, 433, 635, 619, 603, 636, 719, 435]

def unpackLines(splitline):
    *values = splitline
    for iv in range(len(values)):
        values[iv] = int(values[iv])
    x = values[0]
    y = values[1]
    gbin = values[2]
    adc = []
    adc_ped = []
    for iadc in range(len(values[3:])):
        if iadc%5==0:
            adc.append( values[iadc] )
        else:
            adc_ped.append(values[iadc] )
    return x, y, gbin, adc, adc_ped


def processMap(inputFileName):
    """
    - find mean and average value in each (x,y) point by Gaussian fit
    - plots the results
    - save fit parameters in the map to a tree
    """

    # get pedestal mean values from their distribution

    with open(inputFileName) as file:
        lines = file.readLines()
        nl = len(lines)

        adc_pedestal = [0, 0, 0, 0]
        for il in range(nl):
            x, y, gbin, adc, adc_ped = unpackLines(lines[il].split())
            adc_pedestal.append(adc_ped)



    treePedestal = TTree("pedestal_tree","")
    treePedestal.ReadFile(inputFileName.replace('PREMAP','PEDESTAL'),"adc0:adc1:adc2:adc3")

    h = [0, 0, 0, 0]
    adc_pedestal = [0, 0, 0, 0]
    for i in range(4):
        h[i] = TH1D("h{}".format(i),"",38,2700,3004)
        treePedestal.Draw("adc{}>>h{}".format(i,i),"","goff")
        adc_pedestal[i] = h[i].GetMean()

    adc_pedestal_print = np.array(adc_pedestal)
    np.set_printoptions(precision=1)
    print(inputFileName, adc_pedestal_print)
    #print 'but they are reset, hahaha'
    #adc_pedestal = [2792, 2780, 2801, 2790]


    # control whether to plot Gaussian fits
    plotFitProcess = False

    start = time.clock()
    #outputFolderName = os.path.dirname(os.path.realpath(inputFileName))

    fit_array_tmp = list()

    # Read data from input file
    inFile = open(inputFileName, "r")
    lines = inFile.readlines()
    nl = len(lines)

    #start = time.clock()
    ic = 0
    for il in range(nl):
        words = lines[il].split()
        x = int(words[0])
        which_adc = findADCIndex(x)
        y = int(words[1])
        gbin = int(words[2])
        adc = words[3:]
        mu = -999
        std = -999
        nraw = len(adc)
        # Initialise to have all points
        fit_array_tmp.append((x, y, mu, std, nraw, 0.0, 0.0, 0))

        adcf = map(float, adc)
        adcc = []

        for iadc in adcf:
            adcc.append(iadc - adc_pedestal[which_adc])


        if len(adcc) > 2:

            mu = np.median(adcc)
            std= np.std(adcc)

            # ESTIMATE RESULTS AND SLICE THE ARRAY
            # OTHERWISE THE FIT PICKS UP ON OUTLIERS
            xmin = mu - 2 * std
            xmax = mu + 2 * std

            adcf_sliced = [_ix for _ix in adcc if xmin < _ix < xmax]
            # get rid of noise (important for adc0)
            # adcf_sliced = [_ix for _ix in adcf_sliced2 if _ix>50 ]




            if len(adcf_sliced) > 2:
                # Fit a normal distribution to the data:
                mu, std = norm.fit(adcf_sliced)
                chi2 = chisquare(adcf_sliced)

                #mu = np.median(adcf_sliced)
                #std = np.std(adcf_sliced)

                #if 55 < x < 63 and y > 140:
                #    print x, y, mu, std, adcf_sliced

                # redefine range for successful fit
                # if chi2[0]<3:
                #    xmin = mu-2*std
                #    xmax = mu+2*std

                # find occurence of values that are in +-2sigma off of the mean
                npeak = 0
                for iadc in adcf_sliced:
                    if xmax>iadc>xmin:
                        npeak+=1

                # 3rd order correction for nonlinearity 
                # (probably gas diffusion?)
                # A=1700.0
                # mu=mu*(1+mu*mu/A/A)

                fit_array_tmp[ic] = (x, y, mu, std, nraw, npeak, chi2[0], chi2[1])

                if x>56 and x<65 and y==100:
                    if plotFitProcess:
                        plt.clf()

                        count, bins, ignored = plt.hist(adcf_sliced, 30, normed=True)
                        plt.plot(bins, 1/(std * np.sqrt(2 * np.pi)) * np.exp( - (bins - mu)**2 / (2 * std**2) ), linewidth=2, color='r')
                        #p = norm.pdf(plot_x, mu, std)
                        #plt.plot(plot_x, p, 'k', linewidth=2)
                        title = "Fit results: mu = %.2f,  std = %.2f" % (mu, std)
                        plt.title(title)
                        plt.savefig("fig_fit/fit_X{}_Y{}.png".format(x, y))
                        #plt.show()

        ic += 1

    pass
    stop = time.clock()
    print('Generating map took [', stop - start, '] sec')

    fit_array = np.array(fit_array_tmp,
                         dtype=[('x', np.int),
                                ('y', np.int),
                                ('mean', np.float64),
                                ('sigma', np.float64),
                                ('nraw', np.float64),
                                ('npeak', np.float64),
                                ('chi2',np.float64),
                                ('pvalue',np.float64)
                                ])

    outputFileName = inputFileName.replace('PREMAP.txt', 'MAP.txt')
    print('saving to text file:', outputFileName)
    np.savetxt(outputFileName, fit_array,'%.0f')

    outputFileName = outputFileName.replace(".txt",".root")
    treeGain = array2tree(fit_array, name='gain_tree')
    print('writing file to:', outputFileName)
    outFile = TFile(outputFileName, 'RECREATE')
    outFile.cd()


    treePedestal.Write()
    treeGain.Write()
    outFile.Write()
    outFile.Close()



"""
M A I N   P R O G R A M
"""

# input is the output of generateMap.py
# example: python fitMap.py IROC_14_a/Run127/Run127PREMAP.txt

print("")
processMap(sys.argv[1])
print("")

