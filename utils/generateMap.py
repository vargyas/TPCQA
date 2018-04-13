###################################################################
### This macro reads the gain scan output (*.ebe) and generates
### an X-Y map from it, with the ADC values array at each point.
### fitMap.py then finds the mean ADC values for each point.
###
### Author: Vargyas, Marton
### Email: mvargyas@cern.ch
### 2016
###################################################################


# initial parameters (estimated based on past data)
###################################################################
init_mean = 440
init_width = 100
adc_pedestal = [2792, 2780, 2801, 2790]
###################################################################

from array import array
import numpy as np
import sys
import time

def getGlobalIndex(x, y):
    return 224*y + x

def getNextIndex(words, prev_index):
    """
    Reads x or y index and gets the next index
    (order is xl, yl, xr, yr)
    """

    index_len = int(words[prev_index])
    if index_len == 0:
        next_index = prev_index + 1
    else:
        next_index = prev_index + index_len + 1
    return next_index

def generateMap(fileName):
    """
    Collect the continuous ADC values
    """

    print 'generating map from: ', fileName

    aADC = []
    adc_pedestal = []

    for i in range(224 * 160):
        aADC.append(array('f'))

    with open(fileName) as f:
        lines = f.readlines()
        nl = len(lines)

        start = time.clock()
        for il in range(nl):

            xarrl = []
            yarrl = []
            xarrr = []
            yarrr = []
            words = lines[il].split()
            # get adc indices
            ixl_index = 3
            iyl_index = getNextIndex(words, ixl_index)
            ixr_index = getNextIndex(words, iyl_index)
            iyr_index = getNextIndex(words, ixr_index)
            iyr_index_up = getNextIndex(words, iyr_index)

            # collect adc indices:
            # collect x left
            for ixl in range(ixl_index + 1, iyl_index):
                xarrl.append(int(words[ixl]))
            # collect y left
            for iyl in range(iyl_index + 1, ixr_index):
                yarrl.append(int(words[iyl]))
            # collect x right
            for ixr in range(ixr_index + 1, iyr_index):
                xarrr.append(int(words[ixr]))
            # collect y right
            for iyr in range(iyr_index + 1, iyr_index_up):
                yarrr.append(int(words[iyr]))

            # it rejects empty channel arrays to save time,
            # then it rejects non-continuous channels
            if len(xarrl) > 0:
                xl = findClusterPart(xarrl)
            else:
                xl = -1
            if len(xarrr) > 0:
                xr = findClusterPart(xarrr)
            else:
                xr = -1
            if len(yarrl) > 0:
                yl = findClusterPart(yarrl)
            else:
                yl = -1
            if len(yarrr) > 0:
                yr = findClusterPart(yarrr)
            else:
                yr = -1

            # we accept only one cluster pair (x-y)
            # decide here which will be kept
            x, y = findCluster(xl, xr, yl, yr)

            # only fill if both x and y are valid clusters
            #if x < 0 or y < 0:
            #    continue

            # get ADC index and value
            adc_all = words[iyr_index_up + 1:iyr_index_up + 1 + 4]

            # loop over left and right
            for ix in range(2):
                if x[ix] < 0 or y[ix] < 0:
                    continue

                which_adc = findADCIndex(x[ix])

                adc_all = np.array(adc_all)
                adc_all = adc_all.astype(np.float)
                # for some reason half value is stored, correcting here
                adc_all[1] *= 2.

                # copy all ADC values except the triggered data
                adc_ped = adc_all.copy()
                adc_ped[which_adc] = -1.0
                adc_pedestal.append( adc_ped )

                adc = adc_all[which_adc]
                # subtract pedestal after the correction
                # not now, this will be done later, after pedestal estimation
                # adc = adc - adc_pedestal[which_adc]

                # also this correction in the awk file
                # adc[0] *= 1.4

                X = int(x[ix])
                Y = int(y[ix])
                gbin = getGlobalIndex(X, Y)
                aADC[gbin].append(adc)

        stop = time.clock()
        print '\nProcessing file took [', stop - start, '] sec'

        outFileName = fileName.replace('.ebe', 'PREMAP.txt')
        outFileName = outFileName.replace('GemQa2','')
        print 'Saving map to: {}'.format(outFileName)

        outF = open(outFileName, 'w+')
        for ix in range(224):
            for iy in range(160):

                gbin = getGlobalIndex(ix, iy)
                adc_str = ""
                if len(aADC[gbin])>1:
                    for iadc in aADC[gbin]:
                        adc_str+=str(iadc)+"\t"

                outF.writelines(["{:d}\t{:d}\t{:d}\t{:s}\n".format(ix, iy, getGlobalIndex(ix, iy), adc_str)])

        outFileName = outFileName.replace('PREMAP','PEDESTAL')
        print 'Saving pedestal to: {}'.format(outFileName)
        np.savetxt(outFileName, adc_pedestal, "%.0f")


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


def findClusterPart(arr):
    """
    Find cluster (=continuous array)
    from short array
    """

    mean = arr[0]
    # break if it is not continuous
    for i in range(len(arr) - 1):
        mean += arr[i + 1]
        #if arr[i]+1 != arr[i + 1]:
        # enable one broken channel (one can be missing)
        if arr[i] + 1 != arr[i + 1]:
            if arr[i] + 2 != arr[i + 1]:
               #if arr[i] + 3 != arr[i + 1]:
               return -1

    mean = float(mean) / float(len(arr))
    return mean

def findCluster(xl, xr, yl, yr):

    x=[-1, -1]
    y=[-1, -1]

    if xl>0:
        if yl>0:
            x[0]=xl; y[0]=yl
    if xr>0:
        if yr>0:
            x[1]=xr + 112; y[1]=yr

    return x, y




### M A I N   P R O G R A M
print ""
generateMap(sys.argv[1])
print ""
