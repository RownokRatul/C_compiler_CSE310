#include<iostream>
#include<cstdlib>
#include<cctype>
#include<sstream>
#include<string>
#include<vector>
using namespace std; 

class stringUtil {

	public: 

		bool startsWith(string t, string prefix) {
			return t.rfind(prefix, 0) == 0 ? true : false;
		}

		string toLower(string s) {
			string temp = "";
			for(int i=0;i<s.length();i++) {
				temp += tolower(s[i]);
			}
			return temp;
		}

		string toUpper(string s) {
			string temp = "";
			for(int i=0;i<s.length();i++) {
				temp += toupper(s[i]);
			}
			return temp;
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
};