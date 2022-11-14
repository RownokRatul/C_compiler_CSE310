%{
	#include<iostream>
	#include<cstdlib>
	#include<cstring>
	#include<cstdio>
	#include<vector>
	#include<cmath>
	#include "utils/functionUtil.h"
	using namespace std;

	#define SPLITTER ","
	#define INT_TYPE 1
	#define FLOAT_TYPE 2 
	#define VOID_TYPE 3

	extern FILE *yyin;
	extern int yylineno;
	FILE *logFile;
	FILE *errorFile;

	symbolTable *table = new symbolTable();
	stringUtil *strAnalyzer = new stringUtil();
	arrayUtil *arrAnalyzer = new arrayUtil();
	functionUtil *funcAnalyzer = new functionUtil();
	semanticErrorHandler *semanticErr = new semanticErrorHandler();
	typeConverter *typeConversion = new typeConverter();

	int line_count = 1;
	int error_count = 0;
	bool definingFunctionScope = false;
	string typeSpec;
	string funcRetType;
	string currentFunction;
	vector<int> variableDeclarationType;

	int yyparse(void);
	int yylex(void);

	void printLine(FILE *fp) {
		fprintf(fp, "Line %d: ", line_count);
	}

	void yyerror(char *s) {
		error_count++;
		fprintf(errorFile, "Error at line:%d %s\n\n", line_count, s);
		fprintf(logFile, "Error at line:%d %s\n\n", line_count, s);
		//cout<<"error found : "<<s<<endl;
	}

	string getVarTypeSpec() {
		return (variableDeclarationType.back() == INT_TYPE) ? "INT" : ((variableDeclarationType.back() == FLOAT_TYPE) ? "FLOAT" : "VOID" );
	}

	string getFuncTypeSpec() {
		return (variableDeclarationType[0] == INT_TYPE) ? "INT" : ((variableDeclarationType[0] == FLOAT_TYPE) ? "FLOAT" : "VOID" );
	}

	void printSummary() {
		table->printNonEmptyRows(logFile);
		fprintf(logFile, "Total Lines: %d\nTotal Errors: %d", line_count-1, error_count);
	}



%} 

%union {
	symbolInfo* symbol;
}

%token<symbol> ID CONST_INT CONST_FLOAT CONST_CHAR ADDOP MULOP LOGICOP RELOP
%token IF ELSE FOR WHILE DO INT FLOAT VOID SWITCH DEFAULT BREAK RETURN CHAR DOUBLE CASE CONTINUE PRINTLN
%token ASSIGNOP NOT INCOP DECOP LPAREN RPAREN LTHIRD RTHIRD LCURL RCURL COMMA SEMICOLON 

%type<symbol> var_declaration declaration_list start unit program
%type<symbol> parameter_list func_declaration func_definition
%type<symbol> variable factor term
%type<symbol> logic_expression unary_expression simple_expression rel_expression expression
%type<symbol> expression_statement compound_statement statement statements
%type<symbol> argument_list arguments

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

start : program
	{
		//write your code in this block in all the similar blocks below
		$$ = $1;
		fprintf(logFile, "Line %d: ", line_count-1);
		fprintf(logFile, "start : program\n\n");
		//fprintf(logFile, "%s\n\n", $$->getName().c_str());

	}
	;

program : program unit 
	{
		$$ = new symbolInfo($1->getName()+"\n"+$2->getName(), $2->getType());
		printLine(logFile);
		fprintf(logFile, "program : program unit\n\n");
		fprintf(logFile, "%s\n\n", $$->getName().c_str());
	}
	| unit
	{
		$$ = $1;
		printLine(logFile);
		fprintf(logFile, "program : unit\n\n");
		fprintf(logFile, "%s\n\n", $$->getName().c_str());
	}
	;
	
unit : var_declaration
	{
		$$ = $1;
		printLine(logFile);
		fprintf(logFile, "unit : var_declaration\n\n");
		fprintf(logFile, "%s\n\n", $$->getName().c_str());
	}
     | func_declaration
     {
     	$$ = $1;
     	printLine(logFile);
		fprintf(logFile, "unit : func_declaration\n\n");
		fprintf(logFile, "%s\n\n", $$->getName().c_str());
     }
     | func_definition
     {
     	$$ = $1;
     	printLine(logFile);
		fprintf(logFile, "unit : func_definition\n\n");
		fprintf(logFile, "%s\n\n", $$->getName().c_str());
     }
     ;
     
