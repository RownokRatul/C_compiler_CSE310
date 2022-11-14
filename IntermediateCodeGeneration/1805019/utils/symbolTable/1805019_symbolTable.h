#ifndef _symbolTable_h_
#define _symbolTable_h_

#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<vector>
#include<string>
using namespace std;

#define NIL nullptr
#define ROOT_ID "1"
#define DEFAULT_SIZE 13
#define UNK "**UNKNOWN**"

class arrayInfo {
    
    public:
        string arrName;
        string arrType;
        int arrSize;

};

class functionInfo {

    public:

        string retType;
        string name;
        vector<string> paramList;
        bool defined;

};

class symbolInfo {

        string name;
        string type; // ID, LOGICOP etc etc
        string variableType; //int, float, function, array etc
        arrayInfo* arr;
        symbolInfo* next;
        functionInfo* func;
        int offset; //if zero offset -> global variable

    public:

        symbolInfo(string nm, string tp) {
            //cout<<nm<<tp;
            name = nm;
            type = tp;
            next = NIL;
            func = NIL;
            arr = NIL;
            variableType = "";
            offset = 0;
            //cout<<name<<type<<endl;
        }

        symbolInfo(string nm, string tp, string varType) {
            //cout<<nm<<tp;
            name = nm;
            type = tp;
            next = NIL;
            func = NIL;
            arr = NIL;
            offset = 0;
            variableType = varType;
            //cout<<name<<type<<endl;
        }

        symbolInfo(string nm, string tp, string varType, int off) {
            //cout<<nm<<tp;
            name = nm;
            type = tp;
            next = NIL;
            func = NIL;
            arr = NIL;
            offset = off;
            variableType = varType;
            //cout<<name<<type<<endl;
        }

        symbolInfo(string nm, string tp, functionInfo* f) {
            //cout<<nm<<tp;
            name = nm;
            type = tp;
            next = NIL;
            func = f;
            arr = NIL;
            variableType = "FUNCTION";
            offset = 0;
            //cout<<name<<type<<endl;
        }

        symbolInfo(string nm, string tp, arrayInfo* i) {
            //cout<<nm<<tp;
            name = nm;
            type = tp;
            next = NIL;
            func = NIL;
            arr = i;
            variableType = "ARRAY";
            offset = 0;
            //cout<<name<<type<<endl;
        }

        void setStackOffset(int o) {
            offset = o;
        }

        int getStackOffset() {
            return offset;
        }

        arrayInfo* getArrayInfo() {
            return arr;
        }

        void setArrayInfo(arrayInfo* a) {
            arr = a;
        }

        string getName() {
            return name;
        }

        string getType() {
            return type;
        }

        void setVariableType(string var) {
            variableType = var;
        }

        string getVariableType() {
            return variableType;
        }

        void setFunctionInfo(functionInfo* f) {
            func = f;
        }

        functionInfo* getFunctionInfo() {
            return func;
        }

        void setNameType(string nm, string tp) {
            name = nm;
            type = tp;
        }

        void setNext(symbolInfo* nxt) {
            next = nxt;
        }

        symbolInfo* getNext() {
            return next;
        }

        ~symbolInfo() {
            if(func != NIL) {
                delete func;
            }
            if(arr != NIL) {
                delete arr;
            }
        }

};

class scopeTable {

        int bucketSize;
        scopeTable* parentScope;
        vector<symbolInfo*> hashTable;
        string scopeID;
        int childScopeCount;

        /*unsigned int hash_sdbm(string s) {
            char *str = s.c_str();
            unsigned int hash = 0;
            int c;
            while (c = *str++)
                hash = c + (hash << 6) + (hash << 16) - hash;
            return (hash % bucketSize);
        }*/

        unsigned int hash_sdbm(string s) {
            int len = s.length();
            unsigned int hash = 0;
            for(int i=0;i<len;i++) {
                //hash = (int)s[i] + hash*65599 + hash;
                hash = (int)s[i] + (hash << 6) + (hash << 16) - hash;
            }
            return hash % bucketSize;
        }

        void generateScopeID() {
            if(parentScope == NIL) {
                scopeID = ROOT_ID;
            }
            else {
                if(parentScope->getChildScopeCount()) {
                    scopeID = parentScope->getScopeID() + ".";
                    scopeID += (char)(parentScope->getChildScopeCount() + 48);
                }
                else {
                    scopeID = parentScope->getScopeID() + ".1";
                }
                //cout<<"New ScopeTable with ID# "<<scopeID<<" created.\n";
            }
        }

    public: 

        scopeTable(int size) {
            bucketSize = size;
            parentScope = NIL;
            hashTable.resize(bucketSize);
            childScopeCount = 0;
            generateScopeID();
        }

        scopeTable(int size, scopeTable* parent) {
            bucketSize = size;
            parentScope = parent;
            hashTable.resize(bucketSize);
            childScopeCount = 0;
            generateScopeID();
        }

        scopeTable* getParentScope() {
            return parentScope;
        }

