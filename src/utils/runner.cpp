#include "header/runner.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#include "header/ast_printer.hpp"

#include "lexer/header/lexer.hpp"
#include "parser/header/parser.hpp"
#include "semantic/header/semantic.hpp"
#include "semantic/header/ast.hpp"
#include "semantic/header/ast_builder.hpp"
#include "interpreter/header/icgen.hpp"

static std::string objClassToStr(ObjClass obj) {
    switch(obj) {
        case ObjClass::CONSTANT:  return "constant";
        case ObjClass::VARIABLE:  return "variable";
        case ObjClass::TYPE:      return "type";
        case ObjClass::PROCEDURE: return "procedure";
        case ObjClass::FUNCTION:  return "function";
        case ObjClass::PROGRAM:   return "program";
        case ObjClass::FIELD:     return "field";
        default:                  return "unknown";
    }
}

static void printTab(const SymbolTable& symtab, std::ostream& out) {
    out << "\n=== SYMBOL TABLE (tab) ===\n";
    out << std::left 
        << std::setw(6)  << "idx" 
        << std::setw(15) << "id" 
        << std::setw(12) << "obj" 
        << std::setw(6)  << "type" 
        << std::setw(6)  << "ref" 
        << std::setw(6)  << "nrm" 
        << std::setw(6)  << "lev" 
        << std::setw(6)  << "adr" 
        << std::setw(6)  << "link" << "\n";
    out << std::string(69, '-') << "\n";
    
    // Print reserved words placeholder (0-32)
    out << "...  (reserved words 0-32)\n";
    
    // Print entries from index 33 onwards
    for (size_t i = 33; i < symtab.tab.size(); ++i) {
        const auto& entry = symtab.tab[i];
        out << std::left 
            << std::setw(6)  << i 
            << std::setw(15) << entry.name 
            << std::setw(12) << objClassToStr(entry.obj)
            << std::setw(6)  << entry.type
            << std::setw(6)  << entry.ref 
            << std::setw(6)  << entry.nrm 
            << std::setw(6)  << entry.lev 
            << std::setw(6)  << entry.adr 
            << std::setw(6)  << entry.link << "\n";
    }
}

// Print the btab (block table) 
static void printBTab(const SymbolTable& symtab, std::ostream& out) {
    out << "\n=== BLOCK TABLE (btab) ===\n";
    out << std::left 
        << std::setw(6)  << "idx" 
        << std::setw(8)  << "last" 
        << std::setw(8)  << "lpar" 
        << std::setw(8)  << "psze" 
        << std::setw(8)  << "vsze" << "\n";
    out << std::string(38, '-') << "\n";
    
    for (size_t i = 0; i < symtab.btab.size(); ++i) {
        const auto& entry = symtab.btab[i];
        out << std::left 
            << std::setw(6)  << i 
            << std::setw(8)  << entry.last
            << std::setw(8)  << entry.lpar 
            << std::setw(8)  << entry.psze 
            << std::setw(8)  << entry.vsze;
        
        // Add descriptive comments for blocks
        if (i == 0) {
            out << "  <- global block (program)";
        }
        out << "\n";
    }
}

// Print the atab (array table) 
static void printATab(const SymbolTable& symtab, std::ostream& out) {
    out << "\n=== ARRAY TABLE (atab) ===\n";
    if (symtab.atab.empty()) {
        out << "(kosong karena tidak ada array)\n";
        return;
    }
    out << std::left 
        << std::setw(6)  << "idx" 
        << std::setw(8)  << "xtyp" 
        << std::setw(8)  << "etyp" 
        << std::setw(8)  << "eref" 
        << std::setw(8)  << "low" 
        << std::setw(8)  << "high" 
        << std::setw(8)  << "elsz" 
        << std::setw(8)  << "size" << "\n";
    out << std::string(62, '-') << "\n";
    
    for (size_t i = 0; i < symtab.atab.size(); ++i) {
        const auto& entry = symtab.atab[i];
        out << std::left 
            << std::setw(6)  << i 
            << std::setw(8)  << entry.xtyp 
            << std::setw(8)  << entry.etyp 
            << std::setw(8)  << entry.eref 
            << std::setw(8)  << entry.low 
            << std::setw(8)  << entry.high 
            << std::setw(8)  << entry.elsz 
            << std::setw(8)  << entry.size << "\n";
    }
}

