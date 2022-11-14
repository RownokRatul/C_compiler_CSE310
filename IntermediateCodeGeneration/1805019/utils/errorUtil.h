#include<iostream>
#include<cstdlib>
#include<cctype>
#include<sstream>
#include<string>
#include "stringUtil.h"
using namespace std;

extern FILE* errorFile;
extern int error_count;
extern int line_count;
extern stringUtil *strAnalyzer;

class semanticErrorHandler {

	public:

		void undeclaredVariable(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: undeclared variable %s\n\n", line_count, err.c_str());
		} 

		void undeclaredFunctionCall(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Function not Declared %s\n\n", line_count, err.c_str());
		}

		void notaFunction(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: %s not a Function\n\n", line_count, err.c_str());
		}

		void voidTypeParameter(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: \"void\" type Parameter declared, \"%s\" Function declaration failed\n\n", line_count, err.c_str());
		}

		void voidTypeVariable(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: \"void\" type Variable declared\n\n", line_count, err.c_str());
		}

		void mismatchParameter(string name, vector<string> &oldParams, vector<string> &newParams) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Mismatch Parameter in function %s\n", line_count, name.c_str());
			fprintf(errorFile, "Expected: (");
			int len = oldParams.size();
			for(int i=0;i<len;i++) {
				fprintf(errorFile, "%s,", strAnalyzer->splitString(oldParams[i], ' ')[0].c_str());
				if(i != (len-1)) {
					fprintf(errorFile, ",");
				}
			}
			fprintf(errorFile, ") Found: (");
			len = newParams.size();
			for(int i=0;i<len;i++) {
				fprintf(errorFile, "%s,", strAnalyzer->splitString(newParams[i], ' ')[0].c_str());
				if(i != (len-1)) {
					fprintf(errorFile, ",");
				}
			}
			fprintf(errorFile, ")\n\n");
		}

		void mismatchArguments(string name, vector<string> &oldArgs, vector<string> &newArgs) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Mismatch Argument in function %s\n", line_count, name.c_str());
			fprintf(errorFile, "Expected: (");
			int len = oldArgs.size();
			for(int i=0;i<len;i++) {
				fprintf(errorFile, "%s", strAnalyzer->toUpper(strAnalyzer->splitString(oldArgs[i], ' ')[0]).c_str());
				if(i != (len-1)) {
					fprintf(errorFile, ",");
				}
			}
			fprintf(errorFile, "), Found: (");
			len = newArgs.size();
			for(int i=0;i<len;i++) {
				fprintf(errorFile, "%s", strAnalyzer->toUpper(strAnalyzer->splitString(newArgs[i], ' ')[0]).c_str());
				if(i != (len-1)) {
					fprintf(errorFile, ",");
				}
			}
			fprintf(errorFile, ")\n\n");
		}

		void mismatchReturnType(string name, string oldRet, string newRet) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Return Type mismatch in function %s\nExpected: %s, Found %s\n\n",line_count, name.c_str(), oldRet.c_str(), newRet.c_str());
		}

		void multipleDeclarationFunction(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Multiple declaration of function %s\n\n",line_count, err.c_str());
		}

		void multipleDeclarationVariable(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Multiple declaration of %s, cannot resolve\n\n",line_count, err.c_str());
		}

		void parameterNameNotGiven(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Parameter type has no variable name in Function %s\n\n",line_count, err.c_str());
		}

		void expectFunctionCall(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Expecting argument list (...) after %s (function type)\n\n",line_count, err.c_str());
		}

		void expectArray(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Expecting index [..] after %s (array type)\n\n",line_count, err.c_str());	
		}

		void voidTypeIncompatible(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: void type function %s usage in expression\n\n", line_count, err.c_str());
		}

		void unknownTypeIncompatibe(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: unknown type %s usage in expression\n\n", line_count, err.c_str());
		}

		void notAnArray(string err) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: %s not an Array type\n\n", line_count, err.c_str());
		}

		void invalidAssignmentOperator(symbolInfo* l, symbolInfo* r) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Invalid Assignment Operator usage : %s = %s\n\n", line_count, l->getType().c_str(), r->getType().c_str());
		}

		void floatCastToInt(string err) {
			error_count++;
			fprintf(errorFile, "Warning at line no %d: Type mismatch:: FLOAT type %s casting to INT\n\n",line_count, err.c_str());
		}

		void invalidReturnType(string actual, string given, string funcName) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Invalid Return type for function \"%s\" : Expected %s, Given %s\n\n",line_count, funcName.c_str(), actual.c_str(), given.c_str());
		}

		void arrayIndexError(string err, string givenType) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Array Index type error in %s. Expected Type: INT, Given Type: %s\n\n",line_count, err.c_str(), givenType.c_str());
		}

		void modulusOperatorUsage(string left, string right) {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Operands of MODULUS operator must be INT type. Given Type: %s %% %s\n\n",line_count, left.c_str(), right.c_str());
		}

		void modulusByZero() {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Modulus by Zero\n\n",line_count);
		}

		void invalidExpressionIF() {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Invalid Expression usage inside if-statement. Must be INT or FLOAT type.\n\n",line_count);
		}

		void invalidExpressionWHILE() {
			error_count++;
			fprintf(errorFile, "Error at line no %d: Invalid Expression usage inside while-statement. Must be INT or FLOAT type.\n\n",line_count);
		}
};