func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON
		{
			typeSpec = getFuncTypeSpec();
			funcAnalyzer->handleFunctionDeclaration($2->getName(), typeSpec, $4->getName());
			variableDeclarationType.clear();
			$$ = new symbolInfo(strAnalyzer->toLower(typeSpec)+" "+$2->getName()+" ("+$4->getName()+");", "FUNC_DECLARATION");
			printLine(logFile);
			fprintf(logFile, "func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| type_specifier ID LPAREN RPAREN SEMICOLON
		{
			typeSpec = getFuncTypeSpec();
			funcAnalyzer->handleFunctionDeclaration($2->getName(), typeSpec, "");
			variableDeclarationType.clear();
			$$ = new symbolInfo(strAnalyzer->toLower(typeSpec)+" "+$2->getName()+" ();", "FUNC_DECLARATION");
			printLine(logFile);
			fprintf(logFile, "func_declaration : type_specifier ID LPAREN RPAREN SEMICOLON\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		;
		 
func_definition : type_specifier ID LPAREN parameter_list RPAREN
		{
			definingFunctionScope = true;
			typeSpec = getFuncTypeSpec();
			funcRetType = typeSpec;
			currentFunction = $2->getName();
			//cout<<$2->getName()<<" "<<typeSpec<<" "<<$4->getName()<<endl;
			funcAnalyzer->handleFunctionDefinition($2->getName(), typeSpec, $4->getName());
			variableDeclarationType.clear();

		}
		compound_statement
		{
			$$ = new symbolInfo(strAnalyzer->toLower(funcRetType)+" "+$2->getName()+"("+$4->getName()+")"+$7->getName(), "FUNC_DEFINITON");
			printLine(logFile);
			fprintf(logFile, "func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| type_specifier ID LPAREN RPAREN
		{
			definingFunctionScope = true;
			typeSpec = getFuncTypeSpec();
			funcRetType = typeSpec;
			currentFunction = $2->getName();
			funcAnalyzer->handleFunctionDefinition($2->getName(), typeSpec, "");
			variableDeclarationType.clear();
		}
		compound_statement
		{
			$$ = new symbolInfo(strAnalyzer->toLower(typeSpec)+" "+$2->getName()+"()"+$6->getName(), "FUNC_DEFINITON");
			printLine(logFile);
			fprintf(logFile, "func_definition : type_specifier ID LPAREN RPAREN compound_statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
 		;				


parameter_list  : parameter_list COMMA type_specifier ID
		{
			typeSpec = getVarTypeSpec();
			$1->setNameType($1->getName()+SPLITTER+strAnalyzer->toLower(typeSpec)+" "+$4->getName(), $1->getType());
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "parameter_list  : parameter_list COMMA type_specifier ID\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| parameter_list COMMA type_specifier
		{
			typeSpec = getVarTypeSpec();
			$1->setNameType($1->getName()+SPLITTER+strAnalyzer->toLower(typeSpec), $1->getType());
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "parameter_list  : parameter_list COMMA type_specifier\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
 		| type_specifier ID
 		{
 			typeSpec = getVarTypeSpec();
 			$$ = new symbolInfo(strAnalyzer->toLower(typeSpec)+" "+$2->getName(), "PARAMETER_LIST");
 			printLine(logFile);
 			fprintf(logFile, "parameter_list : type_specifier ID\n\n");
 			fprintf(logFile, "%s\n\n", $$->getName().c_str());
 		}
		| type_specifier
		{
			typeSpec = getVarTypeSpec();
			$$ = new symbolInfo(strAnalyzer->toLower(typeSpec), "PARAMETER_LIST");
			printLine(logFile);
			fprintf(logFile, "parameter_list : type_specifier\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
 		;

 		
compound_statement : LCURL
				{
					if(!definingFunctionScope) {
						table->enterScope();
					}
					definingFunctionScope = false;
				}
				statements RCURL
				{
					$$ = new symbolInfo("{\n"+$3->getName()+"}\n", "COMPOUND_STATEMENT");
					printLine(logFile);
					fprintf(logFile, "compound_statement : LCURL statements RCURL\n\n");
					fprintf(logFile, "%s\n\n",$$->getName().c_str());
					table->printNonEmptyRows(logFile);
					table->exitScope();
					//table->printNonEmptyRows(logFile);
		    	}
 		    	| 
 		    	LCURL
 		    	{
 		    		if(!definingFunctionScope) {
						table->enterScope();
					}
					definingFunctionScope = false;
 		    	}
 		    	RCURL
 		    	{
 		    		$$ = new symbolInfo("{\n}\n", "COMPOUND_STATEMENT");
 		    		printLine(logFile);
					fprintf(logFile, "compound_statement : LCURL RCURL\n\n");
					fprintf(logFile, "%s\n\n",$$->getName().c_str());
					table->printNonEmptyRows(logFile);
					table->exitScope();
					//table->printNonEmptyRows(logFile);
 		    	}
 		    	;
 		    
var_declaration : type_specifier declaration_list SEMICOLON
		{
			typeSpec = getVarTypeSpec();
			if(variableDeclarationType.back() == VOID_TYPE) {
				//error
				semanticErr->voidTypeVariable($2->getName());
			}
			vector<string> ls = strAnalyzer->splitString($2->getName(), ',');
			for(string name : ls) {
				if(arrAnalyzer->validArray(name)) {
					arrayInfo* arr = arrAnalyzer->extractArrayInfo(name);
					arr->arrType = typeSpec;
					if(!table->insert(arr->arrName, "ID", arr)) {
						//Error
						semanticErr->multipleDeclarationVariable(arr->arrName);
					}
				}
				else {
					if(!table->insert(name, "ID", typeSpec)) {
						//Error
						semanticErr->multipleDeclarationVariable(name);
					}
				}
			}
			//table->printNonEmptyRows(logFile);
			variableDeclarationType.clear();
			$$ = new symbolInfo(strAnalyzer->toLower(typeSpec)+ " " + $2->getName()+ ";", "VAR_DECLARATION");
			printLine(logFile);
			fprintf(logFile, "var_declaration : type_specifier declaration_list SEMICOLON\n\n");
			fprintf(logFile, "%s\n\n",$$->getName().c_str());
		}
 		;
 		 
type_specifier	: INT
		{
			variableDeclarationType.push_back(INT_TYPE);
			printLine(logFile);
			fprintf(logFile, "type_specifier : INT\n\n");
			fprintf(logFile, "int\n\n");
		}
 		| FLOAT
 		{
 			variableDeclarationType.push_back(FLOAT_TYPE);
 			printLine(logFile);
			fprintf(logFile, "type_specifier : FLOAT\n\n");
			fprintf(logFile, "float\n\n");
 		}
 		| VOID
 		{
 			variableDeclarationType.push_back(VOID_TYPE);
 			printLine(logFile);
			fprintf(logFile, "type_specifier : VOID\n\n");
			fprintf(logFile, "void\n\n");
 		}
 		;
 		
declaration_list : declaration_list COMMA ID
		{
			$1->setNameType($1->getName() + SPLITTER + $3->getName() , $1->getType());
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "declaration_list : declaration_list COMMA ID\n\n");
			fprintf(logFile, "%s\n\n",$1->getName().c_str());
		}
 		| declaration_list COMMA ID LTHIRD CONST_INT RTHIRD
 		{
 			$1->setNameType($1->getName()+SPLITTER+$3->getName()+"["+$5->getName()+"]", $1->getType());
 			$$ = $1;
 			printLine(logFile);
			fprintf(logFile, "declaration_list : declaration_list COMMA ID LTHIRD CONST_INT RTHIRD\n\n");
			fprintf(logFile, "%s\n\n",$1->getName().c_str());
 		}
 		| ID 
 		{
 			$$ = $1;
 			printLine(logFile);
			fprintf(logFile, "declaration_list : ID\n\n");
			fprintf(logFile, "%s\n\n",$1->getName().c_str());
 		}
 		| ID LTHIRD CONST_INT RTHIRD
 		{
 			$1->setNameType($1->getName()+"["+$3->getName()+"]", $1->getType());
 			$$ = $1;
 			printLine(logFile);
			fprintf(logFile, "declaration_list : ID LTHIRD CONST_INT RTHIRD\n\n");
			fprintf(logFile, "%s\n\n",$1->getName().c_str());
 		}
 		;
 		  
statements : statement
		{
			$1->setNameType($1->getName()+"\n", "STATEMENTS");
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "statements : statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
	   	| statements statement
	   	{
	   		$1->setNameType($1->getName()+$2->getName()+"\n", "STATEMENTS");
	   		$$ = $1;
	   		printLine(logFile);
			fprintf(logFile, "statements : statements statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	   	}
	   	;
	   
statement : var_declaration
		{
			$1->setNameType($1->getName(), "STATEMENT");
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "statement : var_declaration\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
	  	| expression_statement
	  	{
	  		$1->setNameType($1->getName(), "STATEMENT");
			$$ = $1;
	  		printLine(logFile);
			fprintf(logFile, "statement : expression_statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	}
	  	| compound_statement 
	  	{
	  		$1->setNameType($1->getName(), "STATEMENT");
			$$ = $1;
	  		printLine(logFile);
			fprintf(logFile, "statement : compound_statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	}
	  	| FOR LPAREN expression_statement expression_statement expression RPAREN statement
	  	{
	  		$$ = new symbolInfo("for("+$3->getName()+$4->getName()+$5->getName()+") "+$7->getName(), "STATEMENT");
	  		//Error possible ? ?
	  		printLine(logFile);
			fprintf(logFile, "statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	}
	  	| IF LPAREN expression RPAREN statement 	%prec LOWER_THAN_ELSE
	  	{
	  		//Error
	  		typeConversion->typeResolve($3, $3->getType());
	  		if($3->getType() == UNK) {
	  			semanticErr->invalidExpressionIF();
	  		}
	  		$$ = new symbolInfo("if("+$3->getName()+") "+$5->getName(), "STATEMENT");
	  		printLine(logFile);
			fprintf(logFile, "statement : IF LPAREN expression RPAREN statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	} 
	  	| IF LPAREN expression RPAREN statement ELSE statement
	  	{
	  		//Error
	  		typeConversion->typeResolve($3, $3->getType());
	  		if($3->getType() == UNK) {
	  			semanticErr->invalidExpressionIF();
	  		}
	  		$$ = new symbolInfo("if("+$3->getName()+") "+$5->getName()+"\nelse "+$7->getName(), "STATEMENT");
	  		//Error possible
	  		printLine(logFile);
			fprintf(logFile, "statement : IF LPAREN expression RPAREN statement ELSE statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	}
	  	| WHILE LPAREN expression RPAREN statement
	  	{
	  		//Error
	  		typeConversion->typeResolve($3, $3->getType());
	  		if($3->getType() == UNK) {
	  			semanticErr->invalidExpressionWHILE();
	  		}
	  		$$ = new symbolInfo("while("+$3->getName()+") "+$5->getName(), "STATEMENT");
	  		printLine(logFile);
			fprintf(logFile, "statement : WHILE LPAREN expression RPAREN statement\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	}
	  	| PRINTLN LPAREN ID RPAREN SEMICOLON
	  	{
	  		//Error don't know what to do
	  		symbolInfo* sym = table->lookupGlobal($3->getName());
	  		if(sym == NIL) {
	  			semanticErr->undeclaredVariable($3->getName());
	  		}
	  		else {
	  			if(sym->getVariableType() == "FUNCTION") {
	  				semanticErr->expectFunctionCall($3->getName());
	  			}
	  			else if(sym->getVariableType() == "ARRAY") {
	  				semanticErr->expectArray($3->getName());
	  			}
	  		}
	  		$$ = new symbolInfo("printf("+$3->getName()+");", "STATEMENT");
	  		printLine(logFile);
			fprintf(logFile, "statement : PRINTLN LPAREN ID RPAREN SEMICOLON\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	}
	  	| RETURN expression SEMICOLON
	  	{
	  		//cout<<$2->getType()<<" "<<funcRetType<<endl;
	  		if($2->getType() != funcRetType) {
	  			semanticErr->invalidReturnType(funcRetType, $2->getType(), currentFunction);
	  			$$ = new symbolInfo("return "+$2->getName()+";", UNK);
	  		}
	  		else {
	  			$$ = new symbolInfo("return "+$2->getName()+";", "STATEMENT");
	  		}
	  		//Error for invalid return type
	  		printLine(logFile);
			fprintf(logFile, "statement : RETURN expression SEMICOLON\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	  	}
	  ;
	  
expression_statement 	: SEMICOLON 
			{
				$$ = new symbolInfo(";", "EMPTY_EXPRESSION");
				printLine(logFile);
				fprintf(logFile, "expression_statement : SEMICOLON\n\n");
				fprintf(logFile, "%s\n\n", $$->getName().c_str());
			}			
			| expression SEMICOLON 
			{
				$1->setNameType($1->getName()+";", $1->getType());
				$$ = $1;
				printLine(logFile);
				fprintf(logFile, "expression_statement : SEMICOLON\n\n");
				fprintf(logFile, "%s\n\n", $$->getName().c_str());
			}
			;
	  
variable : ID
		{
			symbolInfo* sym = table->lookupGlobal($1->getName());
			if(sym == NIL) {
				//Error
				semanticErr->undeclaredVariable($1->getName());
				$1->setNameType($1->getName(), UNK);
			}
			else {
				string tp = typeConversion->variableTypeSetter(sym);
				$1->setNameType($1->getName(), tp);
			}
			$$ = $1;
			//cout<<$$->getType()<<endl;
			printLine(logFile);
			fprintf(logFile, "variable : ID\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		} 		
	 	| ID LTHIRD expression RTHIRD
	 	{
	 		symbolInfo* sym = table->lookupGlobal($1->getName());
			if(sym == NIL) {
				//Error
				semanticErr->undeclaredVariable($1->getName());
				$1->setNameType($1->getName()+"["+$3->getName()+"]", UNK);
			}
			else {
				if(sym->getVariableType() == "ARRAY") {
					if(arrAnalyzer->arrayIndexCheck($3->getType())) {
						$1->setNameType($1->getName()+"["+$3->getName()+"]", sym->getArrayInfo()->arrType);

					}
					else {
						//Error array index NOT INT
						semanticErr->arrayIndexError($1->getName()+"["+$3->getName()+"]", $3->getType());
						$1->setNameType($1->getName(), UNK);
					}
				}
				else {
					//Error variable not array
					semanticErr->notAnArray($1->getName());
					$1->setNameType($1->getName()+"["+$3->getName()+"]", UNK);
				}
			}
	 		$$ = $1;
	 		printLine(logFile);
			fprintf(logFile, "variable : ID LTHIRD expression RTHIRD\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	 	} 
	 	;
	 
 expression : logic_expression	
 		{
 			$$ = $1;
 			printLine(logFile);
			fprintf(logFile, "expression : logic_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
 		}
	   	| variable ASSIGNOP logic_expression 	
	   	{
	   		//Error check type compatibility
	   		if($3->getType() == "VOID") {
	   			semanticErr->voidTypeIncompatible($3->getName());
	   			$$ = new symbolInfo($1->getName()+" = "+$3->getName(), UNK);
	   		} 
	   		if($1->getType() == UNK || $3->getType() == UNK) {
	   			//semanticErr->invalidAssignmentOperator($1, $3);
	   			$$ = new symbolInfo($1->getName()+" = "+$3->getName(), UNK);
	   		}
	   		if($1->getType() == "INT" && $3->getType() == "FLOAT") {
	   			semanticErr->floatCastToInt($3->getName());
	   			$$ = new symbolInfo($1->getName()+" = "+$3->getName(), "FLOAT");
	   		}
	   		else {
	   			$$ = new symbolInfo($1->getName()+" = "+$3->getName(), $1->getType());
	   		}
	   		printLine(logFile);
			fprintf(logFile, "expression : variable ASSIGNOP logic_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	   	}
	   	;
			
logic_expression : rel_expression 	
		{
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "logic_expression : rel_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| rel_expression LOGICOP rel_expression 
		{
			typeConversion->typeResolve($1, "INT");
			typeConversion->typeResolve($3, "INT");
			string promotedType = typeConversion->typePromotion($1->getType(), $3->getType());
			$1->setNameType($1->getName()+" "+$2->getName()+" "+$3->getName(), promotedType);
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "logic_expression : rel_expression LOGICOP rel_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}	
		;
			
rel_expression	: simple_expression 
		{
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "rel_expression : simple_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| simple_expression RELOP simple_expression	
		{
			typeConversion->typeResolve($1, "INT");
			typeConversion->typeResolve($3, "INT");
			$1->setNameType($1->getName()+" "+$2->getName()+" "+$3->getName(), typeConversion->typePromotion($1->getType(), $3->getType()));
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "rel_expression : simple_expression RELOP simple_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		;
				
simple_expression : term 
		{
			$$ = $1;
			printLine(logFile);
			fprintf(logFile, "simple_expression : term\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| simple_expression ADDOP term 
		{
			typeConversion->typeResolve($1, $1->getType());
			typeConversion->typeResolve($3, $3->getType());
			$1->setNameType($1->getName()+$2->getName()+$3->getName(), typeConversion->typePromotion($1->getType(), $3->getType()));
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"simple_expression : simple_expression ADDOP term\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		;
					
term :	unary_expression
		{
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"term :	unary_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
     	|  term MULOP unary_expression
     	{
     		typeConversion->typeResolve($1, $1->getType());
			typeConversion->typeResolve($3, $3->getType());
			if($2->getName() == "%") {
     			if($1->getType() != "INT" || $3->getType() != "INT") {
     				semanticErr->modulusOperatorUsage($1->getType(), $3->getType());
     				$1->setNameType($1->getName()+$2->getName()+$3->getName(), UNK);
     			}
     			else {
     				if($3->getName() == "0") {
     					semanticErr->modulusByZero();
     				}
     				$1->setNameType($1->getName()+$2->getName()+$3->getName(), "INT");
     			}
     		}
     		else {
     			$1->setNameType($1->getName()+$2->getName()+$3->getName(), typeConversion->typePromotion($1->getType(), $3->getType()));
     		}
     		$$ = $1;
     		printLine(logFile);
			fprintf(logFile,"term :	term MULOP unary_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
     	}
     	;

unary_expression : ADDOP unary_expression
		{
			typeConversion->typeResolve($2, $2->getType());
			$2->setNameType($1->getName()+$2->getName(), $2->getType());
			$$ = $2;
			printLine(logFile);
			fprintf(logFile,"unary_expression : ADDOP unary_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}  
		| NOT unary_expression 
		{
			typeConversion->typeResolve($2, "INT");
			$2->setNameType("!"+$2->getName(), $2->getType());
			$$ = $2;
			printLine(logFile);
			fprintf(logFile,"unary_expression : NOT unary_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| factor
		{
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"unary_expression : factor\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		} 
		;
	
factor	: variable 
		{ 
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"factor : variable\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| ID LPAREN argument_list RPAREN
		{
			string temp = funcAnalyzer->checkValidFunctionCall($1->getName(), $3->getType());
			//cout<<"here -> "<<temp<<endl;
			$$ = new symbolInfo($1->getName()+"("+$3->getName()+")", temp);
			printLine(logFile);
			fprintf(logFile,"factor : ID LPAREN argument_list RPAREN\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| LPAREN expression RPAREN
		{
			typeConversion->typeResolve($2, $2->getType());
			$2->setNameType("("+$2->getName()+")", $2->getType());
			$$ = $2;
			printLine(logFile);
			fprintf(logFile,"factor : LPAREN expression RPAREN\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| CONST_INT 
		{
			$1->setNameType($1->getName(), "INT");
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"factor : CONST_INT\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| CONST_FLOAT
		{
			$1->setNameType($1->getName(), "FLOAT");
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"factor : CONST_FLOAT\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| variable INCOP 
		{
			$1->setNameType($1->getName()+"++", $1->getType());
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"factor : variable INCOP\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		| variable DECOP
		{
			$1->setNameType($1->getName()+"--", $1->getType());
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"factor : variable DECOP\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
		;
	
argument_list : arguments
			{
				$$ = $1;
				printLine(logFile);
				fprintf(logFile,"argument_list : arguments\n\n");
				fprintf(logFile, "%s\n\n", $$->getName().c_str());
			}
			|
			{
				//empty argument
				$$ = new symbolInfo("", "");
				printLine(logFile);
				fprintf(logFile,"argument_list : <empty string>\n\n");
				fprintf(logFile, "%s\n\n", $$->getName().c_str());
			}
			;
	
arguments : arguments COMMA logic_expression
		{
			//cout<<$3->getType()<<endl;
			$1->setNameType($1->getName()+", "+$3->getName(), $1->getType()+","+$3->getType());
			$$ = $1;
			printLine(logFile);
			fprintf(logFile,"arguments : arguments COMMA logic_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
		}
	     | logic_expression
	     {
	     	$$ = $1;
	     	printLine(logFile);
			fprintf(logFile,"arguments : logic_expression\n\n");
			fprintf(logFile, "%s\n\n", $$->getName().c_str());
	     }
	     ;

%%

int main(int argc, char* argv[]) {
	FILE *fp;
	if((fp = fopen("input.txt", "r")) == NULL) {
		printf("Cannot Open Input File.\n");
		exit(1);
	}
	logFile = fopen("log.txt", "w");
	fclose(logFile);
	errorFile = fopen("error.txt", "w");
	fclose(errorFile);
	logFile = fopen("log.txt", "a");
	errorFile = fopen("error.txt", "a");

	yyin = fp;
	cout<<"parse starting\n";
	yyparse();
	cout<<"parsing done\n";
	printSummary();
	fclose(logFile);
	fclose(errorFile);

	return 0;
}


