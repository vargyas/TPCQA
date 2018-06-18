
import numpy as np
import numpy.ma as ma
#from functools import reduce
from scipy.optimize import minimize
from ROOT import TCanvas, TPad, TH1D, TH2D, TTree, gStyle, TGraph, TFile, TF1, TLegend, TMath, TPaveText, TLatex
from root_numpy import hist2array, array2hist
import os.path
#from matplotlib import pyplot as plt
#import math
import sys

def ThisThesis(x=0.85, y=0.95, size=50):
    """
    Add 'This thesis' text to plot
    """
    latex =  TLatex(x, y, "This thesis")
    print("ThisThesis()",x,y)
    latex.SetTextFont(43)
    latex.SetNDC()
    latex.SetTextSize(size)
    latex.SetTextColor(1)
    return latex

def ThisThesis_old(x,y,size):
    """
    Add 'This thesis' text to plot
    """
    pt = TPaveText(x,y,1,1,"bnNDC")
    pt.AddText("This thesis")
    pt.SetFillColor(0)
    pt.SetLineColor(0)
    pt.SetLineWidth(0)
    pt.SetTextSize(size)
    pt.SetTextAlign(12)
    pt.SetTextFont(42)
    return pt

def GetMeanZ(h, thr):
    """
    Get mean of TH2 entries with threshold
    """
    nb = h.GetNbinsX() * h.GetNbinsY()
    n, meanz, z = 0, 0.0, 0.0
    for ib in range(1, nb+1):
        z = h.GetBinContent(ib)
        if z>thr:
            meanz+=z
            n+=1 
    if n>0: return meanz/n
    else: return -1

def CorrelationArrayToTHN(xmin, xmax, x, ymin, ymax, y, dim=2):
    """
    Create correlation from two lists, 
    dim==2: TH2 is better than TGraph as it has the colz drawing option
    dim==1: TH1 residual histogram
    """
    if dim==2:
        htmp = TH2D("htmp2","",100*int(xmax-xmin),xmin,xmax,100*int(ymax-ymin),ymin,ymax)
    else:
        htmp = TH1D("htmp1","",400, 0.0, 3.0)
        
    if len(x)!=len(y):
        print("Error in CorrelationArrayToTH2(), the two lists do not have the same dimension\n")
        print("Returning empty histogram")
        return htmp
    else:
        for i in range(len(x)):
            if dim==2: htmp.Fill(x[i], y[i])
            else: 
                if y[i]>0: htmp.Fill(x[i]/y[i])
        return htmp 
    
    
