import os
import time

def GuessFoilName(name, method):
    fName = name
    fName=fName.replace("/{}".format(method),"")
    fName = os.path.basename(fName)
    
    # remove budapest conventions
    fName = fName.replace("data_","")
    fName = fName.replace("-s-1","-s")
    fName = fName.replace("-u-1","-u")
    fName = fName.replace("/","")

    return fName

method = "results_svm" 
#method = "results_conv4"

scandir='/mnt/alicetpc/storage/opticalScan/'
datadir='/mnt/alicetpc/storage/opticalData/'

dirs = next(os.walk(scandir))[1]
dir_all = []
dir_proc = []
#fOutDir = "/home/vargyas/cernbox/Work/ALICE/serviceWork/OS";
fOutDir = datadir+method;

# looping over all directories
# determine which was processed with optical recognition
for idir in dirs:
    # select in which are we interested in
    if 'I-' in idir or 'O2-' in idir or 'O3-' in idir:
        subdirs = os.listdir(scandir+idir)
        for isubdir in subdirs:
            if isubdir == method: 
                dir_all.append(idir)

# looping over processed directories and run hdf5 converter
# on those which were not converted already
for idir in dir_all:

    fDataDir = scandir+idir+"/"+method;
    # ROOTname has to be determined
    fName = GuessFoilName(fDataDir,method);
    ROOTName = "{}/{}.root".format(fOutDir, fName);

    # if ROOT file exists, skip processing
    if os.path.isfile(ROOTName):
         continue

    dir_proc.append(idir)

print(dir_proc)
print("{} folders will be converted".format(len(dir_proc)))
time.sleep(20)

count = 1
ROOTNames = []
for idir in dir_proc:
    print('PROCESSING FOLDER ', count, '/', len(dir_proc), ':')

    #fDataDir = "/media/gridserv/opticalScan/"+idir+"/results_conv4";
    fDataDir = scandir+idir+"/"+method;
    # ROOTname has to be determined
    fName = GuessFoilName(fDataDir,method);
    ROOTName = "{}/{}.root".format(fOutDir, fName);
    ROOTNames.append("{}.root".format(fName))
    # This program creates a ROOT tree from the optical scan's hdf5 file output
    execute = "python loadHDF5tree.py {} {}".format(fDataDir,ROOTName);
    print( "Processing command:\n", execute )
    os.system(execute)
    count += 1


# After conversion finished, run the optical processing on the ROOT files
# Note that only for the segmented side, the optreport finds the unsegmented 
#    assuming the same name and substitutes s->u.
for iroot in ROOTNames:
    if "-s" in iroot:
        execute = "../optreport {}".format(iroot)
        print("Processing command:\n", execute)
        os.system(execute)


