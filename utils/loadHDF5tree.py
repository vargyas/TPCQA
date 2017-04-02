#########################################################################
###
### This macro reads the neural network outputs of the optical scan.
### Files are stored in HDF5 format, details are in ALICE3.recipe. 
### It reads them into a ROOT tree and saves them into a ROOT file.
###
### python loadHDF5tree.py [directory path which contain the hdf5 files]
###  it will create outfile.root in same directory with the ROOT trees
###
### Author: Vargyas, Marton
### Email: mvargyas@cern.ch
### 2016
#########################################################################

import numpy as np
import h5py
from ROOT import *
import tables, os, glob
import sys

from root_numpy import root2array, tree2array
from root_numpy import array2tree, array2root
from root_numpy.testdata import get_filepath


def convert_dict_nparray(dictionary, key):
    """
    Convert dictionary to named numpy array
    which will be converted into a TTree
    (names are according to ALICE3.recipe,
    with whitespaces replaced by "_")
    """
    while dictionary.has_key(key):
        try:
            # convert np.ndarray (2D) into a list of tuples
            tmp_array = list()
            for line in dictionary[key]:
                line = tuple(line)
                # only keep x, y, area, diameter, moment_1, moment_2
                #sliceObj = slice(0, 1, 6, 7) 
                tmp_array.append( (line[0], line[1], line[6], line[7], line[12], line[13]) )
            
            # create a named list which will be directly
            # converted to a TTree object
            my_array = np.array( tmp_array, 
                    dtype=[('x',np.float64),
                        ('y',np.float64),
#                        ('bb_x',np.float64),
#                        ('bb_y',np.float64),
#                        ('bb_width',np.float64),
#                        ('bb_height',np.float64),
                        ('area',np.float64),
                        ('diameter',np.float64),
#                        ('perimColour_B',np.float64),
#                        ('perimColour_G',np.float64),
#                        ('perimColour_R',np.float64),
#                        ('ellipse_angle',np.float64),
                        ('moment_1',np.float64),
                        ('moment_2',np.float64),
#                        ('moment_3',np.float64),
#                        ('moment_4',np.float64),
#                        ('moment_5',np.float64),
#                        ('moment_6',np.float64),
#                        ('ellipse_big_axis',np.float64),
#                        ('ellipse_small_axis',np.float64),
#                        ('bb_Colour_B',np.float64),
#                        ('bb_Colour_G',np.float64),
#                        ('bb_Colour_R',np.float64),
#                        ('bool_0=background_1=foreground',np.bool)
                        ] )
            return my_array
        except:
            print 'not valid key'
            return -1



def load_HDF5file_dict(infilename):
    """
    Load HDF5 files into dictionary
    """
    h5file = tables.open_file(infilename, mode='r')
    data={} 
    for idx, group in enumerate(h5file.walk_groups()):
        if idx>0:
            flavour=group._v_name
            # only load inner and outer identifiers
            if flavour=='inner' or flavour=='outer':
                data[flavour]=group.data.read()
    h5file.close()
    return data


"""
M A I N   P R O G R A M
"""

# Creating empty container for data (the ugly way)
data_array_inner = np.array( [(0,0,0,0,0,0)],
                    dtype=[('x',np.float64),
                        ('y',np.float64),
                        ('area',np.float64),
                        ('diameter',np.float64), 
                        ('moment_1',np.float64), 
                        ('moment_2',np.float64)] )
data_array_outer = np.array( [(0,0,0,0,0,0)],
                    dtype=[('x',np.float64),
                        ('y',np.float64),
                        ('area',np.float64),
                        ('diameter',np.float64),
                        ('moment_1',np.float64), 
                        ('moment_2',np.float64)] )

# Loop over the chunks
print '========================='
ifile = 0
datadir = sys.argv[1]
if len(sys.argv) > 2:
    outname = sys.argv[2]

for ifilename in glob.iglob('{}/*.h5'.format(datadir)):
    print 'load file:', ifile,'/',len(glob.glob('./{}/*.h5'.format(datadir))), ':', ifilename
    # load HDF5 into a python dictionary
    data_dict = load_HDF5file_dict(ifilename)
    # convert that to named (structured) numpy array
    data_array_tmp_inner = convert_dict_nparray(data_dict, 'inner')
    data_array_tmp_outer = convert_dict_nparray(data_dict, 'outer')


    # merge that with the empty container defined outside of the loop
    data_array_inner = np.concatenate( (data_array_tmp_inner, data_array_inner), axis=0 )
    data_array_outer = np.concatenate( (data_array_tmp_outer, data_array_outer), axis=0 )

    ifile += 1
    print '========================='

outname = "outfile.root"
# create a tree from the array
print '\ncreate a tree from the array'
treeInner = array2tree(data_array_inner, name='inner_tree')
treeOuter = array2tree(data_array_outer, name='outer_tree')

# merge the trees (takes some time)
#tree = TTree.MergeTrees( tlist )

print 'writing file to:', outname
outFile = TFile(os.path.join(datadir,outname), 'RECREATE')
outFile.cd()

treeInner.Write()
treeOuter.Write()

outFile.Write()
outFile.Close()