class Foil:
    
    def __init__(self, ID, ftype, gainfile, optfile):
        print("Initializing foil:", gainfile)
        self._ID = ID # foil index to ensure unique ROOT naming
        self._iplot = 0 # plot index to avoid overwriting of canvases
        self._type = ftype # 0: SP, 1: LP
        self._gdir = "/home/vargyas/Dropbox/TPCQA/data/GS/"
        self._odir = "/home/vargyas/Dropbox/TPCQA/data/OS/"
        
        # init name from gain directory name as it is usually the barcode name
        self.SetName(gainfile)
        self.CreateContainers()
        self.LoadGain(gainfile)
        self.LoadOptical(optfile)
        self.ConvertHistsToArray()
        
    def CleanHist2(self, h):
        """
        Create trapezoid mask from blueprint and sets histograms content to 0 outside that.
        """
        # blueprint sizes in mm with the copper area included
        # iroc,  oroc1, oroc2, oroc3, unrecognised (largest is chosen)
        xl_bp = [467.0, 595.8, 726.2, 867.0, 867.0]
        xs_bp = [292. , 457.8, 602.8, 733.3, 733.3]
        m_bp  = [496.5, 353.0, 350.0, 379.0, 379.0]
        i = self._type
        # two sides of the trapezoid
        coeff_left  = np.polyfit([-xl_bp[i]/2.,-xs_bp[i]/2.],[m_bp[i]/2.,-m_bp[i]/2.], 1)
        coeff_right = np.polyfit([xl_bp[i]/2.,xs_bp[i]/2.],[m_bp[i]/2.,-m_bp[i]/2.], 1)
        side_left = np.poly1d(coeff_left)
        side_right = np.poly1d(coeff_right)

        # apply mask to histogram (slightly larger than blueprint)
        eps = 0
        nx = h.GetNbinsX()
        ny = h.GetNbinsY()
        for ix in range(1, nx+1):
            for iy in range(1, ny+1):
                x = h.GetXaxis().GetBinCenter(ix)
                y = h.GetYaxis().GetBinCenter(iy)
                if y>m_bp[i]/2.+eps or y<-m_bp[i]/2.-eps or y<side_left(x) or y<side_right(x):
                    h.SetBinContent(ix,iy,0)
        
    def CleanHist(self, h):
        """
        Sets histograms content to 0 outside of provided mask.
        """
       
        # TF1: left side, right side, top, bottom
        f_l,f_r,f_t,f_b=self.GetFrame()
        
        # apply mask to histogram (slightly larger than blueprint)
        nx = h.GetNbinsX()
        ny = h.GetNbinsY()
        for ix in range(1, nx+1):
            for iy in range(1, ny+1):
                x = h.GetXaxis().GetBinCenter(ix)
                y = h.GetYaxis().GetBinCenter(iy)
                if y<f_b(0) or y>f_t(0) or y<f_l(x) or y<f_r(x):
                    h.SetBinContent(ix,iy,0)
                    
    def GetFrame(self, epsx=0, epsy=0):
        """
        Create trapezoid mask from blueprint (sizes in mm), with the copper 
        area included. Frame can be reduced with variable eps. Default is 10mm,
        marking the active area (also from blueprint)
        values from https://cds.cern.ch/record/1622286/files/ALICE-TDR-016.pdf
        """
        
        # iroc,  oroc1, oroc2, oroc3, unrecognised (largest is chosen) 
        xl_bp = [467.0, 595.8, 726.2, 867.0, 867.0]
        xs_bp = [292. , 457.8, 602.8, 733.3, 733.3]
        m_bp  = [496.5, 353.0, 350.0, 379.0, 379.0]
        
        i  = self._type
        xl = xl_bp[i]-epsx
        xs = xs_bp[i]-epsx
        m  = m_bp[i]-epsy
        # two sides of the trapezoid
        coeff_left  = np.polyfit([-xl/2.,-xs/2.],[m/2.,-m/2.], 1)
        coeff_right = np.polyfit([xl/2.,xs/2.],[m/2.,-m/2.], 1)
        #side_left = np.poly1d(coeff_left)
        #side_right = np.poly1d(coeff_right)
        f0 = TF1("f0","pol1",-1000,1000); f0.SetParameters(coeff_left[1], coeff_left[0])
        f1 = TF1("f1","pol1",-1000,1000); f1.SetParameters(coeff_right[1], coeff_right[0])
        f2 = TF1("f2","pol1",-1000,1000); f2.SetParameters(m/2., 0)
        f3 = TF1("f3","pol1",-1000,1000); f3.SetParameters(-m/2., 0)
        return f0,f1,f2,f3
    
    def GetFrame2(self):
        """
        blueprint sizes in mm with the copper area included
        iroc,  oroc1, oroc2, oroc3, unrecognised (largest is chosen)
        values from https://cds.cern.ch/record/1622286/files/ALICE-TDR-016.pdf
        """
        xl_bp = [467.0, 595.8, 726.2, 867.0, 867.0]
        xs_bp = [292. , 457.8, 602.8, 733.3, 733.3]
        m_bp  = [496.5, 353.0, 350.0, 379.0, 379.0]
        
        i = self._type
        # two sides of the trapezoid
        coeff_left  = np.polyfit([-xl_bp[i]/2.,-xs_bp[i]/2.],[m_bp[i]/2.,-m_bp[i]/2.], 1)
        coeff_right = np.polyfit([xl_bp[i]/2.,xs_bp[i]/2.],[m_bp[i]/2.,-m_bp[i]/2.], 1)
        #side_left = np.poly1d(coeff_left)
        #side_right = np.poly1d(coeff_right)
        f0 = TF1("f0","pol1",-1000,1000); f0.SetParameters(coeff_left[1], coeff_left[0])
        f1 = TF1("f1","pol1",-1000,1000); f1.SetParameters(coeff_right[1], coeff_right[0])
        f2 = TF1("f2","pol1",-1000,1000); f2.SetParameters(m_bp[i]/2., 0)
        f3 = TF1("f3","pol1",-1000,1000); f3.SetParameters(-m_bp[i]/2., 0)
        return f0,f1,f2,f3
        
    def SetName(self, infilename):
        """
        Sets name of the foil and decides type (0: IROC, 1-3: OROC1-3)
        and flavour (0: S, 1: LP)
        """
        
        self._name = infilename 
        # and decide foil type and flavour
        if "I-" in self._name: self._type = 0
        elif "O1-" in self._name: self._type = 1
        elif "O2-" in self._name: self._type = 2
        elif "O3-" in self._name: self._type = 3
        else: self._type = 3 # if in doubt, choose largest one
        print(infilename, "type recognized:", self._type)

        if "G1" in self._name or "G4" in self._name: self._flavour = 0
        elif "G2" in self._name or "G3" in self._name: self._flavour = 1
        else: self._flavour = 0 # if in doubt, choose S
        print("\tflavour recognized:", self._flavour)
        
    def CreateHisto(self, name, title):
        """
        Create usual histo with 224*160 dimensions
        """
        return TH2D(name, title, 224,-224*2,224*2,160,-160*3/2.,160*3/2.)
    
    def CreateContainers(self):
        """
        Create all 2D histograms for gain (1) and diameters (16)
        """
        self._hdiam = {}
        
        self._hdiam["dis"] = self.CreateHisto("hdis", "Inner segmented diameter")
        self._hdiam["diu"] = self.CreateHisto("hdiu", "Inner unsegmented diameter")
        self._hdiam["dos"] = self.CreateHisto("hdos", "Outer segmented diameter")
        self._hdiam["dou"] = self.CreateHisto("hdou", "Outer unsegmented diameter")
        
        self._hdiam["nis"] = self.CreateHisto("hnis", "Inner segmented density")
        self._hdiam["niu"] = self.CreateHisto("hniu", "Inner unsegmented density")
        self._hdiam["nos"] = self.CreateHisto("hnos", "Outer segmented density")
        self._hdiam["nou"] = self.CreateHisto("hnou", "Outer unsegmented density")
        
        self._hdiam["ais"] = self.CreateHisto("hais", "Inner segmented minor axis")
        self._hdiam["aiu"] = self.CreateHisto("haiu", "Inner unsegmented minor axis")
        self._hdiam["aos"] = self.CreateHisto("haos", "Outer segmented minor axis")
        self._hdiam["aou"] = self.CreateHisto("haou", "Outer unsegmented minor axis")
        
        self._hdiam["bis"] = self.CreateHisto("hbis", "Inner segmented major axis")
        self._hdiam["biu"] = self.CreateHisto("hbiu", "Inner unsegmented major axis")
        self._hdiam["bos"] = self.CreateHisto("hbos", "Outer segmented major axis")
        self._hdiam["bou"] = self.CreateHisto("hbou", "Outer unsegmented major axis")
        
        self._hgain = self.CreateHisto("hgain_{}".format(self._ID),"MEASURED GAIN") 
        
    def CopyOptical(self, hin, hout):
        """
        Copies hin into the hout histogram, which is binned similarly but
        has different ranges. 
        """
        nx = hout.GetNbinsX()
        ny = hout.GetNbinsY()
        for ix in range(1, nx+1):
            for iy in range(1, ny+1):
                x = hout.GetXaxis().GetBinCenter(ix)
                y = hout.GetYaxis().GetBinCenter(iy)
                ix_in = hin.GetXaxis().FindBin(x)
                iy_in = hin.GetYaxis().FindBin(y)
                z = hin.GetBinContent(ix_in, iy_in)
                hout.SetBinContent(ix, iy, z)
        #print("CopyOptical() Over/Underflow in x and y")
        #x, y = hout.GetXaxis(), hout.GetYaxis()
        #print(hout.GetBinContent(0, 0), hout.GetBinContent(nx, ny))
        
    def LoadOptical(self, infilename):
        """
        Loads optical histograms from ROOT file. If no ROOT file is present, it loads
        text file and generates ROOT file.
        """
        if os.path.isfile(self._odir+infilename+".root"):
            print("\t Loading optical from ROOT...")
            histnames = ["hdis","hdiu","hdos","hdou","hnis","hniu","hnos","hnou","hais","haiu","haos","haou"]
            self.infile = TFile.Open(self._odir+infilename+".root")
            for i in range(len(histnames)):
                #self._hdiam[i] = self.infile.Get(histnames[i])
                histname = histnames[i]
                hindex = histname[1:]
                self.CopyOptical(self.infile.Get(histnames[i]), self._hdiam[hindex])
                self._hdiam[hindex].SetName("{}_{}".format(self._hdiam[hindex].GetName(),self._ID))
        
        else:
            print("\t Input file", infilename, "cannot be found, exiting program")
            sys.exit
        
        for ih in self._hdiam:
            self.CleanHist(self._hdiam[ih])
        
    def LoadGain(self, infilename):
        """
        Loads gain map from file into numpy array.
        Replaces artificial negative values with 0.
        """
        if os.path.isfile(self._gdir+infilename+".root"):
            print("\t Loading gain from ROOT...")        
            self.infile_gain = TFile.Open(self._gdir+infilename+".root")
            self._hgain = self.infile_gain.Get("hgain")
            self.CleanHist(self._hgain)
               
        else:
            print("\t Loading gain from text...")        
            self._tgain = TTree("gain_tree","gain tree");
            self._tgain.SetDirectory(0);
            self._tgain.ReadFile(self._gdir+infilename+".txt","x:y:mean_hv:mean_ref:n_hv:n_ref:std_hv:std_ref");
            self._tgain.Draw("(y-80)*3:(x-111.5)*4>>hgain_{}".format(self._ID),"mean_hv/mean_ref","goff");

            rootfile = TFile(self._gdir+infilename+".root", "RECREATE")
            self._hgain.Write("hgain")
            rootfile.Write()         
        
        if self._name=='I-G1-008': 
            print("\t CORRECTING FOIL FOR DIFFERENT SW VOLTAGE")
            self._hgain.Scale(1./1.42)

        if self._name=='O2-G1-005': 
            print("\t CORRECTING FOIL FOR DIFFERENT SW VOLTAGE")
            self._hgain.Scale(1.42)        
        if self._name=='O2-G1-006': 
            print("\t CORRECTING FOIL FOR DIFFERENT SW VOLTAGE")
            self._hgain.Scale(1.42) 
        
        if self._name=='O2-G2-021':
            print("\t CORRECTING FOIL FOR DIFFERENT SW VOLTAGE")
            self._hgain.Scale(1./1.42) 
        if self._name=='O2-G2-027':
            print("\t CORRECTING FOIL FOR DIFFERENT SW VOLTAGE")
            self._hgain.Scale(1./1.42)    
        self.CleanHist(self._hgain)
        
    def ConvertHistsToArray(self):
        """
        Extracts 2D arrays from the histograms and reshapes them to 1D
        Also generates masked histograms, so outlyers would not contaminate the correlation
        """
        
        self._adiam = {}
        self._adiam_c = {}
        
        for ikey in self._hdiam.keys():
            self._adiam[ikey] = hist2array(self._hdiam[ikey]).reshape(224*160)
                                    
        # probably best is to use this function and mask outlyers while calculating the correlation
        for ikey in ["dis","diu","ais","aiu","bis","biu"]:
            self._adiam_c[ikey] = ma.masked_outside(self._adiam[ikey], 30, 90)
       
        for ikey in ["dos","dou","aos","aou","bos","bou"]:
            self._adiam_c[ikey] = ma.masked_outside(self._adiam[ikey], 40, 150)

        self._g = hist2array(self._hgain).reshape(224*160)
        self._g_c = ma.masked_outside(self._g, 0.1, 5)
        
        self._m_mean = ma.mean(self._adiam_c["dis"])
        self._t_mean = ma.mean(self._adiam_c["dos"])
        self._b_mean = ma.mean(self._adiam_c["dou"])
        print("Middle mean: {:.1f}, Top mean: {:.1f}, Bottom mean {:.1f}".format( self._m_mean, self._t_mean, self._b_mean) )

    def OptFormula(self, coeffs):

        w_am = coeffs[0]
        w_at = coeffs[1]
        w_ab = coeffs[2]
        
        #w_am = coeffs[3]
        #w_ab = coeffs[4]
        #w_all = w_dm+w_dt+w_db+w_am+w_ab
        w_all = w_at + w_ab + w_am

        formula = w_ab/w_all*self._adiam_c["aou"] + w_at/w_all*self._adiam_c["aos"] + w_am/w_all*self._adiam_c["ais"]
        return ma.masked_outside(formula, 0, 1000)
        
    def GainFormula(self, coeffs):
        G_all = [0.0456, 0.0456, 0.0434, 0.0456]
        G = G_all[0] # assuming universal gain coefficient 
  
        #d0_all = [75.5, 90, 80, 80]
        #d0 = d0_all[self._flavour]
        d0 = coeffs[2+self._ID]
        #with np.errstate(divide='ignore'):
        return np.exp( G * (d0-self.OptFormula(coeffs)) )
        
    def GeneratePredGain(self, coeffs):
        print("Generating pred. gain with parameters", coeffs)
        self._hpredgain =  self.CreateHisto("hpredgain_{}".format(self._ID),"PREDICTED GAIN")        
        self._predgain = self.GainFormula(coeffs).reshape(224, 160) 
        print("predicted gain mean:",np.mean(self._predgain[self._predgain<10]))
        print("predicted gain std:",np.std(self._predgain[self._predgain<10]))
        print("measured gain mean:",np.mean(self._g))
        print("measured gain std:",np.std(self._g))
        array2hist(self._predgain, self._hpredgain)  
        self.CleanHist(self._hpredgain)
        
    def GenerateCorrelation(self): 
        """
        Generates the opt-gain correlation graph and the residual histogram
        """
        #kWhite  = 0,   kBlack  = 1,   kGray    = 920,  kRed    = 632,  kGreen  = 416,
        #kBlue   = 600, kYellow = 400, kMagenta = 616,  kCyan   = 432,  kOrange = 800,
        #kSpring = 820, kTeal   = 840, kAzure   =  860, kViolet = 880,  kPink   = 900
        colors = [601, 823, 1, 633, 920, 617, 861, 401, 432, 880, 417, 800, 900]
        
        p = self._predgain.reshape(1, 224*160) 
        g = self._g.reshape(1, 224*160)
        n = 224*160
        
        # opt-gain correlation
        self._gcorr = TGraph(n)
        self._gcorr.SetName("gcorr{}".format(self._ID))
        for i in range(n): self._gcorr.SetPoint(i, p[0][i], g[0][i])
        self._gcorr.GetXaxis().SetTitle("Predicted gain")
        self._gcorr.GetYaxis().SetTitle("Measured gain")
        self._gcorr.SetTitle(self._name)
        self._gcorr.SetMarkerColor(colors[self._ID])
        self._gcorr.SetLineColor(colors[self._ID])
        
        # opt-gain residual
        self._hocc = TH1D("hocc{}".format(self._ID),"",400, 0.0, 3.0)
        for i in range(n): 
            if(g[0][i]>0):
                self._hocc.Fill(p[0][i] / g[0][i])
        self._hocc.SetTitle(self._name)
        self._focc = TF1("focc{}".format(self._ID),"gaus",0.0,3.0)
        self._focc.SetParameters(1000, 1, 0.3)
        self._focc.SetLineColor(1)
        self._hocc.Fit(self._focc)
        
        # residual check
        xmean, sigma = 0, 0
        mean = self._hocc.GetMean()
        nbins = self._hocc.GetNbinsX()
        n = self._hocc.GetEntries()
        nunder = self._hocc.GetBinContent(0)
        nover  = self._hocc.GetBinContent(nbins+1)
        
        n = n-nunder-nover
        for ib in range(nbins):
            x=self._hocc.GetBinCenter(ib+1)
            y=self._hocc.GetBinContent(ib+1)
            sigma+=self._hocc.GetBinContent(ib+1)*TMath.Power((x-mean),2)
            xmean += x*y
        sigma2 = TMath.Sqrt(sigma/n)
        xmean=xmean/n
        print("estimated width: ", self._hocc.GetRMS(), self._hocc.GetStdDev(), sigma2)
        print("mean check", mean, xmean)
        self._res=self._focc.GetParameter(2)/self._focc.GetParameter(1)*100
        
    def GetResText(self):
        return "{} #sigma/#mu={:.0f}%".format(self._name, self._res)
    
    def DrawGainMaps(self):
        self._iplot += 1

        gStyle.SetOptStat(0)
        xrange = 250
        # default size is IROC, change to OROC2 if recognized
        if self._type==2: xrange = 400
        self._hgain.GetXaxis().SetRangeUser(-xrange, xrange)
        self._hpredgain.GetXaxis().SetRangeUser(-xrange, xrange)
        
        self._hgain.GetZaxis().SetRangeUser(0.5, 1.5)
        self._hpredgain.GetZaxis().SetRangeUser(0.5, 1.5)
        self.FormatMapHist(self._hgain)
        self._hgain.SetTitle("MEASURED GAIN")
        self.FormatMapHist(self._hpredgain)
        
        c = TCanvas("c{}".format(self._iplot),"",1600,800); c.Draw(); c.cd()
        #pt=ThisThesis(0.82, 0.91, 0.05*2)
        pt=ThisThesis()
        pt.Draw()
        
        p1 = TPad("p","",0,0,0.5,0.9); p1.SetRightMargin(0.15)
        p1.Draw(); p1.cd(); self._hgain.Draw("colz")
        f = self.GetFrame()
        for ifunc in f:
            ifunc.SetLineColor(1)
            ifunc.SetLineStyle(2)
            ifunc.Draw("same")
        #f0.Draw("same")
        #f1.Draw("same")
        #f2.Draw("same")
        #f3.Draw("same")
        c.cd()
        p2 = TPad("p","",0.5,0,1,0.9); p2.SetRightMargin(0.15)
        p2.Draw(); p2.cd(); self._hpredgain.Draw("colz")
        #f0.Draw("same")
        #f1.Draw("same")
        #f2.Draw("same")
        #f3.Draw("same")
        for ifunc in f: ifunc.Draw("same")
        c.SaveAs("{}_predgain.pdf".format(self._name))
        
    #def DrawCorrelation(self):
    #    c = TCanvas("c","",1600,900)
    def FormatMapHist(self, h):
        gStyle.SetTitleFontSize(0.08)
        gStyle.SetTextFont(42)
        h.GetXaxis().SetLabelSize(0.04)
        h.GetYaxis().SetLabelSize(0.04)
        h.GetZaxis().SetLabelSize(0.06)
        h.GetZaxis().SetTitleSize(0.07)
        h.GetZaxis().CenterTitle()
        
    def FormatHist(self, h):
        gStyle.SetTitleFontSize(0.08)
        gStyle.SetTextFont(42)
        h.GetXaxis().SetLabelSize(0.04)
        h.GetYaxis().SetLabelSize(0.04)
        h.GetXaxis().SetTitleSize(0.06)
        h.GetYaxis().SetTitleSize(0.06)
        h.GetXaxis().SetTitleOffset(1.1)
        h.GetYaxis().SetTitleOffset(1.1)
        h.GetXaxis().CenterTitle()
        h.GetYaxis().CenterTitle()

    def DrawOptMaps(self, coeffs):
        self._iplot += 1
        gStyle.SetOptStat(0)
        xrange = 250
        # default size is IROC, change to OROC2 if recognized
        if self._type==2: xrange = 400
        
        self._hdiam["dis"].GetXaxis().SetRangeUser(-xrange, xrange)
        self._hdiam["dis"].GetZaxis().SetRangeUser(40, 70)
        self._hdiam["dis"].SetTitle("INNER SEGMENTED")
        self._hdiam["dis"].GetZaxis().SetTitle("hole diameter [#mum]")
        self.FormatMapHist(self._hdiam["dis"])
                
        self._hdiam["dos"].GetXaxis().SetRangeUser(-xrange, xrange)
        self._hdiam["dos"].GetZaxis().SetRangeUser(60, 100)
        self._hdiam["dos"].SetTitle("OUTER SEGMENTED")
        self._hdiam["dos"].GetZaxis().SetTitle("hole diameter [#mum]")
        self.FormatMapHist(self._hdiam["dos"])

        self._hdiam["dou"].GetXaxis().SetRangeUser(-xrange, xrange)
        self._hdiam["dou"].GetZaxis().SetRangeUser(60, 100)
        self._hdiam["dou"].SetTitle("OUTER UNSEGMENTED")
        self._hdiam["dou"].GetZaxis().SetTitle("hole diameter [#mum]")
        self.FormatMapHist(self._hdiam["dou"])

        self._hopt = self.CreateHisto("hopt_{}".format(self._ID),"LINEAR COMBINATION")        
        opt = self.OptFormula(coeffs)
        newopt = opt.reshape(224, 160)
        array2hist(newopt, self._hopt)
        self._hopt.GetXaxis().SetRangeUser(-xrange, xrange)
        self._hopt.GetZaxis().SetRangeUser(40, 110)
        self._hopt.GetZaxis().SetTitle("hole diameter [#mum]")
        self.FormatMapHist(self._hopt)

        
        c = TCanvas("c{}".format(self._iplot),"",1600,1200); c.Draw(); c.cd()
        x=1.0
        y=0.9
        #pt=ThisThesis(0.82, 0.91, 0.05)
        pt=ThisThesis()
        pt.Draw()

        # BOTTOM LEFT
        p1 = TPad("p1","",0,0,x/2,y/2); p1.SetRightMargin(0.2)
        p1.Draw(); p1.cd(); self._hdiam["dis"].Draw("colz")  
        c.cd()
        # BOTTOM RIGHT
        p2 = TPad("p2","",x/2,0,x,y/2); p2.SetRightMargin(0.2)
        p2.Draw(); p2.cd();  self._hopt.Draw("colz")
        c.cd()
        # TOP LEFT
        p3 = TPad("p3","",0,y/2,x/2,y); p3.SetRightMargin(0.2)
        p3.Draw(); p3.cd();    self._hdiam["dos"].Draw("colz")    
        c.cd()
        # TOP RIGHT
        p4 = TPad("p4","",x/2,y/2,x,y); p4.SetRightMargin(0.2)
        p4.Draw(); p4.cd(); self._hdiam["dou"].Draw("colz")  

        c.Update()
        c.SaveAs("{}_opt.pdf".format(self._name))
                
                
    def DrawCorrelation(self):
        """
        Drawing correlation and residual histogram for individual foils
        """
        
        self._iplot += 1
        c = TCanvas("c{}".format(self._iplot),"",1000,1000)
        xmin, xmax = 0.6, 1.5
        
        hcorr=TH2D("hcorr{}".format(self._ID),"",2,xmin,xmax,2,xmin,xmax)
        hcorr.GetXaxis().SetTitle("Predicted gain")
        hcorr.GetXaxis().CenterTitle()
        hcorr.GetYaxis().SetTitle("Measured gain")
        hcorr.GetYaxis().CenterTitle()
        hcorr.Draw()
        self._gcorr.Draw("P same")

        pt=ThisThesis()
        pt.Draw()

        f1 = TF1("f1","pol1",0,2)
        f1.SetParameters(0,1); f1.SetLineColor(1); f1.SetLineStyle(1)
        f1.SetTitle("1")
        
        f2 = TF1("f2","pol1",0,2)
        f2.SetParameters(0,1.2); f2.SetLineColor(1); f2.SetLineStyle(2)
        f2.SetTitle("+20%")
        
        f3 = TF1("f3","pol1",0,2)
        f3.SetParameters(0,0.8); f3.SetLineColor(1); f3.SetLineStyle(2)
        f3.SetTitle("-20%")
        f1.Draw("same"); f2.Draw("same"); f3.Draw("same")
        
        c.SaveAs("{}_corr.pdf".format(self._name))

        #######################################################################
        
        self._iplot += 1
        c = TCanvas("c{}".format(self._iplot),"",1000,1000)
        pt=ThisThesis()
        pt.Draw()

        self._hocc.Draw()
        l = TLegend(0.6, 0.6, 0.88, 0.88)
        l.SetBorderSize(0)
        #l.AddEntry(self._hocc, "{:.0f}%".format(self._hocc.GetRMS()/self._hocc.GetMean()*100))
        l.AddEntry(self._hocc, "#sigma/#mu={:.0f}%".format(self._res))
        l.Draw()
        
        c.SaveAs("{}_res.pdf".format(self._name))
        


    def DrawHoleCorrelation(self):
        
        self._iplot += 1
        print("DrawHoleCorrelation()...")
        # inner average - outer average
        # inner average - outer unsegmented
        # outer segmented - outer unsegmented
        ia = (0.5*(self._adiam["dis"] +self._adiam["diu"])).reshape(1,224*160)
        oa = (0.5*(self._adiam["dos"]+self._adiam["dou"])).reshape(1,224*160)
        os = self._adiam["dos"].reshape(1,224*160)
        ou = self._adiam["dou"].reshape(1,224*160)
        
        h_ia_oa = CorrelationArrayToTHN(40, 60, ia[0][:], 70, 90, oa[0][:], 2)
        h_ia_ou = CorrelationArrayToTHN(40, 60, ia[0][:], 70, 90, ou[0][:], 2)
        h_os_ou = CorrelationArrayToTHN(70, 90, os[0][:], 70, 90, ou[0][:], 2)

        h_ia_oa.SetTitle("inner average vs. outer average")
        h_ia_ou.SetTitle("inner average vs. outer unsegmented")
        h_os_ou.SetTitle("outer segmented vs. outer unsegmented")
           
        c = TCanvas("c{}".format(self._iplot),"",1000,300)
        
        c.Divide(3,1)
        h_ia_oa.GetZaxis().SetNdivisions(5)
        h_ia_ou.GetZaxis().SetNdivisions(5)
        h_os_ou.GetZaxis().SetNdivisions(5)
        c.cd(1); self.FormatMapHist(h_ia_oa); h_ia_oa.Draw("colz")
        c.cd(2); self.FormatMapHist(h_ia_ou); h_ia_ou.Draw("colz")
        c.cd(3); self.FormatMapHist(h_os_ou); h_os_ou.Draw("colz")
        c.SaveAs("{}_holecorr.pdf".format(self._name))
        
    def DrawMapAlignment(self):
        self._iplot += 1
        c = TCanvas("c{}".format(self._iplot),"",900,600); c.Draw(); c.cd()

        self._hgain.SetMarkerColor(1)
        self._hgain.SetMarkerStyle(20)
        self._hgain.SetMarkerSize(0.1)
        self._hgain.Draw("colz")
        self._hgain.GetZaxis().SetRangeUser(0,3)
        f0,f1,f2,f3 = self.GetFrame()
        f0.Draw("same")
        f1.Draw("same")
        f2.Draw("same")
        f3.Draw("same")
        
        #self._hdiam[6].SetMarkerColor(2)
        #self._hdiam[6].SetMarkerStyle(20)
        #self._hdiam[6].SetMarkerSize(0.1)
        #self._hdiam[6].Draw("same")
        
        c.SaveAs('{}_alignment.png'.format(self._name))
        
    def CostForOptAlignment(self, shift):
        """
        The absolute value of the difference of the inner 
        densities of the two sides of the foil
        """
        cost = 0  
        cost += np.sum(np.abs(  self._adiam["nis"]-self._adiam["niu"] ))
        return cost
        
    def CheckOptMapAlignment(self):
        """
        Check and corrects the alignment of the optical measurement 
        of the two sides of the foil. Plots before and after correction data
        """
        self._iplot += 1
        c = TCanvas("c","",900,600); c.Draw(); 
        c.Divide(2, 2)


