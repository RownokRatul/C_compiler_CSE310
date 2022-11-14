#ifndef _optimize_h_
#define _optimize_h_

#include<iostream>
#include<vector>
#include<stack>
#include<cstdlib>
#include<fstream>
using namespace std;

class codeOptimizer {

        fstream optimized;
        fstream toOptimize;
        string prev;
        string next;
        string optimizedPrev;
        string optimizedNext;
        string whiteSpace = " \n\t\f\b\r";
        int stackedAddition;
        bool parsingStackedAdd;

        string ltrim(const string &s) {
            size_t start = s.find_first_not_of(whiteSpace);
            return (start == string::npos) ? "" : s.substr(start);
        }

        string rtrim(const string &s) {
            size_t end = s.find_last_not_of(whiteSpace);
            return (end == string::npos) ? "" : s.substr(0, end+1);
        }

        string trim(const string &s) {
            return rtrim(ltrim(s));
        }

        bool isNumber(const string &s) {
            int len = s.length();
            for(int i=0;i<len;i++) {
                if((s[i] != '-') && (s[i] > '9' || s[i] < '0')) {
                    return false;
                    
                }
            }
            return true;
        }

        bool startsWith(string x, string prefix) {
            return (x.rfind(prefix, 0) == 0) ? true : false;
        }

        bool endsWith(string x, string suffix) {
            return x.size() >= suffix.size() && (0 == x.compare(x.size()-suffix.size(), suffix.size(), suffix));
        }

        vector<string> splitString(string ls, char delim) {
			stringstream stream(ls);
			vector<string> v;
			string temp;
			while(getline(stream, temp, delim)) {
				v.push_back(temp);
			}
			return v;
		}

        bool checkPushPop(vector<string> &p, vector<string> &n) {
            if(p.size() && n.size()) {
                if((p[0] == "PUSH" && n[0] == "POP") || (p[0] == "POP" && n[0] == "PUSH")) {
                    return p[1] == n[1];
                }
            }
            return false;
        }

        bool checkMovRedundancy(vector<string> &p, vector<string> &n) {
            if(p.size() && n.size()) {
                if(p[0] == "MOV" && n[0] == "MOV") {
                    vector<string> regp = splitString(p[1], ',');
                    vector<string> regn = splitString(n[1], ',');
                    if((regp[0] == n[2]) && (regn[0] == p[2])) {
                        return true;
                    }
                }
            }
            return false;
        }

        bool checkStackedAdd(vector<string> &p, vector<string> &n) {
            if(p.size() && n.size()) {
                if(p[0] == "ADD" && n[0] == "ADD") {
                    //cout<<isNumber("-6")<<endl;
                    if((p[1] == n[1]) && isNumber(p[2]) && isNumber(n[2])) {
                        stackedAddition += stoi(p[2]);
                        return true;
                    }
                }
            }
        }

        void peepholeOptimize() {
            vector<string> p = splitString(prev, ' ');
            vector<string> n = splitString(next, ' ');
            parsingStackedAdd = false;
            //push pop
            if(checkPushPop(p, n)) {
                if(optimized.is_open()) {
                    optimized<<"; Optimization: consecutive PUSH POP"<<"\n";
                    optimizedPrev = ";" + optimizedPrev;
                    optimizedNext = ";" + optimizedNext;
                }
            }
            //mov redundancy
            else if(checkMovRedundancy(p, n)) {
                if(optimized.is_open()) {
                    optimized<<"; Optimization: redundant MOV"<<"\n";
                    optimizedNext = ";" + optimizedNext;
                }
            }
            //multiple add or sub
            else if(checkStackedAdd(p, n)) {
                if(optimized.is_open()) {
                    optimizedPrev = ";" + optimizedPrev;
                    parsingStackedAdd = true;
                }
            }
            
            if(!parsingStackedAdd && stackedAddition) {
                stackedAddition += stoi(p[2]);
                optimized<<(";"+optimizedPrev)<<endl;
                int t = optimizedPrev.find_first_not_of("\t");
                optimized<<"; Optimization: stacked ADD"<<"\n";
                while(t--) {
                    optimized<<"\t";
                }
                optimized<<"ADD "+p[1]+" "+to_string(stackedAddition)<<endl;
                stackedAddition = 0;
            }
            else if(optimizedPrev != "") {
                optimized<<optimizedPrev<<endl;
            }
        }

    public: 

        codeOptimizer() {
            optimized.open("optimized_code.asm", ios::out);
            stackedAddition = 0;
            parsingStackedAdd = false;
        }


        void optimize(string name) {
            toOptimize.open(name, ios::in);
            getline(toOptimize, optimizedPrev);
            prev = trim(optimizedPrev);
            //optimized<<prev<<endl;
            while(getline(toOptimize, optimizedNext)) {
                next = trim(optimizedNext);
                if(next != "" && next[0] != ';') {
                    //cout<<prev<<"\n"<<next<<endl;
                    peepholeOptimize();
                    prev = next;
                    optimizedPrev = optimizedNext;
                }
                else {
                    //optimized<<optimizedNext<<endl;
                }
                //cout<<optimizedNext<<endl;
            }
            optimized<<optimizedPrev<<endl;
        }


};

#endif