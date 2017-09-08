import os
import time

def GuessFoilName(name):
    fName = name
    fName=fName.replace("/results_conv4","")
    fName = os.path.basename(fName)
    
    # remove budapest conventions
    fName = fName.replace("data_","")
    fName = fName.replace("-s-1","-s")
    fName = fName.replace("-u-1","-u")
    fName = fName.replace("/","")

    return fName


dirs = next(os.walk('/media/gridserv/opticalScan/'))[1]
dir_all = []
dir_proc = []
fOutDir = "/home/vargyas/cernbox/Work/ALICE/serviceWork/OS";

# looping over all directories
# determine which was processed with optical recognition
for idir in dirs:
    # select in which are we interested in
    if 'I-' in idir or 'O2-' in idir:
        subdirs = os.listdir('/media/gridserv/opticalScan/'+idir)
        for isubdir in subdirs:
            if isubdir=='results_conv4':
                dir_all.append(idir)

# looping over processed directories and run hdf5 converter
# on those which were not converted already
for idir in dir_all:

    fDataDir = "/media/gridserv/opticalScan/"+idir+"/results_conv4";
    # ROOTname has to be determined
    fName = GuessFoilName(fDataDir);
    ROOTName = "{}/{}.root".format(fOutDir, fName);

    # if ROOT file exists, skip processing
    if os.path.isfile(ROOTName):
         continue

    dir_proc.append(idir)

print len(dir_proc), "folders will be converted"

count = 1
for idir in dir_proc:
    print 'PROCESSING FOLDER ', count, '/', len(dir_proc), ':'

    fDataDir = "/media/gridserv/opticalScan/"+idir+"/results_conv4";
    # ROOTname has to be determined
    fName = GuessFoilName(fDataDir);
    ROOTName = "{}/{}.root".format(fOutDir, fName);

    # This program creates a ROOT tree from the optical scan's hdf5 file output
    execute = "python loadHDF5tree.py {} {}".format(fDataDir,ROOTName);
    print "Processing command:\n", execute 
    os.system(execute)
    count += 1

