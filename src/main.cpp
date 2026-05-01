#include <iostream>
#include <fstream>
#include <string>
#include "lexer/header/lexer.hpp"
#include "parser/header/parser.hpp"

// milestone 1
static int runMilestone1(const std::string& filename) {
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
static int runMilestone2(const std::string& filename) {
    const std::string in_path      = "test/milestone-2/input/"  + filename + ".txt";
    const std::string token_path   = "test/milestone-2/tokens/" + filename + ".txt";
    const std::string out_path     = "test/milestone-2/output/" + filename + ".txt";

    // lexer
    std::ifstream srcFile(in_path);
    if (!srcFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuka \"" << in_path << "\"\n";
        return 1;
    }

    std::ofstream tokenFile(token_path);
    if (!tokenFile.is_open()) {
        std::cerr << "Error: Tidak bisa membuat file token\n";
        return 1;
    }

    std::vector<Token> tokens = lexer(srcFile, tokenFile);
    srcFile.close();
    tokenFile.close();

    std::ifstream readToken(token_path);
    if(readToken.is_open()){
        std::cout << "=== Lexer Output ===\n";
        std::cout << readToken.rdbuf();
        readToken.close();
    }

    // parser
    std::cout << "\n=== Parse Tree ===\n";

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

int main() {
    int milestone = 0;
    std::cout << ">> Pilih milestone (1 atau 2): ";
    std::cin  >> milestone;

    std::string filename;
    if (milestone == 1) {
        std::cout << ">> Masukkan nama file (dari test/milestone-1/input/, tanpa .txt): ";
        std::cin  >> filename;
        return runMilestone1(filename);
    } else if (milestone == 2) {
        std::cout << ">> Masukkan nama file (dari test/milestone-2/input/, tanpa .txt): ";
        std::cin  >> filename;
        return runMilestone2(filename);
    } else {
        std::cerr << "Milestone belum ada :(" << milestone << "\n";
        return 1;
    }
}