# IROC FOILS
gainSP = ["I-G1-005", "I-G1-006", "I-G1-007", "I-G4-003", "I-G1-008", "I-G1-025"]
# TRY different I-G4-003s???
#optSP  = ["I-G1-005-2_2D", "I-G1-006_2D", "I-G1-007-2_2D", "I-G4-003-2_2D", "I-G1-008_2D", "I-G1-025_2D"]
optSP = ["I-G1-005-2_opt", "I-G1-006_opt", "I-G1-007-2_opt", "I-G4-003-2_opt", "I-G1-008_opt", "I-G1-025_opt"]

gainLP = ["I-G2-004", "I-G2-005", "I-G3-004", "I-G2-015", "I-G3-008", "I-G3-009"]
optLP  = ["I-G2-004-2_opt", "I-G2-005-2_opt", "I-G3-004_opt", "I-G2-015_opt", "I-G3-008_opt", "I-G3-009-2_opt"]

# OROC FOILS
#gainO2_S = ["O2-G1-005",    "O2-G1-006",    "O2-G1-008",    "O2-G1-016",    "O2-G1-013",    "O2-G1-018",    "O2-G1-023",    "O2-G4-002",    "O2-G4-020"]
#optO2_S  = ["O2-G1-005_2D", "O2-G1-006_2D", "O2-G1-008_2D", "O2-G1-016_2D", "O2-G1-013_2D", "O2-G1-018_2D", "O2-G1-023_2D", "O2-G4-002_2D", "O2-G4-020_2D"]
#every except  and  018
gainO2_S = ["O2-G1-005",    "O2-G1-006",    "O2-G1-008",    "O2-G1-013",    "O2-G1-016",    "O2-G1-023",    "O2-G4-020",    "O2-G4-002"]
optO2_S  = ["O2-G1-005_opt", "O2-G1-006_opt", "O2-G1-008_opt", "O2-G1-013_opt", "O2-G1-016_opt", "O2-G1-023_opt", "O2-G4-020_opt", "O2-G4-002_opt"]
#only very good
#gainO2_S = ["O2-G1-006",    "O2-G1-008",    "O2-G1-016",   "O2-G1-023" ]
#optO2_S  = ["O2-G1-006_2D", "O2-G1-008_2D", "O2-G1-016_2D", "O2-G1-023_2D"]