        void setParentScope(scopeTable* t) {
            parentScope = t;
        }


        int getChildScopeCount() {
            return childScopeCount;
        }

        void incrementChildScopeCount() {
            childScopeCount++;
        }

        string getScopeID() {
            return scopeID; 
        }

        bool insert(string name, string type, string varType) {
            int hash = hash_sdbm(name);
            symbolInfo* cur = hashTable[hash];
            symbolInfo* prev = NIL;
            int i = 0;
            while(cur != NIL) {
                if(cur->getName() == name) {
                    // print message
                    //cout<<"< "<<name<<" , "<<type<<" > already exists in Current ScopeTable.\n";
                    return false;
                }
                i++;
                prev = cur;
                cur = cur->getNext();
            }
            symbolInfo* toInsert = new symbolInfo(name, type, varType);
            if(prev == NIL) {
                hashTable[hash] = toInsert;
            }
            else {
                prev->setNext(toInsert);
            }
            // print message
            //cout<<"Inserted in ScopeTable# "<<scopeID<<" at position "<<hash<<", "<<i<<"\n";
            return true;
        }

        bool insert(string name, string type, arrayInfo* ai) {
            int hash = hash_sdbm(name);
            symbolInfo* cur = hashTable[hash];
            symbolInfo* prev = NIL;
            int i = 0;
            while(cur != NIL) {
                if(cur->getName() == name) {
                    // print message
                    //cout<<"< "<<name<<" , "<<type<<" > already exists in Current ScopeTable.\n";
                    return false;
                }
                i++;
                prev = cur;
                cur = cur->getNext();
            }
            symbolInfo* toInsert = new symbolInfo(name, type, ai);
            if(prev == NIL) {
                hashTable[hash] = toInsert;
            }
            else {
                prev->setNext(toInsert);
            }
            // print message
            //cout<<"Inserted in ScopeTable# "<<scopeID<<" at position "<<hash<<", "<<i<<"\n";
            return true;
        }

        bool insert(string name, string type, functionInfo* f) {
            int hash = hash_sdbm(name);
            symbolInfo* cur = hashTable[hash];
            symbolInfo* prev = NIL;
            int i = 0;
            while(cur != NIL) {
                if(cur->getName() == name) {
                    // print message
                    //cout<<"< "<<name<<" , "<<type<<" > already exists in Current ScopeTable.\n";
                    return false;
                }
                i++;
                prev = cur;
                cur = cur->getNext();
            }
            symbolInfo* toInsert = new symbolInfo(name, type, f);
            if(prev == NIL) {
                hashTable[hash] = toInsert;
            }
            else {
                prev->setNext(toInsert);
            }
            // print message
            //cout<<"Inserted in ScopeTable# "<<scopeID<<" at position "<<hash<<", "<<i<<"\n";
            return true;
        }

        bool insert(string name, string type) {
            int hash = hash_sdbm(name);
            symbolInfo* cur = hashTable[hash];
            symbolInfo* prev = NIL;
            int i = 0;
            while(cur != NIL) {
                if(cur->getName() == name) {
                    // print message
                    //cout<<"< "<<name<<" , "<<type<<" > already exists in Current ScopeTable.\n";
                    return false;
                }
                i++;
                prev = cur;
                cur = cur->getNext();
            }
            symbolInfo* toInsert = new symbolInfo(name, type);
            if(prev == NIL) {
                hashTable[hash] = toInsert;
            }
            else {
                prev->setNext(toInsert);
            }
            // print message
            //cout<<"Inserted in ScopeTable# "<<scopeID<<" at position "<<hash<<", "<<i<<"\n";
            return true;
        }

        symbolInfo* lookup(string name) {
            int hash = hash_sdbm(name);
            symbolInfo* cur = hashTable[hash];
            int i = 0;
            while(cur != NIL) {
                if(cur->getName() == name) {
                    // print message
                    //cout<<"Found in ScopeTable# "<<scopeID<<" in position "<<hash<<", "<<i<<"\n";
                    return cur;
                }
                i++;
                cur = cur->getNext();
            }
            return NIL;
        }

        bool deleteSymbol(string name) {
            int hash = hash_sdbm(name);
            symbolInfo* cur = hashTable[hash];
            symbolInfo* prev = NIL;
            int i = 0;
            while(cur != NIL) {
                if(cur->getName() == name) {
                    break;
                }
                i++;
                prev = cur;
                cur = cur->getNext();
            }
            if(cur == NIL) {
                // print message
                //cout<<"Not Found\n";
                return false;
            }
            else {
                if(prev != NIL) {
                    //cout<<"NOT  NIL\n";
                    prev->setNext(cur->getNext());
                }
                else {
                    //cout<<"NIL\n";
                    hashTable[hash] = cur->getNext();
                }
                // print message
                //cout<<"Found in ScopeTable# "<<scopeID<<" in position "<<hash<<", "<<i<<"\n";
                delete cur;
                //cout<<"Deleted Entry "<<hash<<", "<<i<<" from Current Scope Table.\n";
            }
            return true;
        }

