#include <iostream>
#include <fstream>
#include <string>
#include "lexer.hpp"

int main(){
    std::string filename = "";
    std::cout << ">> Input .txt filename (from test/milestone-1/input/): ";
    std::cin >> filename;

    std::ifstream file;
    file.open("test/milestone-1/input/"+filename+".txt");

    if(!file.is_open()){
        std::cerr << "Error opening file " << filename << "\n";
        return -1;
    }

    lexer(file);

    file.close();

    return 0;
}