#include <iostream>
#include <string>
#include "utils/header/runner.hpp"

int main() {
    int milestone = 0;
    std::cout << ">> Pilih milestone (1, 2, atau 3): ";
    std::cin  >> milestone;

    std::string filename;
    std::cout << ">> Masukkan nama file (tanpa .txt): ";
    std::cin  >> filename;

    if (milestone == 1) {
        return runMilestone1(filename);
    } else if (milestone == 2) {
        return runMilestone2(filename);
    } else if (milestone == 3) {
        return runMilestone3(filename);
    } else {
        std::cerr << "Milestone belum ada :(" << milestone << "\n";
        return 1;
    }
}