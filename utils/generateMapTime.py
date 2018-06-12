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
import concurrent.futures
import os
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from matplotlib import gridspec
from mpl_toolkits.axes_grid1 import make_axes_locatable
from scipy.stats import norm
from scipy.stats import chisquare

def get_global_index(x, y):
    return 224*y + x

def get_next_index(words, prev_index):
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

#def getIndices(_words):


def unpack(*args):
    aa, aa, aa, *args, aa, adc0, adc1, adc2, adc3, aa, aa = args
    print(aa, args, adc0, adc1, adc2, adc3)


def generate_map_from_file(fileName):
    """
    Collect the continuous ADC values
    
    :param fileName: raw gain scan file (.ebe format)
    """
    
    nCPU = os.cpu_count()
    print('Your machine has {} cpus'.format(nCPU))
    print('Generating map from: ', fileName)

    with open(fileName) as file:
        lines = file.readlines()
        nl = len(lines)
        print("reading file done, has lines", nl)

        
        # slice file according to real time:
        time_unit = 4*60*1000000 # granularity is 4 minutes
        meas_time = 0
        counter = 0
        indices = []

        for it in range(len(lines)):
            meas_time += float( lines[it].split()[1] )
            if(counter * time_unit < meas_time):
                indices.append(it)
                counter += 1
        
        m, s = divmod(meas_time/1000000, 60)
        h, m = divmod(m, 60)
        print("total measurement time: {:.0f}:{:.0f} hours".format(h, m))
        print(indices)
        ntimes = len(indices)    

        # get runnumber
        dirname=os.path.dirname(fileName)
        runnumber=fileName.rsplit('.')[0].rsplit('Run')[1]
        basename=os.path.join(dirname,"Run{}".format(runnumber))
        
        futures = {}
        results = []
        with concurrent.futures.ProcessPoolExecutor(max_workers=nCPU-1) as executor:
            for i in range(ntimes-1):
                future = executor.submit(generate_map, i, fileName, basename, ntimes,  *lines[indices[i]:indices[i+1]])
                futures[future] = lines[indices[i]:indices[i+1]]

            for f in concurrent.futures.as_completed(futures):
                aADC = f.result()
                #adc = futures[f]
                results.append(aADC)

        results=np.array(results)
        results = results[results[:,0].argsort()]

        # draw results
        time  = results[:,0]
        mu    = results[:,1]
        nraw  = results[:,2]
        npeak = results[:,3]
        
        draw_mean(time, mu, ntimes, basename)
        draw_n(time, nraw, "nraw", ntimes, basename)
        draw_n(time, npeak, "npeak", ntimes, basename)
        
        # convert maps to gifs and clean up
        # append absolute path for unix
        # map_basename = "{}/{}_mean_map".format(os.path.abspath('.'), basename)
        map_basename = "{}_mean_map".format(basename)
        print('convert {}_*.png {}.gif'.format(map_basename, map_basename))
        os.system('convert {}_*.png {}.gif'.format(map_basename, map_basename))
        print('rm {}_*.png'.format(map_basename))
        os.system('rm {}_*.png'.format(map_basename))
        
    #outFileName = outFileName.replace('PREMAP','PEDESTAL')
    #print( 'Saving pedestal to: {}'.format(outFileName))
    #np.savetxt(outFileName, adc_pedestal, "%.0f")
    #print("Program took", toc, "[s] time")

def add_2_5_percent_lines(ax, mean):
    ax.axhline(mean, linestyle='--', color='k')
    ax.axhline(mean*0.98, linestyle=':', color='k')
    ax.axhline(mean*1.02, linestyle=':', color='k')
    ax.axhline(mean*0.95, linestyle='--', color='k')
    ax.axhline(mean*1.05, linestyle='--', color='k')
    
