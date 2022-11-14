#include<iostream>
#include<cstdlib>
#include<cctype>
#include<sstream>
#include<string>
#include<vector>
#include "symbolTable/1805019_symbolTable.h"
using namespace std; 

class arrayUtil {
	
	public:

		bool arrayIndexCheck(string type) {
			return type == "INT" ? true : false;
		}

		bool validArray(string id) {
			return (id.find('[') == string::npos) ? false : true;
		}

		arrayInfo* extractArrayInfo(string id) {
			string name = "";
			string sz = "";
			int i = 0;
			int len = id.length();
			while(i < len) {
				if(id[i] == '[') {
					i++;
					break;
				}
				name += id[i];
				i++;
			}
			while(i < len) {
				if(id[i] == ']') {
					break;
				}
				sz += id[i];
				i++;
			}
			int arrSz = atoi(sz.c_str());
			if(arrSz <= 0) {
				//handle SEMANTIC Error;
			} 
			arrayInfo* info = new arrayInfo();
			info->arrName = name;
			info->arrSize = arrSz;
			return info;
		}
};