// Print all three symbol tables (tab, btab, atab)
static void printAllSymbolTables(const SymbolTable& symtab, std::ostream& out) {
    printTab(symtab, out);
    printBTab(symtab, out);
    printATab(symtab, out);
}

// milestone 1
int runMilestone1(const std::string& filename) {
    const std::string in_path  = "test/milestone-1/input/"  + filename + ".txt";
    const std::string out_path = "test/milestone-1/output/" + filename + ".txt";

    std::ifstream input(in_path);
    if (!input.is_open()) {
        std::cerr << "Error: Tidak bisa membuka \"" << in_path << "\"\n";
        return 1;
    }

    std::ofstream output(out_path);
    if (!output.is_open()) {
        std::cerr << "Error: Tidak bisa membuat \"" << out_path << "\"\n";
        return 1;
    }

    lexer(input, output);
    input.close();
    output.close();

    input.open(out_path);
    if(input.is_open()){
        std::cout << "=== Hasil Lexer ===\n";
        std::cout << input.rdbuf();
        input.close();
    }
    return 0;
}

// milestone 2
int runMilestone2(const std::string& filename) {
    const std::string in_path      = "test/milestone-2/input/"  + filename + ".txt";
    const std::string token_path   = "test/milestone-2/tokens/" + filename + ".txt";
    const std::string out_path     = "test/milestone-2/output/" + filename + ".txt";

    std::ifstream srcFile(in_path);
    if (!srcFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuka \"" << in_path << "\"\n"; return 1;
    }

    std::ofstream tokenFile(token_path);
    std::vector<Token> tokens = lexer(srcFile, tokenFile);
    srcFile.close(); tokenFile.close();

    std::ifstream readToken(token_path);
    if(readToken.is_open()){
        std::cout << "=== Lexer Output ===\n" << readToken.rdbuf();
        readToken.close();
    }

    std::cout << "\n=== Parse Tree (CST) ===\n";
    std::ofstream treeFile(out_path);
    if (!treeFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuat output file\n";
        return 1;
    }

    std::ostringstream buf;
    runParser(tokens, buf); 
    std::string result = buf.str();
    std::cout << result;
    treeFile << result;

    treeFile.close();
    return 0;
}

// milestone 3
int runMilestone3(const std::string& filename) {
    const std::string in_path    = "test/milestone-3/input/" + filename + ".txt";
    const std::string token_path = "test/milestone-3/tokens/" + filename + ".txt";
    const std::string pt_path    = "test/milestone-3/parsetree/" + filename + ".txt";
    const std::string out_path   = "test/milestone-3/output/" + filename + ".txt";

    std::ifstream srcFile(in_path);
    if (!srcFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuka file input \"" << in_path << "\"\n"; 
        return 1;
    }

    std::ofstream tokenFile(token_path);
    if (!tokenFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuat file token di \"" << token_path << "\"\n"; 
        return 1;
    }
    std::vector<Token> tokens = lexer(srcFile, tokenFile);
    srcFile.close(); 
    tokenFile.close();
    std::cout << "--- 1. Lexer Selesai (Tokens disimpan di " << token_path << ") ---\n";

    Parser parser(tokens);
    ParseNode* parseTreeRoot = parser.parse(); 

    if (parser.hasErrors()) {
        std::cout << "\n=== SYNTAX ERRORS ===\n";
        for (const auto& err : parser.getErrors()) std::cout << err << "\n";
        std::cout << "Parsing gagal, Semantic Analysis dibatalkan.\n";
        delete parseTreeRoot;
        return 1;
    }

    std::ofstream ptFile(pt_path);
    if (ptFile.is_open()) {
        printTree(parseTreeRoot, ptFile);
        ptFile.close();
        std::cout << "--- 2. Parser Selesai (Parse Tree disimpan di " << pt_path << ") ---\n";
    }

    std::cout << "--- 3. Konversi Parse Tree -> AST ---\n";
    ASTBuilder builder;
    ProgramNode* astRoot = builder.build(parseTreeRoot);

    std::cout << "--- 4. Menjalankan Semantic Analysis ---\n";
    SemanticVisitor semantic;
    if (astRoot) {
        astRoot->accept(semantic); 
    }

    std::ofstream outFile(out_path);
    if (!outFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuat file output di \"" << out_path << "\"\n";
    }

    if (semantic.hasErrors()) {
        std::cout << "\n=== SEMANTIC ERRORS ===\n";
        if (outFile.is_open()) outFile << "=== SEMANTIC ERRORS ===\n";

        semantic.printErrors();
        
        if (outFile.is_open()) {
            for(const auto& err : semantic.errors) {
                outFile << "Semantic Error: " << err << "\n";
            }
        }
    } else {
        std::cout << "\n=== SEMANTIC ANALYSIS BERHASIL (TIDAK ADA ERROR) ===\n";
        if (outFile.is_open()) outFile << "=== SEMANTIC ANALYSIS BERHASIL (TIDAK ADA ERROR) ===\n";
    }

    // Print all three symbol tables (tab, btab, atab)
    printAllSymbolTables(semantic.symtab, std::cout);
    if (outFile.is_open()) printAllSymbolTables(semantic.symtab, outFile);
    
    std::cout << "\n=== DECORATED AST ===\n";
    printAST(astRoot, std::cout, true); 
    
    if (outFile.is_open()) {
        outFile << "\n=== DECORATED AST ===\n";
        printAST(astRoot, outFile, true);
        outFile.close();
        std::cout << "\n>>> Output Semantic Analysis berhasil disimpan di " << out_path << "\n";
    }
    
    delete parseTreeRoot;
    delete astRoot;

    return 0;
}

