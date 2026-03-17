#include <iostream>
#include <fstream>
#include <string>
#include "lexer.hpp"

int main(){
    std::string filename = "";
    std::cout << ">> Input .txt filename (from test/milestone-1/input/): ";
    std::cin >> filename;

    std::ifstream input;
    input.open("test/milestone-1/input/"+filename+".txt");

    if(!input.is_open()){
        std::cerr << "Error opening file " << filename << "\n";
        return -1;
    }

    std::ofstream output("test/milestone-1/output/"+filename+".txt"); 

    if(!output.is_open()){
        std::cerr << "Error creating file " << "\n";
        return -1;
    }

    lexer(input, output);

    input.close();
    output.close();

    input.open("test/milestone-1/output/"+filename+".txt");

    if(input.is_open()){
        std::cout << input.rdbuf();
    }

    return 0;
}