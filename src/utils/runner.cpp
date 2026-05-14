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

static void printSymbolTable(const SymbolTable& symtab, std::ostream& out) {
    out << "\n=== SYMBOL TABLE (tab) ===\n";
    out << std::left 
        << std::setw(5)  << "IDX" 
        << std::setw(15) << "NAME" 
        << std::setw(15) << "OBJ_CLASS" 
        << std::setw(5)  << "TYPE" 
        << std::setw(5)  << "LEV" 
        << std::setw(5)  << "LINK" << "\n";
    out << std::string(50, '-') << "\n";
    
    for (size_t i = 33; i < symtab.tab.size(); ++i) {
        std::string obj_str = "";
        switch(symtab.tab[i].obj) {
            case ObjClass::CONSTANT: obj_str = "CONSTANT"; break;
            case ObjClass::VARIABLE: obj_str = "VARIABLE"; break;
            case ObjClass::TYPE: obj_str = "TYPE"; break;
            case ObjClass::PROCEDURE: obj_str = "PROCEDURE"; break;
            case ObjClass::FUNCTION: obj_str = "FUNCTION"; break;
            case ObjClass::PROGRAM: obj_str = "PROGRAM"; break;
            default: obj_str = "UNKNOWN";
        }
        
        out << std::left 
            << std::setw(5)  << i 
            << std::setw(15) << symtab.tab[i].name 
            << std::setw(15) << obj_str 
            << std::setw(5)  << symtab.tab[i].type 
            << std::setw(5)  << symtab.tab[i].lev 
            << std::setw(5)  << symtab.tab[i].link << "\n";
    }
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

    printSymbolTable(semantic.symtab, std::cout);
    if (outFile.is_open()) printSymbolTable(semantic.symtab, outFile);
    
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