#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<cstdlib>
#include<cctype>
#include "errorUtil.h"
using namespace std;

extern semanticErrorHandler *semanticErr;

class typeConverter {

	public:

		string variableTypeSetter(symbolInfo* sym) {
			string type = UNK;
			if(sym->getVariableType() == "ARRAY") {
				semanticErr->expectArray(sym->getName());
			}
			else if(sym->getVariableType() == "FUNCTION"){
				semanticErr->expectFunctionCall(sym->getName());
			}
			else {
				//cout<<sym->getVariableType();
				type = sym->getVariableType();
			}
			return type;
		}

		string typePromotion(string left, string right) {
			if(left == UNK || right == UNK) {
				return UNK;
			}
			if(right == "FLOAT" || left == "FLOAT") {
				return "FLOAT";
			}
			else {
				return "INT";
			}
		}

		void typeResolve(symbolInfo* sym, string targetType) {
			if(sym->getType() == "VOID") {
				semanticErr->voidTypeIncompatible(sym->getName());
				sym->setNameType(sym->getName(), UNK);
			}
			if(sym->getType() == UNK) {
				
			}
			else {
				sym->setNameType(sym->getName(), targetType);
			}
		}

};