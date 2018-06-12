import numpy as np
import sys
from ROOT import TFile, TTree, TH2D


def CleanHist(h, i):
    """
    Create trapezoid mask from blueprint and sets histograms content to 0 outside that.
    i is index, 0: IROC, 1-3: OROC1-3
    """
    # blueprint sizes in mm with the copper area included
    # iroc,  oroc1, oroc2, oroc3, unrecognised (largest is chosen)
    xl_bp = [467.0, 595.8, 726.2, 867.0, 867.0]
    xs_bp = [292. , 457.8, 602.8, 733.3, 733.3]
    m_bp  = [496.5, 353.0, 350.0, 379.0, 379.0]
    
    # two sides of the trapezoid
    coeff_left  = np.polyfit([-xl_bp[i]/2.,-xs_bp[i]/2.],[m_bp[i]/2.,-m_bp[i]/2.], 1)
    coeff_right = np.polyfit([xl_bp[i]/2.,xs_bp[i]/2.],[m_bp[i]/2.,-m_bp[i]/2.], 1)
    side_left = np.poly1d(coeff_left)
    side_right = np.poly1d(coeff_right)

    # apply mask to histogram
    nx = h.GetNbinsX()
    ny = h.GetNbinsY()
    for ix in range(1, nx+1):
        for iy in range(1, ny+1):
            x = h.GetXaxis().GetBinCenter(ix)
            y = h.GetYaxis().GetBinCenter(iy)
            if y>m_bp[i]/2. or y<-m_bp[i]/2. or y<side_left(x) or y<side_right(x):
                h.SetBinContent(ix,iy,0)


def GetIndex(name):
    """
    Sets name of the foil and decides flavour (0: IROC, 1-3: OROC1-3)
    """
    if "I-" in name: return 0
    elif "O1-" in name: return 1
    elif "O2-" in name: return 2
    elif "O3-" in name: return 3
    else: return 3


odir = "/home/vargyas/Dropbox/TPCQA/data/OS/"
infilename = sys.argv[1]


topt = TTree("opt_tree", "optical tree")
#self._topt.ReadFile(infilename, "x/D:y:d_i_s:d_o_s:d_i_u:d_o_u:n_i_s:n_o_s:n_i_u:n_o_u:std_i_s:std_o_s:std_i_u:std_o_u:foo1:foo2")
topt.ReadFile(odir+infilename+".txt", "x/D:y:d_i_s:d_o_s:d_b_s:d_d_s:d_e_s:d_i_u:d_o_u:d_b_u:d_d_u:d_e_u:n_i_s:n_o_s:n_b_s:n_d_s:n_e_s:n_i_u:n_o_u:n_b_u:n_d_u:n_e_u:std_i_s:std_o_s:std_i_u:std_o_u:fg_s:fg_u")        

# create histograms:
hdiam = []
hdiam.append( TH2D("hdis","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hdiu","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hdos","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hdou","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hdstds","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hdstdu","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hndis","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hndiu","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hndos","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )
hdiam.append( TH2D("hndou","",224,-224*2,224*2,160,-160*3/2.,160*3/2.) )

# note: segmented side should be flipped
topt.Draw("y:-x>>hdis","d_i_s","goff")
topt.Draw("y:x>>hdiu", "d_i_u","goff")
topt.Draw("y:-x>>hdos","d_o_s","goff")
topt.Draw("y:x>>hdou", "d_o_u","goff")
topt.Draw("y:-x>>hdstds","std_o_s","goff")
topt.Draw("y:x>>hdstdu", "std_o_u","goff")
topt.Draw("y:-x>>hndis","n_i_s","goff")
topt.Draw("y:x>>hndiu", "n_i_u","goff")
topt.Draw("y:-x>>hndos","n_o_s","goff")
topt.Draw("y:x>>hndou", "n_o_u","goff")

# correcting diameter histograms for different
# binning, optical is 1mm step, gain is 3mmx4mm
# also normalising each diameter histo to its mean
diam_mean = [0, 0, 0, 0, 0, 0]

rootfile = TFile(odir+infilename+".root","RECREATE")
for i in range(len(hdiam)):
    #CleanHist(hdiam[i],GetIndex(infilename))
    hdiam[i].Scale(1./12.)    
    hdiam[i].Write()
    # threshold should be different for rim?
    # self._diam_mean[i] = GetMeanZ(self._hdiam[i], 1)
    # self._hdiam[i].Scale(1./self._diam_mean[i])
    # hdiam[i].SetName("{}_{}".format(self._hdiam[i].GetName(),self._ID))
rootfile.Write()