        void printHashTable() {
            cout<<"ScopeTable # "<<scopeID<<" :\n";
            for(int i=0;i<bucketSize;i++) {
                cout<<i<<"--> ";
                symbolInfo* cur = hashTable[i];
                while(cur != NIL) {
                    cout<<"< "<<cur->getName()<<" , "<<cur->getType()<<" > ";
                    cur = cur->getNext();
                }
                cout<<'\n';
            }
        }

        void printNonEmptyRows(FILE* log) {
            //cout<<"ScopeTable # "<<scopeID<<" :\n";
            fprintf(log, "ScopeTable # %s :\n", scopeID.c_str());
            for(int i=0;i<bucketSize;i++) {
                symbolInfo* cur = hashTable[i];
                if(cur != NIL) {
                    //cout<<i<<"--> ";
                    fprintf(log, "%d--> ",i);
                    while(cur != NIL) {
                        //cout<<"< "<<cur->getName()<<" , "<<cur->getType()<<" > ";
                        fprintf(log, "< %s : %s > ",cur->getName().c_str(), cur->getType().c_str());
                        cur = cur->getNext();
                    }
                    //cout<<"\n";
                    fprintf(log, "\n");
                }
            }
            //cout<<"___ENDING___\n";
            fprintf(log, "\n");
        }

        ~scopeTable() {
            for(int i=0;i<bucketSize;i++) {
                symbolInfo* curDel;
                symbolInfo* nextDel = NIL;
                curDel = hashTable[i];
                while (curDel != NIL) {
                    nextDel = curDel->getNext();
                    delete curDel;
                    curDel = nextDel;
                }
            }
        }

};

class symbolTable {

        int bucketSize;
        scopeTable* currentScope;

    public:

        symbolTable(int size = DEFAULT_SIZE) {
            bucketSize = size;
            currentScope = new scopeTable(bucketSize);
            //cout<<"Symbol table created.\n";
        }

        void enterScope() {
            if(currentScope == NIL) {
                currentScope = new scopeTable(bucketSize);
            }
            else {
                currentScope->incrementChildScopeCount();
                scopeTable* nextScope = new scopeTable(bucketSize, currentScope);
                nextScope->setParentScope(currentScope);
                currentScope = nextScope;
            }
            
        }

        void exitScope() {
            if(currentScope == NIL) {
                cout<<"No Scope Exists.\n";
            }
            else {
                scopeTable* prevScope = currentScope->getParentScope();
                //cout<<"Scope Table with ID# "<<currentScope->getScopeID()<<" removed.\n";
                delete currentScope;
                currentScope = prevScope;
            }
        }

        bool insert(string name, string type, string varType) {
            if(currentScope != NIL) {
                return currentScope->insert(name, type, varType);
            }
            else {
                cout<<"No Scope Table.\n";
                return false;
            }
        }

        bool insert(string name, string type, functionInfo* f) {
            if(currentScope != NIL) {
                return currentScope->insert(name, type, f);
            }
            else {
                cout<<"No Scope Table.\n";
                return false;
            }
        }

        bool insert(string name, string type, arrayInfo* i) {
            if(currentScope != NIL) {
                return currentScope->insert(name, type, i);
            }
            else {
                cout<<"No Scope Table.\n";
                return false;
            }
        }

        bool insert(string name, string type) {
            if(currentScope != NIL) {
                return currentScope->insert(name, type);
            }
            else {
                cout<<"No Scope Table.\n";
                return false;
            }
        }

        bool remove(string name) {
            if(currentScope != NIL) {
                //cout<<"yessir"<<endl;
                return currentScope->deleteSymbol(name);
                //cout<<"yessir"<<endl;
            }
            else {
                cout<<"No Scope Table.\n";
                return false;
            }
        }

        string getCurrentScopeID() {
            return currentScope->getScopeID();
        }

        symbolInfo* lookupGlobal(string name) {
            scopeTable* cur = currentScope;
            while(cur != NIL) {
                symbolInfo* result = cur->lookup(name);
                if(result != NIL) {
                    return result;
                }
                cur = cur->getParentScope();
            }
            // print message
            //cout<<"Not Found.\n";
            return NIL;
        }

        symbolInfo* lookupLocal(string name) {
            return currentScope->lookup(name);
        }

        void printNonEmptyRows(FILE* log) {
            scopeTable* cur = currentScope;
            while(cur != NIL) {
                //cout<<"SCOPEEE:: "<<cur->getScopeID()<<endl;
                cur->printNonEmptyRows(log);
                cur = cur->getParentScope();
            }
        }

        void printCurrentScopeTable() {
            if(currentScope != NIL) {
                currentScope->printHashTable();
            }
        }

        void printAllScopeTable() {
            scopeTable* cur = currentScope;
            while(cur != NIL) {
                cur->printHashTable();
                cur = cur->getParentScope();
            }
        }

        ~symbolTable() {
            delete currentScope;
        }

};

#endif