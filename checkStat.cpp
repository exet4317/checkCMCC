#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <iomanip>
#include "Utils.h"
using namespace std;

// "sshpass -f pwfile scp smingtsai@140.114.94.235:/home/smingtsai/clin/"MrCosmoMC"/CMCCLogfile.log ."

int total = 0;
int done = 0;
int total_points = 0;
int done_points = 0;
double channelNum, log_mx;

struct minFormat
{
    double col[13];
};

void processLog(string name)
{
    int l = 0; // line number
    int startAt = -999;
    int endAt = -999;
    int err_line = 0;
    int done_line = 0;
    double mx, sv_min, sv_max, sv_step, mode, pts;
    ifstream fin;
    string tmp;
    string filename = "./CMCCLogs/" + name + ".log";
    fin.open(filename.c_str());

    string fileline;
    string CMC_cmd;

	Utils extFunction;
    int cmdAt;
    while (getline(fin, fileline)){

        if (fileline == "// ---- CMCC START ---- //") {
            err_line = 0;
            done_line = 0;
            startAt = l;
        } else if (l == startAt + 2) {
            cmdAt = l;
            CMC_cmd = fileline;
            stringstream ss(CMC_cmd.c_str());
            ss >> tmp;
            ss >> tmp;

            ss >> mx;
            ss >> sv_min;
            ss >> sv_max;
            ss >> sv_step;
            ss >> mode;
            channelNum = mode;
            log_mx = mx;
        } else if (fileline == "// ---- CMCC END ---- //") {
            endAt = l;
        } else if (fileline[fileline.length() - 1] == '.') {
            if  (fileline[fileline.length() - 2] == '0' && fileline[fileline.length() - 3] == ' ') {
                done_line ++;
            }
            if (fileline[fileline.length() - 2] == 'd') {
            }
            if (fileline[fileline.length() - 2] == '.') {
            }
            if (fileline[fileline.length() - 2] >= '1' && fileline[fileline.length() - 2] <= '9') {
                err_line ++;
            }
        }

        l++;

    }
    fin.close();
    

    int tot_pts = 0;
    if (sv_step == 0) tot_pts = 1;
    else {
		for (double dd = sv_min; ; dd += sv_step){
			if ( extFunction.tooSimilar(dd, sv_max) || dd > sv_max ){
				tot_pts ++;
				break;
			}
			tot_pts ++;
		}
	}

    cout << endl << "# -------------------------------------------------------------------------" << endl;
    cout << endl << "  Filename = " << filename << endl;
    cout << "     l     = " << l << endl;
    cout << "     Start = " << startAt << endl;
    cout << "     End   = " << endAt << endl;
    cout << "     cmdAt = " << cmdAt << endl;
    cout << "     cmd   = " << CMC_cmd << endl;
    
    if (startAt < endAt) {
        cout << "     Done? = yes, successful = " << done_line << "/" << tot_pts << ", failed = " << err_line << "/" << tot_pts << "." << endl;
        done ++;
    } else {
        cout << "     Done? = no, successful = " << done_line << "/" << tot_pts << ", failed = " << err_line << "/" << tot_pts << "." << endl;
    }
	total_points += tot_pts;
	done_points += done_line;
}

int countProcess()
{
    int num = 0;
    int X = -999;
    string cmd;
    string title1 = "sshpass -f pwfile scp smingtsai@140.114.94.235:/home/smingtsai/clin/";
    string title2 = "/CMCCLogfile.log ./CMCCLogs/";
    string title3 = ".log ";
    string name = "MrCosmoMC";
    string title4 = "> /dev/null 2>&1";

	cmd = title1 + name + "1" + title2 + name + "1" + title3 + title4;
    //cout << cmd << endl;

    X = system(cmd.c_str());
    
    while ( X == 0 ){
        num ++;
		name = "MrCosmoMC" + to_string(num);
        processLog(name.c_str());

        name = "MrCosmoMC" + to_string(num+1);
        cmd = title1 + name + title2 + name + title3 + title4;
        X = system(cmd.c_str());
    }
    
    //return X;
    return num;
}




int outDataLine = 0;
void processMinData(string N)
{
    Utils extFunction;
    minFormat outData[100];
    minFormat in;
    int X = -999;
    int num = 0;
    bool found = 0;
    string name;
    string title1 = "sshpass -f pwfile scp smingtsai@140.114.94.235:/home/smingtsai/clin/MrCosmoMC";
    string target = "./MinData/chisqmin_";
    string title3 = ".dat > /dev/null 2>&1";
    string cmd = title1 + "1" + "/" + N + " " + target + "1" + title3;

    X = system(cmd.c_str());
    
    while ( X == 0 ){
        num ++;
        ifstream fin;
        string finName = target + to_string(num) + ".dat";
        fin.open(finName.c_str());

        while (fin >> in.col[0]){
            found = 0;
            for (int i = 1; i < 12; i++) fin >> in.col[i];

            for (int i = 0; i < outDataLine; i++) {
                if ( extFunction.tooSimilar(in.col[1], outData[i].col[1])){ 
                    found = 1;
                    if(in.col[5] < outData[i].col[5]) {
                        for (int j = 0; j < 12; j++) outData[i].col[j] = in.col[j];
                        break;
                    }
                }
            }
            if (found == 0) {
                for (int j = 0; j < 12; j++) outData[outDataLine].col[j] = in.col[j];
                outDataLine ++;
            }            
        }
        fin.close();
        cout << "    " << target << num << ".dat" << endl;

        name = to_string(num+1);
        cmd = title1 + name + "/" + N + " " + target + name + title3;
        X = system(cmd.c_str()); 
    }
    cout << endl;
    cout << "  Done." << endl << endl;
    string outname = "./MinData/chisqmin_";

    if (channelNum > 0.8 && channelNum < 1.2) {
        outname += "ee_";
    } else if (channelNum > 1.8 && channelNum < 2.2) {
        outname += "bb_";
    }
    outname += to_string(pow(10, log_mx)) + "GeV.dat";
    ofstream fout;
    fout.open(outname);
    for (int i = 0; i < outDataLine; i++) {
        for (int j = 0; j < 12; j++ ) {
            if ( j == 1 ) {
                fout << scientific << outData[i].col[j] << " ";
            } else {
                fout << fixed << setprecision(8) << outData[i].col[j] << " ";
            }
        }
        fout << endl;
    }
    fout.close();
}

int main()
{
    Utils extFunc;
    system("mkdir CMCCLogs > /dev/null 2>&1");
    system("mkdir MinData > /dev/null 2>&1");
    total = countProcess();
    cout << endl << "# -------------------------------------------------------------------------" << endl << endl;
	
    cout << "  Points finished = " << done_points << "/" << total_points << "." << endl;
	cout << "  Overall status  = " << done << "/" << total << "." << endl << endl;
	if (done == total){
        cout << "# -------------------------------------------------------------------------" << endl << endl;
        cout << "  Processing chisqmin data : " << endl << endl;
		string minTableName = "chisqmin_" + extFunc.num2ch(channelNum) + "_" + to_string(pow(10, log_mx)) + "gev.dat";
        processMinData(minTableName);
	}

    return 0;
}