def draw_mean(time, mu, ntimes, basename):
    plt.figure()
    mu_mean = np.mean(mu)
    plt.plot(time*4, mu, marker='o')
    plt.xlim(0,ntimes*4)
    plt.gca().set_xlabel('time [min]')
    plt.gca().set_ylabel('$\mu$ (ADC)')
    """
    plt.gca().axhline(mu_mean, linestyle='--', color='k')
    plt.gca().axhline(mu_mean*0.95, linestyle=':', color='k')
    plt.gca().axhline(mu_mean*1.05, linestyle=':', color='k')
    plt.gca().axhline(mu_mean*0.9, linestyle='--', color='k')
    plt.gca().axhline(mu_mean*1.1, linestyle='--', color='k')
    """
    add_2_5_percent_lines(plt.gca(), mu_mean)
    #plt.show()
    plt.savefig("{}_mean.png".format(basename))
    
def draw_n(time, n, name, ntimes, basename):
    plt.figure()
    plt.plot(time*4, n, marker='o')
    plt.xlim(0,ntimes*4)
    plt.gca().set_xlabel('time [min]')
    plt.gca().set_ylabel(name)
    n_mean = np.mean(n)
    add_2_5_percent_lines(plt.gca(), n_mean)

    #plt.show()
    plt.savefig("{}_{}.png".format(basename,name))
    
