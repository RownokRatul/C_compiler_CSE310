#ifndef _codeGen_h_
#define _codeGen_h_

#include<iostream>
#include<vector>
#include<string>
#include<stack>
#include "symbolTable/1805019_symbolTable.h"
using namespace std;

extern symbolTable* table;
extern FILE* codeFile;
extern FILE* tempCode;
extern FILE* printlnFile;
extern int indentCount;

extern int localVariableOffset;
extern int parameterOffset;

stack<pair<string,string> > labels;

class intermediateCodeGenerator {

        long long labelNum;
        long long tempNum;
        pair<string,string> l1;
        pair<string,string> l2;
        

    public: 

        intermediateCodeGenerator() {
            labelNum = 0;
            tempNum = 0;
        }

        string newLabel() {
            labelNum++;
            return "_LABEL_" + to_string(labelNum);
        }

        string newTemp() {
            tempNum++;
            return "_TEMP_" + to_string(tempNum);
        }

        void appendAllCode() {
            fputs(".CODE\n\n", codeFile);
	        fclose(tempCode);
            FILE* printer = fopen("utils/println.asm", "r");
            tempCode = fopen("tempCode.asm", "r");
	        char c;
	        while((c = fgetc(printer)) != EOF) {
		        fputc(c, codeFile);
	        }
            while((c = fgetc(tempCode)) != EOF) {
		        fputc(c, codeFile);
	        }
            fclose(printer);
        }

        void startCode() {
            fprintf(codeFile, ".MODEL SMALL\n\n");
	        fprintf(codeFile, ".STACK 400H\n\n");
	        fprintf(codeFile, ".DATA\n\n");
        }

        void printIndentation(FILE* fp) {
            for(int i=0;i<indentCount;i++) {
                fprintf(fp, "\t");
            }
        }

        void handleGlobalVariableDec(string name, string type) {
            printIndentation(codeFile);
            fprintf(codeFile, (name+" DW ?\n").c_str());
        }

        void handleLocalVariableDec(string name, string type) {
            symbolInfo* sym = table->lookupLocal(name);
            sym->setStackOffset(localVariableOffset);
            localVariableOffset -= 2;
            printIndentation(tempCode);
            fprintf(tempCode, "ADD SP, -2\n");
        }

        void handleGlobalArrayDec(arrayInfo* arr) {
            printIndentation(codeFile);
            fprintf(codeFile, (arr->arrName+" DW "+to_string(arr->arrSize)+" DUP(?)\n").c_str());
        }

        void handleLocalArrayDec(arrayInfo* arr) {
            symbolInfo* sym = table->lookupLocal(arr->arrName);
            sym->setStackOffset(localVariableOffset);
            localVariableOffset -= arr->arrSize*2;
            printIndentation(tempCode);
            fprintf(tempCode, ("ADD SP, "+to_string(arr->arrSize*(-2))+"\n").c_str());
        }

        void handleParameterDec(string name) {
            symbolInfo* sym = table->lookupLocal(name);
            sym->setStackOffset(parameterOffset);
            parameterOffset += 2;
        }