#gainO2_S = ["O2-G1-013",    "O2-G1-018",    "O2-G1-023",    "O2-G4-002",    "O2-G4-020"]
#optO2_S  = ["O2-G1-013_2D", "O2-G1-018_2D", "O2-G1-023_2D", "O2-G4-002_2D", "O2-G4-020_2D"]

#gainO2_LP = ["O2-G2-007",      "O2-G2-021",    "O2-G2-022",    "O2-G2-027",    "O2-G3-026",    "O2-G3-022"]
#optO2_LP  = ["O2-G2-007-2_opt", "O2-G2-021_opt", "O2-G2-022_opt", "O2-G2-027_opt", "O2-G3-026_opt", "O2-G3-022_opt"]
gainO2_LP = ["O2-G2-007",       "O2-G2-027",    "O2-G3-026",    "O2-G3-022"]
optO2_LP  = ["O2-G2-007-2_opt", "O2-G2-027_opt", "O2-G3-026_opt", "O2-G3-022_opt"]
foils = []

if len(sys.argv)<2:
    print("No argument provided, processing default IROC-S foil")
    ftype=0
else:
    ftype=int(sys.argv[1])

types=["IROC-SP", "IROC-LP", "OROC2-SP", "OROC2-LP"]
print("Processing foils of type {}".format(types[ftype]))