def generate_map(itime, fileName, basename, ntimes, *lines):
    """
    Collect content of lines into a grid
    
    :param itime: index for file save
    :param fileName: output file name (passed so it would be similar as input file's name)
    :param lines: data to be processed
    """
    
    create_gif = True
    aADC = []
    adc_pedestal = []

    for i in range(224 * 160):
        aADC.append(array('f'))

    #print("this instance is", len(lines), "long")
    for il in range(len(lines)):

        xarrl = []
        yarrl = []
        xarrr = []
        yarrr = []
        words = lines[il].split()

        # get adc indices
        ixl_index = 3
        iyl_index = get_next_index(words, ixl_index)
        ixr_index = get_next_index(words, iyl_index)
        iyr_index = get_next_index(words, ixr_index)
        iyr_index_up = get_next_index(words, iyr_index)

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
        if len(xarrl) > 0: xl = find_cluster_part(xarrl)
        else: xl = -1
        if len(xarrr) > 0: xr = find_cluster_part(xarrr)
        else: xr = -1
        if len(yarrl) > 0: yl = find_cluster_part(yarrl)
        else: yl = -1
        if len(yarrr) > 0: yr = find_cluster_part(yarrr)
        else: yr = -1

        # we accept only one cluster pair (x-y)
        # decide here which will be kept
        x, y = find_cluster(xl, xr, yl, yr)

        # get ADC index and value
        adc_all = words[iyr_index_up + 1:iyr_index_up + 1 + 4]

        # loop over left and right
        for ix in range(2):
            if x[ix] < 0 or y[ix] < 0:
                continue

            which_adc = find_adc_index(x[ix])

            adc_all = np.array(adc_all)
            adc_all = adc_all.astype(np.float)
            # for some reason half value is stored, correcting here
            adc_all[1] *= 2.
            
            
            # 3rd order correction for nonlinearity 
            # (probably gas diffusion?)
            # A=1700.0
            # for i in range(len(adc_all)):
            #     adc_all[i]=adc_all[i]*(1+adc_all[i]*adc_all[i]/A/A)

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
            gbin = get_global_index(X, Y)
            aADC[gbin].append(adc)

    # get adc pedestal without the negative (-1) values
    adc_pedestal = np.array(adc_pedestal)
    adc_p = []
    for i in range(4): 
        dummy = adc_pedestal[:,i]
        adc_p.append(np.mean(dummy[dummy>0]))
        
    aADC = np.array(aADC)
    # loop over map
    ic = 0
    fit_array_tmp = list()
    for ix in range(224):
        for iy in range(160):
            gbin = get_global_index(ix, iy)   
            which_adc = find_adc_index(ix)
            adc = aADC[gbin]-adc_p[which_adc]
            mu = -1
            std = -1
            nraw = len(adc)
            
            # create dummy array, so the dimensions are right
            # will be changed after fit
            fit_array_tmp.append((x, y, mu, std, nraw, 0.0, 0.0, 0))
            
            if(len(adc)>10):
                mu = np.median(adc)
                std = np.std(adc)
    
                # estimate results and slice the array
                # otherwise the fit picks up on outliers
                xmin = mu - 2 * std
                xmax = mu + 2 * std
    
                adc_sliced = [_ix for _ix in adc if xmin < _ix < xmax]
                
                if len(adc_sliced) > 10:
                    # Fit a normal distribution to the data:
                    mu, std = norm.fit(adc_sliced)
                    chi2 = chisquare(adc_sliced)
                    
                    xmin = mu - 2 * std
                    xmax = mu + 2 * std
                    # find occurence of values that are in +-2sigma off of the mean
                    npeak = 0
                    for iadc in adc_sliced:
                        if xmax>iadc>xmin:
                            npeak+=1
    
                    # 3rd order correction for nonlinearity 
                    # (probably gas diffusion?)
                    A=1700.0
                    mu=mu*(1+mu*mu/A/A)
    
                    fit_array_tmp[ic] = (ix, iy, mu, std, nraw, npeak, chi2[0], chi2[1])
            ic += 1



    fit_array = np.array(fit_array_tmp)
    mu = fit_array[:,2]
    mu = mu.astype( float )
    nraw = fit_array[:,4]
    npeak = fit_array[:,5]
    
    # plot results
    if(create_gif):        
        gs = gridspec.GridSpec(2, 1, height_ratios=[5, 1]) 
        plt.figure(figsize=(40,30))
        plt.rcParams.update({'font.size': 26})
        mumin = 200
        mumax = 800
        plt.subplot(gs[0])
        grid = mu.reshape(224, 160)
        plt.imshow(grid.T, interpolation='nearest', vmin=mumin, vmax=mumax, cmap=cm.viridis)
        plt.gca().invert_yaxis()
        #plt.xlim(43, 180)
        plt.xlim(0, 224)
        plt.ylim(15, 145)
        divider = make_axes_locatable(plt.gca())
        cax = divider.append_axes("right", size="5%", pad=0.05)
        plt.colorbar(cax=cax)
        plt.subplot(gs[1])
        #count, bins, ignored = plt.hist( mu, range=(mumin, mumax), bins=600)
        #step = (mumax-mumin)*itime
        #xmin_plot = mumin+step
        #xmax_plot = mumax+step
        print(itime, np.mean(mu[mu>0]))
        _mu = 0; _mu = np.mean(mu[mu>0])
        plt.scatter(itime*4, _mu)
        plt.xlim(0,ntimes*4)
        plt.ylim(300, 500)
        plt.xticks(np.arange(0, ntimes*4, 4.0))
        plt.gca().set_xlabel('time [min]', fontsize=40)
       
        #plt.show()
        plt.savefig("{}_mean_map_{:02d}.png".format(basename, itime))

    
    return np.array([itime, np.mean(mu[mu>0]), np.sum(nraw), np.sum(npeak)])
    
    """
    outFileName = fileName.replace('.ebe', 'PREMAP')
    outFileName = outFileName.replace('GemQa2','')
    outFileName = '{}{}.txt'.format(outFileName, itime)
    print( 'Saving map to: {}'.format(outFileName))

    outF = open(outFileName, 'w+')
    for ix in range(224):
        for iy in range(160):

            gbin = get_global_index(ix, iy)
            adc_str = ""
            if len(aADC[gbin])>10:
                #print( aADC[gbin])
                for iadc in aADC[gbin]:
                    adc_str += str(iadc)+"\t"

            outF.writelines(["{:d}\t{:d}\t{:d}\t{:s}\n".format(ix, iy, gbin, adc_str)])
    #return aADC
    """





def find_adc_index(x):
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


def find_cluster_part(arr):
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

def find_cluster(xl, xr, yl, yr):

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
print("")
if len(sys.argv)<2:
    filename='/home/vargyas/cernbox/Work/ALICE/serviceWork/GS/I-G1-008/GemQa2Run339.ebe'
else:
    filename=sys.argv[1]

generate_map_from_file(filename)
print("")
