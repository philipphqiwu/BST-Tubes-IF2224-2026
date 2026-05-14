#include "header/symtab.hpp"

SymbolTable::SymbolTable() {
    current_level = 0;
    
    BTabEntry global_block = {0, 0, 0, 0};
    btab.push_back(global_block);
    display.push_back(0); 

    initPredefined();
}

void SymbolTable::initPredefined() {
    for (int i = 0; i < 33; ++i) {
        TabEntry dummy_entry;
        dummy_entry.name = "reserved_" + std::to_string(i);
        dummy_entry.obj = ObjClass::UNKNOWN;
        dummy_entry.type = 0; dummy_entry.ref = 0; dummy_entry.nrm = 0; 
        dummy_entry.lev = 0; dummy_entry.adr = 0; dummy_entry.link = 0;
        tab.push_back(dummy_entry);
    }

    insertTab("integer", ObjClass::TYPE, 1, 0, 1, 0); 
    insertTab("real", ObjClass::TYPE, 2, 0, 1, 0);    
    insertTab("boolean", ObjClass::TYPE, 3, 0, 1, 0); 
    insertTab("char", ObjClass::TYPE, 4, 0, 1, 0);    
    insertTab("string", ObjClass::TYPE, 5, 0, 1, 0);  
    insertTab("true", ObjClass::CONSTANT, 3, 0, 1, 1); 
    insertTab("false", ObjClass::CONSTANT, 3, 0, 1, 0);
    insertTab("writeln", ObjClass::PROCEDURE, 0, 0, 1, 0); 
    insertTab("readln", ObjClass::PROCEDURE, 0, 0, 1, 0); 
}

void SymbolTable::enterBlock() {
    current_level++;
    int new_block_idx = insertBTab();
    display.push_back(new_block_idx);
}

void SymbolTable::leaveBlock() {
    if (current_level > 0) {
        display.pop_back();
        current_level--;
    }
}

int SymbolTable::insertTab(const std::string& name, ObjClass obj, int type, int ref, int nrm, int adr) {
    int btab_idx = display[current_level];
    int prev_link = btab[btab_idx].last;
    
    TabEntry entry;
    entry.name = name;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    entry.nrm = nrm;
    entry.lev = current_level;
    entry.adr = adr;
    entry.link = prev_link;

    tab.push_back(entry);
    int new_idx = tab.size() - 1;
    btab[btab_idx].last = new_idx;

    return new_idx;
}

int SymbolTable::insertBTab() {
    BTabEntry entry = {0, 0, 0, 0};
    btab.push_back(entry);
    return btab.size() - 1;
}

int SymbolTable::insertATab(int xtyp, int etyp, int eref, int low, int high, int elsz, int size) {
    ATabEntry entry = {xtyp, etyp, eref, low, high, elsz, size};
    atab.push_back(entry);
    return atab.size() - 1;
}

int SymbolTable::lookupLocal(const std::string& name) {
    int btab_idx = display[current_level];
    int curr_link = btab[btab_idx].last;
    while (curr_link != 0) {
        if (tab[curr_link].name == name) {
            return curr_link;
        }
        curr_link = tab[curr_link].link;
    }
    return -1;
}

int SymbolTable::lookup(const std::string& name) {
    for (int i = current_level; i >= 0; --i) {
        int btab_idx = display[i];
        int curr_link = btab[btab_idx].last;
        while (curr_link != 0) {
            if (tab[curr_link].name == name) {
                return curr_link;
            }
            curr_link = tab[curr_link].link;
        }
    }
    
    for (size_t i = 0; i < tab.size(); ++i) {
        if (tab[i].name == name) {
            return i;
        }
    }

    return -1;
}