if ftype==2:
    xmin, xmax = 0.6, 2
    for ifoil in range(len(optO2_S)-2):
        foils.append( Foil(ifoil, 0, gainO2_S[ifoil], optO2_S[ifoil]) )
if ftype==3:
    xmin, xmax = 0.8, 2.2
    for ifoil in range(len(optO2_LP)):
        foils.append( Foil(ifoil, 1, gainO2_LP[ifoil], optO2_LP[ifoil]) )
if ftype==1:
    xmin, xmax = 0.5, 1.8
    for ifoil in range(len(optLP)-1):
        foils.append( Foil(ifoil, 1, gainLP[ifoil], optLP[ifoil]) )
if ftype==0:    
    xmin, xmax = 0.6, 1.5
    for ifoil in range(len(gainSP)):
#    for ifoil in range(1):
        foils.append( Foil(ifoil, 0, gainSP[ifoil], optSP[ifoil]) )


coeffs = [ 0., 0.5, 0.5]

d0_individual = []
for i in range(len(foils)): d0_individual.append(90.5)
coeffs = coeffs + d0_individual

# bounds of each parameter:
bnds = ((-1, 1), (0.1, 1),(0.1, 1)) #, (-10, 0), (0, 10), (0, 10))
for ifoil in foils: bnds+=((30, 120),)
print("The limits of the parameters:")
print(bnds)
print("Starting minimalization")

