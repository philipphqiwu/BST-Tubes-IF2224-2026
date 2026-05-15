#ifndef SYMTAB_HPP
#define SYMTAB_HPP

#include <string>
#include <vector>
#include <iostream>

enum class ObjClass {
    UNKNOWN = 0,
    CONSTANT,
    VARIABLE,
    TYPE,
    PROCEDURE,
    FUNCTION,
    PROGRAM,
    FIELD 
};

struct TabEntry {
    std::string name;
    ObjClass obj;
    int type;
    int ref;
    int nrm;
    int lev;
    int adr;
    int link;
};

struct BTabEntry {
    int last;
    int lpar;
    int psze;
    int vsze;
};

struct ATabEntry {
    int xtyp;
    int etyp;
    int eref;
    int low;
    int high;
    int elsz;
    int size;
};

class SymbolTable {
public:
    std::vector<TabEntry> tab;
    std::vector<BTabEntry> btab;
    std::vector<ATabEntry> atab;

    std::vector<int> display;
    int current_level;
    int predefined_count;

    SymbolTable();

    void initPredefined();

    void enterBlock();
    void leaveBlock();

    int insertTab(const std::string& name, ObjClass obj, int type, int ref, int nrm, int adr);
    int insertTabAtLevel(int level, const std::string& name, ObjClass obj, int type, int ref, int nrm, int adr);
    int insertBTab();
    int insertATab(int xtyp, int etyp, int eref, int low, int high, int elsz, int size);

    int lookup(const std::string& name);
    int lookupLocal(const std::string& name);
};

#endif