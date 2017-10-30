#########################################################################
###
### This macro reads the neural network outputs of the optical scan.
### Files are stored in HDF5 format, details are in ALICE*.recipe.
### It reads them into a ROOT tree and saves them into a ROOT file.
###
### python loadHDF5tree.py [directory path which contain the hdf5 files]
###  it will create outfile.root in same directory with the ROOT trees
###
### Author: Vargyas, Marton
### Email: mvargyas@cern.ch
### 2017
#########################################################################

from ROOT import TFile, TTree
import tables, os, glob
import sys
from array import array

# Loop over the chunks
print( '=========================')
ifile = 0
datadir = sys.argv[1]

outname = "~/outfile.root"
if len(sys.argv) > 2:
    outname = sys.argv[2]
outFile = TFile(os.path.join(datadir,outname), 'RECREATE')
outFile = TFile(outname, 'RECREATE')
outFile.cd()

# Generate trees
treenames = ["blocked", "defect", "etching", "inner", "outer"]
tree = []
_x = array("d", [0])
_y = array("d", [0])
_d = array("d", [0])
_a = array("d", [0])
_b = array("d", [0])
_fl = array("d", [0])

for it in range(len(treenames)):
    tree.append(TTree(treenames[it]+"_tree", ""))    
    tree[it].Branch('x', _x, 'x/D')
    tree[it].Branch('y', _y, 'y/D')
    tree[it].Branch('d', _d, 'd/D')
    tree[it].Branch('a', _a, 'a/D')
    tree[it].Branch('b', _b, 'b/D')
    tree[it].Branch('fl', _fl, 'fl/D')          
      
for ifilename in glob.iglob('{}/*.h5'.format(datadir)):
    ifile += 1
    print 'load file:', ifile,'/',len(glob.glob('{}/*.h5'.format(datadir))), ':', ifilename
    h5file = tables.open_file(ifilename, mode='r')

    data={} 
    for idx, group in enumerate(h5file.walk_groups()):
        if idx>0:
            flavour=group._v_name
            data[flavour]=group.data.read()
            x = data[flavour][:,0]
            y = data[flavour][:,1]
            d = data[flavour][:,7]
            a = data[flavour][:,19]
            b = data[flavour][:,18]
            fl = data[flavour][:,24]
            for i in range(len(x)): # all have the same dimension
                _x[0]=x[i]; _y[0]=y[i]; _d[0]=d[i]
                _a[0]=a[i]; _b[0]=b[i]; _fl[0]=fl[i]
                tree[idx-1].Fill()
    h5file.close()

print( '=========================')
for it in tree:
    it.Write()

outFile.Write()