def allcost(coeffs):
    cost = 0  
    
    for ifoil in foils:
        cost += np.sum(np.abs( ifoil._g_c - ifoil.GainFormula(coeffs)))
    return cost

# -----------------------------------------------------------
# Minimize linear combination and generate predicted gain from that
# Also draw the gain-predicted gain comparisons
# -----------------------------------------------------------
res = minimize(allcost, coeffs, bounds=bnds, method="L-BFGS-B")
#res = minimize(allcost, coeffs,  method="Nelder-Mead")
print(res.message)
print(res.x)

plotcoeff = res.x

for ifoil in foils: 
    ifoil.GeneratePredGain(plotcoeff)
    ifoil.GenerateCorrelation()
    ifoil.DrawGainMaps()
    #ifoil.DrawOptMaps(plotcoeff)
    #ifoil.DrawCorrelation()
    #ifoil.DrawHoleCorrelation()
    
# -----------------------------------------------------------
# draw correlations:
# -----------------------------------------------------------

#kWhite  = 0,   kBlack  = 1,   kGray    = 920,  kRed    = 632,  kGreen  = 416,
#kBlue   = 600, kYellow = 400, kMagenta = 616,  kCyan   = 432,  kOrange = 800,
#kSpring = 820, kTeal   = 840, kAzure   =  860, kViolet = 880,  kPink   = 900
colors = [601, 823, 1, 633, 920, 617, 861, 401, 432, 880, 417, 800, 900]
#colors = [ 601, 634, 417, 433, 635, 619, 603, 636, 435,     601, 634, 417, 433, 635, 619, 603, 636, 719, 435]
    
