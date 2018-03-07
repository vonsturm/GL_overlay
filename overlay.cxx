/* Author:       Katharina von Sturm
 * Usage:        ./overlay <isotope> <legendfile> <gerdafile> <gerdafile>
 * Compilation:  g++ $(root-config --cflags) overlay.cxx -o overlay $(root-config --libs)
 */

// C/c++
#include <cstdlib>
#include <string>
#include <cstdio>

// root
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TParameter.h"

using namespace std;

int main( int argc, char * argv[] )
{
    string location   = argv[1];
	string isotope    = argv[2];
	string fn_legend  = argv[3];
	string fn_gerda_1 = argv[4];

    // control output
    cout << "*******************" << endl;
    cout << "Location: " << location << endl;
    cout << "Isotope: " << isotope << endl;
    cout << "Legend file: " << fn_legend << endl;
    cout << "Gerda file1: " << fn_gerda_1 << endl;

    // open legend file and get histograms
	TFile * f_legend  = new TFile( fn_legend.c_str(), "READ" );
    TH1D  * legend    = (TH1D*) ( (TCanvas*) f_legend->Get("c1") )->GetListOfPrimitives()->At(1);
    TH1D  * legend_AC = (TH1D*) ( (TCanvas*) f_legend->Get("c1") )->GetListOfPrimitives()->At(2);

	// open gerda file(s) and get histograms
    TFile * f_gerda_1 = new TFile( fn_gerda_1.c_str(), "READ" );
    TFile * f_gerda_2;
    double NumberOfPrimariesEdep_g1e = ( (TParameter<long> *) f_gerda_1 -> Get( "NumberOfPrimariesEdep" ) ) -> GetVal();
    double NumberOfPrimariesCoin_g1c = ( (TParameter<long> *) f_gerda_1 -> Get( "NumberOfPrimariesCoin" ) ) -> GetVal();

    cout << "\tPrim edep: " << NumberOfPrimariesEdep_g1e << endl;
    cout << "\tPrim coin: " << NumberOfPrimariesCoin_g1c << endl;

    TH1D  * gerda_AC = (TH1D*) f_gerda_1 -> Get( "M1_enrBEGe" ) -> Clone( "gerda_post_AC" ); gerda_AC -> Reset();
    gerda_AC -> Add( (TH1D*) f_gerda_1 -> Get( "M1_enrBEGe" ), 1./NumberOfPrimariesEdep_g1e );
    gerda_AC -> Add( (TH1D*) f_gerda_1 -> Get( "M1_enrCoax" ), 1./NumberOfPrimariesEdep_g1e );

    TH1D  * gerda = (TH1D*) f_gerda_1 -> Get( "M2_enrE1andE2" ) -> Clone( "gerda_pre_AC" ); gerda -> Reset();
    gerda    -> Add( (TH1D*) f_gerda_1 -> Get( "M2_enrE1andE2" ), 1./NumberOfPrimariesCoin_g1c );

    // A224_Z88 : Bi212 + Tl208
    // A222_Z86 : Pb214 + Bi214
    if( isotope == "A224_Z88" || isotope == "A222_Z86" )
    {
        double BR = 1.;
        if( isotope == "A224_Z88" ) BR = 0.3593;

        string fn_gerda_2 = argv[5];
        cout << "Gerda file2: " << fn_gerda_2 << endl;

        f_gerda_2 = new TFile( fn_gerda_2.c_str(), "READ" );
        double NumberOfPrimariesEdep_g2e = ( (TParameter<long> *) f_gerda_2 -> Get( "NumberOfPrimariesEdep" ) ) -> GetVal();
        double NumberOfPrimariesCoin_g2c = ( (TParameter<long> *) f_gerda_2 -> Get( "NumberOfPrimariesCoin" ) ) -> GetVal();

        cout << "\tPrim edep: " << NumberOfPrimariesEdep_g2e << endl;
        cout << "\tPrim coin: " << NumberOfPrimariesCoin_g2c << endl;

        gerda_AC -> Add( (TH1D*) f_gerda_2 -> Get( "M1_enrBEGe" ), BR/NumberOfPrimariesEdep_g2e );
        gerda_AC -> Add( (TH1D*) f_gerda_2 -> Get( "M1_enrCoax" ), BR/NumberOfPrimariesEdep_g2e );

        gerda    -> Add( (TH1D*) f_gerda_2 -> Get( "M2_enrE1andE2" ), BR/NumberOfPrimariesCoin_g2c );
    }

    gerda -> Add( gerda_AC );

    // set names and labels
    gerda     -> SetName( "gerda_pre_AC" );
    gerda_AC  -> SetName( "gerda_post_AC" );
    legend    -> SetName( "legend_pre_AC" );
    legend_AC -> SetName( "legend_post_AC" );

    gerda     -> SetTitle( Form( "gerda %s;energy [keV];counts / decay / keV", isotope.c_str() ) );
    gerda_AC  -> SetTitle( Form( "gerda granularity %s;energy [keV];counts / decay / keV", isotope.c_str() ) );
    legend    -> SetTitle( Form( "legend %s;energy [keV];counts / decay / keV", isotope.c_str() ) );
    legend_AC -> SetTitle( Form( "legend granularity %s;energy [keV];counts / decay / keV", isotope.c_str() ) );

    gerda     -> SetLineColor( kBlack );
    gerda_AC  -> SetLineColor( kRed );
    legend    -> SetLineColor( kAzure );
    legend_AC -> SetLineColor( kOrange-3 );

    gerda -> GetXaxis() -> SetRangeUser( 150., 3500. );

    string cname = Form("%s_%s",location.c_str(),isotope.c_str());
    TCanvas * c = new TCanvas( cname.c_str(), cname.c_str(), 750, 500 );
    gerda     -> Draw( "hist" );
    gerda_AC  -> Draw( "histsame" );
    legend    -> Draw( "histsame" );
    legend_AC -> Draw( "histsame" );
    gPad -> SetLogy();

    TLegend * l = new TLegend( 0.1,0.1,0.3,0.3 );
    l -> AddEntry( gerda,     "GERDA",              "l" );
    l -> AddEntry( gerda_AC,  "GERDA granularity",  "l" );
    l -> AddEntry( legend,    "LEGEND",             "l" );
    l -> AddEntry( legend_AC, "LEGEND granularity", "l" );
    l -> Draw();

    TFile * outfile = new TFile( Form("GL_%s_%s.root",location.c_str(),isotope.c_str()), "RECREATE" );
    c -> Write();
    gerda     -> Write();
    gerda_AC  -> Write();
    legend    -> Write();
    legend_AC -> Write();
    outfile   -> Close();

    f_gerda_1 -> Close();
    if(f_gerda_2) f_gerda_2 -> Close();
    f_legend  -> Close();

    cout << "*******************" << endl;

	return 0;
}
