#include "header/symtab.hpp"

SymbolTable::SymbolTable() {
    current_level = 0;
    predefined_count = 0;

    BTabEntry global_block = {0, 0, 0, 0};
    btab.push_back(global_block);
    display.push_back(0); 

    initPredefined();
    predefined_count = static_cast<int>(tab.size());
    // Start user-defined chain at 0; predefined entries are not linked
    btab[0].last = 0;
}

void SymbolTable::initPredefined() {
    // Reserved words (indices 0-25): Arion language keywords
    // These occupy fixed positions and are not directly referenced
    const char *reserved_words[] = {
        "and", "array", "begin", "case", "const", "div",     // 0-5
        "downto", "do", "else", "end", "for", "function",    // 6-11
        "if", "mod", "not", "of", "or", "procedure",         // 12-17
        "program", "record", "repeat", "then", "to", "type", // 18-23
        "var", "while"                                       // 24-25
    };

    for (int i = 0; i < 26; ++i) {
        TabEntry entry;
        entry.name = reserved_words[i];
        entry.obj = ObjClass::UNKNOWN;
        entry.type = 0;
        entry.ref = 0;
        entry.nrm = 0;
        entry.lev = 0;
        entry.adr = 0;
        entry.link = 0;
        tab.push_back(entry);
    }

    // Predefined identifiers (indices 26-32)
    // 26: integer (type id = 1)
    // 27: real    (type id = 2)
    // 28: boolean (type id = 3)
    // 29: char    (type id = 4)
    // 30: string  (type id = 5)
    // 31: true    (constant, boolean, value 1)
    // 32: false   (constant, boolean, value 0)

    auto pushPredefined = [&](const std::string &name, ObjClass obj, int type, int ref, int nrm, int adr) {
        TabEntry entry;
        entry.name = name;
        entry.obj = obj;
        entry.type = type;
        entry.ref = ref;
        entry.nrm = nrm;
        entry.lev = 0;
        entry.adr = adr;
        entry.link = 0;
        tab.push_back(entry);
    };

    pushPredefined("integer", ObjClass::TYPE, 1, 0, 1, 0);   // idx 26
    pushPredefined("real", ObjClass::TYPE, 2, 0, 1, 0);      // idx 27
    pushPredefined("boolean", ObjClass::TYPE, 3, 0, 1, 0);   // idx 28
    pushPredefined("char", ObjClass::TYPE, 4, 0, 1, 0);      // idx 29
    pushPredefined("string", ObjClass::TYPE, 5, 0, 1, 0);    // idx 30
    pushPredefined("true", ObjClass::CONSTANT, 3, 0, 1, 1);  // idx 31
    pushPredefined("false", ObjClass::CONSTANT, 3, 0, 1, 0); // idx 32

    // total: 33 entries (indices 0-32), user identifiers start at index 33
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

int SymbolTable::insertTabAtLevel(int level, const std::string &name, ObjClass obj, int type, int ref, int nrm, int adr) {
    int saved_level = current_level;
    current_level = level;
    int idx = insertTab(name, obj, type, ref, nrm, adr);
    current_level = saved_level;
    return idx;
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
    // Check predefined/reserved identifiers
    for (int i = 0; i < predefined_count; ++i) {
        if (tab[i].name == name) {
            return i;
        }
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

    // Fallback: predefined/reserved identifiers (not linked to any block)
    for (int i = 0; i < predefined_count; ++i) {
        if (tab[i].name == name) {
            return i;
        }
    }

    return -1;
}