c = TCanvas("ccorr","",800,800)
p = TPad("p","",0,0,1,1)
p.SetLeftMargin(0.15)
p.SetBottomMargin(0.15)
p.SetRightMargin(0.05)
p.SetTopMargin(0.05)
p.Draw(); p.cd()
p.SetLogx()
p.SetLogy()
#f1 = TF1("f1","exp( [0]*(x-[1]) )", 10, 130 );
#f1.SetParameters(-0.0456, 80)
#gcorr.Fit("f1","RF","",65, 85)

f1 = TF1("f1","pol1",0,2)
f1.SetParameters(0,1); f1.SetLineColor(1); f1.SetLineStyle(1)
f1.SetTitle("1")

f2 = TF1("f2","pol1",0,2)
f2.SetParameters(0,1.2); f2.SetLineColor(1); f2.SetLineStyle(2)
f2.SetTitle("+20%")

f3 = TF1("f3","pol1",0,2)
f3.SetParameters(0,0.8); f3.SetLineColor(1); f3.SetLineStyle(2)
f3.SetTitle("-20%")

hcorr=TH2D("hcorr","",2,xmin,xmax,2,xmin,xmax)
gStyle.SetTitleFontSize(0.08)
gStyle.SetTextFont(42)
hcorr.GetXaxis().SetLabelSize(0.04)
hcorr.GetYaxis().SetLabelSize(0.04)
hcorr.GetZaxis().SetLabelSize(0.04)
hcorr.GetXaxis().SetTitleSize(0.06)
hcorr.GetYaxis().SetTitleSize(0.06)
hcorr.GetZaxis().SetTitleSize(0.07)
hcorr.GetXaxis().SetTitleOffset(1.1)
hcorr.GetYaxis().SetTitleOffset(1.1)
hcorr.GetZaxis().CenterTitle()
hcorr.GetXaxis().SetTitle("Predicted gain")
hcorr.GetXaxis().CenterTitle()
hcorr.GetYaxis().SetTitle("Measured gain")
hcorr.GetYaxis().CenterTitle()
hcorr.GetXaxis().SetNdivisions(2)
hcorr.GetYaxis().SetNdivisions(2)
#hcorr.GetXaxis().SetMoreLogLabels()
#hcorr.GetYaxis().SetMoreLogLabels()
hcorr.Draw()

