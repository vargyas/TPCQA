# -*- coding: utf-8 -*-
"""
Created on Fri Sep 22 12:01:27 2017

@author: vargyas
"""

import numpy as np
import os 
import sys

def calc_gain(basedir, hv, ref):
    dhv  = np.loadtxt(os.path.join(basedir,"Run{}MAP.txt".format(hv)))
    dref = np.loadtxt(os.path.join(basedir,"Run{}MAP.txt".format(ref)))

    # x, y, mean_hv, mean_ref, npeak_hv, npeak_ref, chi2_hv, chi2_ref
    result = np.array([
        dref[:,0], dref[:,1], dhv[:,2], dref[:,2], 
        dhv[:,5], dref[:,5], dhv[:,6], dref[:,6] ])
    
    np.savetxt(os.path.join(basedir,"gainmap.txt"), result.T, fmt='%.2f')


if len(sys.argv)<1:
    print "You should provide 'basedir', 'hv run number', 'ref run number'"
    

basedir = sys.argv[1]
hv_number = sys.argv[2]
ref_number = sys.argv[3]

calc_gain(basedir, hv_number, ref_number)