// milestone 4
int runMilestone4(const std::string& filename) {
    const std::string in_path    = "test/milestone-4/input/"  + filename + ".txt";
    const std::string token_path = "test/milestone-4/tokens/" + filename + ".txt";
    const std::string pt_path    = "test/milestone-4/parsetree/" + filename + ".txt";
    const std::string ast_path   = "test/milestone-4/ast/" + filename + ".txt";
    const std::string out_path   = "test/milestone-4/output/" + filename + ".txt";

    std::ifstream srcFile(in_path);
    if (!srcFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuka file input \"" << in_path << "\"\n";
        return 1;
    }

    std::ofstream tokenFile(token_path);
    if (!tokenFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuat file token di \"" << token_path << "\"\n";
        return 1;
    }
    std::vector<Token> tokens = lexer(srcFile, tokenFile);
    srcFile.close();
    tokenFile.close();
    std::cout << "--- 1. Lexer Selesai ---\n";

    Parser parser(tokens);
    ParseNode* parseTreeRoot = parser.parse();

    if (parser.hasErrors()) {
        std::cout << "\n=== SYNTAX ERRORS ===\n";
        for (const auto& err : parser.getErrors()) std::cout << err << "\n";
        delete parseTreeRoot;
        return 1;
    }

    std::ofstream ptFile(pt_path);
    if (ptFile.is_open()) {
        printTree(parseTreeRoot, ptFile);
        ptFile.close();
    }
    std::cout << "--- 2. Parser Selesai ---\n";

    ASTBuilder builder;
    ProgramNode* astRoot = builder.build(parseTreeRoot);
    std::cout << "--- 3. Konversi Parse Tree -> AST ---\n";

    SemanticVisitor semantic;
    if (astRoot) astRoot->accept(semantic);
    std::cout << "--- 4. Semantic Analysis Selesai ---\n";

    if (semantic.hasErrors()) {
        std::cout << "\n=== SEMANTIC ERRORS ===\n";
        semantic.printErrors();
        std::cout << "Intermediate Code Generation dibatalkan.\n";
        delete parseTreeRoot;
        delete astRoot;
        return 1;
    }

    std::ofstream astFile(ast_path);
    if (astFile.is_open()) {
        astFile << "=== SYMBOL TABLES ===\n";
        printAllSymbolTables(semantic.symtab, astFile);
        astFile << "\n=== DECORATED AST ===\n";
        printAST(astRoot, astFile, true);
        astFile.close();
    }

    IntermediateCodeGen icgen(&semantic.symtab);
    icgen.generate(astRoot);
    std::cout << "--- 5. Intermediate Code Generation Selesai ---\n";

    std::cout << "\n=== INTERMEDIATE CODE ===\n";
    icgen.print(std::cout);

    std::ofstream outFile(out_path);
    if (outFile.is_open()) {
        icgen.print(outFile);
        outFile.close();
        std::cout << "\n>>> Intermediate Code berhasil disimpan di " << out_path << "\n";
    } else {
        std::cerr << "Warning: Tidak bisa membuat file output di \"" << out_path << "\"\n";
    }

    delete parseTreeRoot;
    delete astRoot;
    return 0;
}