l = TLegend(0.75, 0.2, 0.9, 0.56, "", "bnNDC")
l.SetBorderSize(0)
for ifoil in foils:
    ifoil._gcorr.Draw("P same")
    l.AddEntry(ifoil._gcorr, ifoil._gcorr.GetTitle(), "l")
l.Draw()
f1.Draw("same");f2.Draw("same");f3.Draw("same")

pt = TPaveText(0.16,0.85,0.8,0.94,"bnNDC")
aw = np.sum(plotcoeff)
#pt.AddText('Weights: inner: {:.1f}, outer seg. {:.1f}, outer unseg. {:.1f}'.format(plotcoeff[0]/aw, plotcoeff[1]/aw, plotcoeff[2]/aw));
pt.SetFillColor(0)
pt.SetLineColor(0)
pt.SetLineWidth(0)
pt.SetTextSize(0.03)
pt.SetTextAlign(12)
pt.SetTextFont(42)
pt.Draw()

#thesis=ThisThesis(0.72, 0.92, 0.06)
thesis=ThisThesis(0.17, 0.9, 40)
thesis.Draw()
c.SaveAs("corr-{}.pdf".format(types[ftype]))
print(f1.Eval(70), f1.Eval(80))

# -----------------------------------------------------------
# draw residuals:
# -----------------------------------------------------------

c = TCanvas("cres","",800,800)
p = TPad("p","",0,0,1,1)
p.SetLeftMargin(0.15)
p.SetBottomMargin(0.15)
p.SetTopMargin(0.05)
p.SetRightMargin(0.05)
p.Draw(); p.cd()

foils[0]._hocc.GetXaxis().SetLabelSize(0.04)
foils[0]._hocc.GetYaxis().SetLabelSize(0.04)
foils[0]._hocc.GetZaxis().SetLabelSize(0.04)
foils[0]._hocc.GetXaxis().SetTitleSize(0.06)
foils[0]._hocc.GetYaxis().SetTitleSize(0.06)
foils[0]._hocc.GetZaxis().SetTitleSize(0.07)
foils[0]._hocc.GetXaxis().SetTitleOffset(1.1)
foils[0]._hocc.GetYaxis().SetTitleOffset(1.1)
foils[0]._hocc.GetZaxis().CenterTitle()
foils[0]._hocc.GetXaxis().SetTitle("Residual")
foils[0]._hocc.GetXaxis().CenterTitle()
foils[0]._hocc.GetYaxis().SetTitle("")
foils[0]._hocc.GetYaxis().CenterTitle()
foils[0]._hocc.GetXaxis().SetNdivisions(10)
foils[0]._hocc.GetYaxis().SetNdivisions(5)
foils[0]._hocc.GetXaxis().SetRangeUser(0., 5)

for i in range(len(foils)):
    foils[i]._hocc.SetLineColor(colors[i])
    foils[i]._hocc.SetTitle("") #intentional for drawing, will be set later
    foils[i]._hocc.Draw("hist") if i==0 else foils[i]._hocc.Draw("hist same")
    #foils[i]._hocc.SetTitle(foils[i]._name)

l = TLegend(0.65, 0.2, 0.98, 0.9, "", "bnNDC")
l.SetBorderSize(0)
for i in range(len(foils)):
    l.AddEntry(foils[i]._hocc, foils[i].GetResText(), "l")
l.Draw()

thesis=ThisThesis(0.76, 0.95, 40)
thesis.Draw()

c.SaveAs("res-{}.pdf".format(types[ftype]))



raw_input("Press Enter to continue...")