        void functionHeader(string name) {
            printIndentation(tempCode);
            fprintf(tempCode, (name+" PROC\n").c_str());
            indentCount++;
            if(name == "main") {
                printIndentation(tempCode);
                fprintf(tempCode, "MOV AX, @DATA\n");
                printIndentation(tempCode);
                fprintf(tempCode, "MOV DS, AX\n");
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH BP\n");
            printIndentation(tempCode);
            fprintf(tempCode, "MOV BP, SP\n");
        }

        void functionFooter(string name, bool expressionFound) {
            if(expressionFound) {
                printIndentation(tempCode);
                fprintf(tempCode, "POP CX\n");
            }
            printIndentation(tempCode);
            fprintf(tempCode, "MOV SP, BP\n");
            printIndentation(tempCode);
            fprintf(tempCode, "POP BP\n");
            if(name != "main") {
                printIndentation(tempCode);
                fprintf(tempCode, ("RET "+to_string(parameterOffset-4)+"\n").c_str());
            }
            else {
                printIndentation(tempCode);
                fprintf(tempCode, "MOV AH, 04CH\n");
                printIndentation(tempCode);
                fprintf(tempCode, "INT 21H\n");
            }
        }

        void functionEnd(string name) {
            indentCount--;
            printIndentation(tempCode);
            fprintf(tempCode, (name+" ENDP\n\n").c_str());
            if(name == "main") {
                fprintf(tempCode, "END main\n");
            }
        }

        void functionCall(string name) {
            printIndentation(tempCode);
            fprintf(tempCode, ("CALL "+name+"\n").c_str());
        }

        void pushArgument() {
            
        }

        void variableCall(symbolInfo* sym) {
            printIndentation(tempCode);
            if(sym->getStackOffset()) {
                fprintf(tempCode, ("MOV CX, [BP+"+to_string(sym->getStackOffset())+"]\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, "PUSH CX\n");
            }
            else {
                fprintf(tempCode, ("PUSH "+sym->getName()+"\n").c_str());
            }
        }

        void arrayCall(symbolInfo* sym) {
            //index on top of stack
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "SAL CX, 1\n");
            if(sym->getStackOffset()) {
                printIndentation(tempCode);
                fprintf(tempCode, "NEG CX\n");
                printIndentation(tempCode);
                fprintf(tempCode, ("ADD CX, "+to_string(sym->getStackOffset())+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, "PUSH BP\n");
                printIndentation(tempCode);
                fprintf(tempCode, "ADD BP, CX\n");
                printIndentation(tempCode);
                fprintf(tempCode, "MOV SI, BP\n");
                printIndentation(tempCode);
                fprintf(tempCode, "MOV CX, [BP]\n");
                printIndentation(tempCode);
                fprintf(tempCode, "POP BP\n");
            }
            else {
                printIndentation(tempCode);
                fprintf(tempCode, ("LEA SI, "+sym->getName()+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, "ADD SI, CX\n");
                printIndentation(tempCode);
                fprintf(tempCode, "MOV CX, [SI]\n");
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH CX\n");
        }

        void popExpression() {
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
        }

        void assignmentExpression(symbolInfo* sym) {
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            if(sym->getVariableType() == "ARRAY") {
                if(sym->getStackOffset()) {
                    fprintf(tempCode, "PUSH BP\n");
                    printIndentation(tempCode);
                    fprintf(tempCode, "MOV BP, DI\n");
                    printIndentation(tempCode);
                    fprintf(tempCode, "MOV [BP], CX\n");
                    printIndentation(tempCode);
                    fprintf(tempCode, "POP BP\n");
                }
                else {
                    fprintf(tempCode, "MOV [DI], CX\n");
                }
            }
            else {
                if(sym->getStackOffset()) {
                    fprintf(tempCode, ("MOV [BP+"+to_string(sym->getStackOffset())+"], CX\n").c_str());
                }
                else {
                    fprintf(tempCode, ("MOV "+sym->getName()+", CX\n").c_str());
                }
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH CX\n");
        } 

        void saveAssignmentAddress() {
            popExpression();
            printIndentation(tempCode);
            fprintf(tempCode, "MOV DI, SI\n");
        }  

        void logicExpression(string op) {
            //no short circuit yet
            string label1 = newLabel();
            string label2 = newLabel();
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "POP AX\n");
            printIndentation(tempCode);
            if(op == "&&") {
                fprintf(tempCode, "CMP AX, 0\n");
                printIndentation(tempCode);
                fprintf(tempCode, ("JE "+label1+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, "CMP CX, 0\n");
                printIndentation(tempCode);
                fprintf(tempCode, ("JE "+label1+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, "PUSH 1\n");
                printIndentation(tempCode);
                fprintf(tempCode, ("JMP "+label2+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, (label1+": PUSH 0\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, (label2+":\n").c_str());

            }
            else {
                fprintf(tempCode, "CMP AX, 0\n");
                printIndentation(tempCode);
                fprintf(tempCode, ("JNE "+label1+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, "CMP CX, 0\n");
                printIndentation(tempCode);
                fprintf(tempCode, ("JNE "+label1+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, "PUSH 0\n");
                printIndentation(tempCode);
                fprintf(tempCode, ("JMP "+label2+"\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, (label1+": PUSH 1\n").c_str());
                printIndentation(tempCode);
                fprintf(tempCode, (label2+":\n").c_str());
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH CX\n");
        }

        void relationalExpression(string op) {
            string label = newLabel();
            string label2 = newLabel();
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "POP AX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "CMP AX, CX\n");
            printIndentation(tempCode);
            if(op == "<") {
                fprintf(tempCode, ("JL "+label+"\n").c_str());
            }
            else if(op == "<=") {
                fprintf(tempCode, ("JLE "+label+"\n").c_str());
            }
            else if(op == ">") {
                fprintf(tempCode, ("JG "+label+"\n").c_str());
            }
            else if(op == ">=") {
                fprintf(tempCode, ("JGE "+label+"\n").c_str());
            }
            else if(op == "==") {
                fprintf(tempCode, ("JE "+label+"\n").c_str());
            }
            else if(op == "!=") {
                fprintf(tempCode, ("JNE "+label+"\n").c_str());
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH 0\n");
            printIndentation(tempCode);
            fprintf(tempCode, ("JMP "+label2+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (label+": PUSH 1\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (label2+":\n").c_str());
            
        }

        void addopExpression(string op) {
            printIndentation(tempCode);
            fprintf(tempCode, "POP AX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            if(op == "+") {
                fprintf(tempCode, "ADD CX, AX\n");
            }
            else {
                fprintf(tempCode, "SUB CX, AX\n");
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH CX\n");
        }

        void mulopExpression(string op) {
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "POP AX\n");
            printIndentation(tempCode);
            if(op == "*") {
                fprintf(tempCode, "IMUL CX\n");
            }
            else if(op == "/"){
                fprintf(tempCode, "CWD\n");
                printIndentation(tempCode);
                fprintf(tempCode, "IDIV CX\n");
            }
            else {
                fprintf(tempCode, "CWD\n");
                printIndentation(tempCode);
                fprintf(tempCode, "IDIV CX\n");
                printIndentation(tempCode);
                fprintf(tempCode, "MOV AX, DX\n");
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH AX\n");
        }

        void unaryAddExpression(string op) {
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            if(op == "-") {
                fprintf(tempCode, "NEG CX\n");
                printIndentation(tempCode);
                fprintf(tempCode, "PUSH CX\n");
            }
            else {
                fprintf(tempCode, "PUSH CX\n");
            }
        }

        void logicalNot() {
            labels.push(make_pair(newLabel(), newLabel()));
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "CMP CX, 0\n");
            printIndentation(tempCode);
            fprintf(tempCode, ("JE "+labels.top().first+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH 0\n");
            printIndentation(tempCode);
            fprintf(tempCode, ("JMP "+labels.top().second+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (labels.top().first+": PUSH 1\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (labels.top().second+":\n").c_str());
            labels.pop();
        }

        void functionReturned() {
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH CX\n");
        }

        void constantExpression(string s) {
            printIndentation(tempCode);
            fprintf(tempCode, ("PUSH "+s+"\n").c_str());
        }

        void incDecExpression(symbolInfo* sym, string op) {
            // printIndentation(tempCode);
            // fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            if(sym->getVariableType() == "ARRAY") {
                if(sym->getStackOffset()) {
                    fprintf(tempCode, "PUSH BP\n");
                    printIndentation(tempCode);
                    fprintf(tempCode, "MOV BP, SI\n");
                    printIndentation(tempCode);
                    fprintf(tempCode, (op+" [BP]\n").c_str());
                    printIndentation(tempCode);
                    fprintf(tempCode, "POP BP\n");
                }
                else {
                    fprintf(tempCode, (op+" [SI]\n").c_str());
                }
            }
            else {
                if(sym->getStackOffset()) {
                    fprintf(tempCode, (op+" WORD PTR [BP+"+to_string(sym->getStackOffset())+"]\n").c_str());
                }
                else {
                    fprintf(tempCode, (op+" "+sym->getName()+"\n").c_str());
                }
            }
            // printIndentation(tempCode);
            // fprintf(tempCode, (op+" CX\n").c_str());
            // printIndentation(tempCode);
            // fprintf(tempCode, "PUSH CX\n");
            
        }

        void callPrintln(symbolInfo* sym) {
            printIndentation(tempCode);
            if(sym->getStackOffset()) {
                fprintf(tempCode, ("MOV CX, [BP+"+to_string(sym->getStackOffset())+"]\n").c_str());
            }
            else {
                fprintf(tempCode, ("MOV CX, "+sym->getName()+"\n").c_str());
            }
            printIndentation(tempCode);
            fprintf(tempCode, "PUSH CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, "CALL PRINTLN\n");
        }

        void labelIfStart() {
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, ("JCXZ "+labels.top().first+"\n").c_str());
        }

        void labelElse() {
            printIndentation(tempCode);
            fprintf(tempCode, ("JMP "+labels.top().second+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (labels.top().first+":\n").c_str());
        }

        void labelElseEnd() {
            printIndentation(tempCode);
            fprintf(tempCode, (labels.top().second+":\n").c_str());
            labels.pop();
        }

        void labelWhileStart() {
            printIndentation(tempCode);
            fprintf(tempCode, (labels.top().second+":\n").c_str());
        }

        void evaluateCondition() {
            labelIfStart();
        }

        void labelWhileEnd() {
            labelElse();
            labels.pop();
        }

        void labelForCondition() {
            l2 = labels.top();
            labels.pop();
            l1 = labels.top();
            labels.push(l2);
            printIndentation(tempCode);
            fprintf(tempCode, (l1.first+":\n").c_str());
        }
        
        void conditionCheckFor() {
            printIndentation(tempCode);
            fprintf(tempCode, ("JCXZ "+l1.second+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, ("JMP "+l2.first+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (l2.second+":\n").c_str());
        }

        void bodyOfFor() {
            printIndentation(tempCode);
            fprintf(tempCode, "POP CX\n");
            printIndentation(tempCode);
            fprintf(tempCode, ("JMP "+l1.first+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (l2.first+":\n").c_str());
        }

        void endFor() {
            printIndentation(tempCode);
            fprintf(tempCode, ("JMP "+l2.second+"\n").c_str());
            printIndentation(tempCode);
            fprintf(tempCode, (l1.second+":\n").c_str());
            labels.pop();
            labels.pop();
        }
};

#endif