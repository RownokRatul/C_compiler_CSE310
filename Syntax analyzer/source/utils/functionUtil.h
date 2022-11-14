#include<iostream>
#include<cstdlib>
#include<cctype>
#include<sstream>
#include<string>
#include<vector>
#include "arrayUtil.h"
#include "typeUtil.h"
using namespace std;

extern symbolTable *table;
extern stringUtil *strAnalyzer;
extern arrayUtil *arrAnalyzer;
extern semanticErrorHandler *semanticErr;

extern FILE* logFile;
extern FILE* errorFile;

extern int line_count;
extern int error_count;


class functionUtil {

	public:

		string checkValidFunctionCall(string id, string argType) {
			//cout<<argList<<endl<<argType<<endl;
			vector<string> args = strAnalyzer->splitString(argType, ',');
			symbolInfo* sym = table->lookupGlobal(id);
			if(sym == NIL)	{
				//Error
				semanticErr->undeclaredFunctionCall(id);
				return UNK;
			}
			else {
				if(sym->getVariableType() == "FUNCTION") {
					functionInfo* func = sym->getFunctionInfo();
					if(matchFunctionArguments(func->paramList, args)) {
						return sym->getFunctionInfo()->retType;
					}
					else {
						//Error Argument Mismatch
						semanticErr->mismatchArguments(id, func->paramList, args);
						return UNK;
					}
				}
				else {
					//Error 
					semanticErr->notaFunction(id);
					return UNK;
				}
			}
		}

		bool matchFunctionArguments(vector<string> &oldArgs, vector<string> &newArgs) {
			if(oldArgs.size() == newArgs.size()) {
				int len = oldArgs.size();
				for(int i=0;i<len;i++) {
					if(strAnalyzer->toUpper(strAnalyzer->splitString(oldArgs[i], ' ')[0]) == "FLOAT" && newArgs[i] == "INT") {
						//typePromotion applied here
					}
					else if(strAnalyzer->toUpper(strAnalyzer->splitString(oldArgs[i], ' ')[0]) != newArgs[i]) {
						//cout<<"returning in "<<newArgs[i]<<endl;
						return false;
					}
				}
			}
			else {
				return false;
			}
			return true;
		}


		bool checkParamterValidity(string &params) {
			if(params == "VOID") {
				return true;
			}
			vector<string> newParams = strAnalyzer->splitString(params, ',');
			for(string each: newParams) {
				vector<string> type_name = strAnalyzer->splitString(each, ' ');
				if(type_name[0] == "VOID") {
					return false;
				}
			}
			return true;
		}

		bool matchFunctionParams(vector<string> oldParams, vector<string> newParams) {
			if(oldParams.size() == newParams.size()) {
				int len = oldParams.size();
				for(int i=0;i<len;i++) {
					if(arrAnalyzer->validArray(oldParams[i]) != arrAnalyzer->validArray(newParams[i])) {
						return false;
					}
					vector<string> x = strAnalyzer->splitString(oldParams[0], ' ');
					vector<string> y = strAnalyzer->splitString(newParams[0], ' ');
					if(x[0] != y[0]) {
						return false;
					} 
				}
			}
			else {
				return false;
			}
			return true;
		}

		void pushParamsScopeTable(vector<string> params, string name) {
			for(string each: params) {
				vector<string> temp = strAnalyzer->splitString(each, ' ');
				//cout<<(temp[0] == "VOID")<<endl;
				if(temp.size() != 2) {
					// Error parameter name not included
					semanticErr->parameterNameNotGiven(name);
				}
				else if(!table->insert(temp[1], "ID",strAnalyzer->toUpper(temp[0]))) {	
					//Error duplicate variable declaration
					semanticErr->multipleDeclarationVariable(temp[1]);
				}
			}
		}

		void handleFunctionDefinition(string name, string retType, string params) {
			symbolInfo* look = table->lookupGlobal(name);
			bool validParam = checkParamterValidity(params);
			if(!validParam) {
				semanticErr->voidTypeParameter(name);
				//return;
			}
			vector<string> newParams;
			if(params != "VOID") {
				newParams = strAnalyzer->splitString(params, ',');
			}
			if(look != NIL) {
				if(look->getVariableType() == "FUNCTION") {
					if(!look->getFunctionInfo()->defined) {
						if(look->getFunctionInfo()->retType != retType) {
							//Error retType did not match
							semanticErr->mismatchReturnType(name, look->getFunctionInfo()->retType, retType);
						}
						if(!matchFunctionParams(look->getFunctionInfo()->paramList, newParams)) {
							//Error parametes did not match
							semanticErr->mismatchParameter(name, look->getFunctionInfo()->paramList, newParams);
						}
						//fprintf(logFile, "DEBUG\n");
						look->getFunctionInfo()->defined = true;
						table->enterScope();
						pushParamsScopeTable(newParams, name);
						//table->printNonEmptyRows(logFile);
					}
					else {
						//Error multiple definition of same named function
						semanticErr->multipleDeclarationFunction(name);
						table->enterScope();
						pushParamsScopeTable(newParams, name);
					}
				}
				else {
					//Error some else variable already named this
					semanticErr->multipleDeclarationVariable(name);
					table->enterScope();
					pushParamsScopeTable(newParams, name);
				}
			}
			else if(validParam) {
				functionInfo* func = new functionInfo();
				func->retType = retType;
				func->name = name;
				func->paramList = newParams;
				func->defined = true;
				table->insert(name, "ID", func);
				table->enterScope();
				pushParamsScopeTable(newParams, name);
				//table->printNonEmptyRows(logFile);
			}
		}
 
		void handleFunctionDeclaration(string name, string retType, string params) {
			symbolInfo* look = table->lookupGlobal(name);
			vector<string> newParams = strAnalyzer->splitString(params, ',');
			bool validParam = checkParamterValidity(params);
			if(!validParam) {
				semanticErr->voidTypeParameter(name);
				//return;
			}
			if(look != NIL) {
				//Error function already declared
				semanticErr->multipleDeclarationVariable(name);
			}
			else if(validParam){
				functionInfo* func = new functionInfo();
				func->retType = retType;
				func->name = name;
				func->paramList = newParams;
				func->defined = false;
				table->insert(name, "ID", func);
				//table->printNonEmptyRows(logFile);
			}
		}

};