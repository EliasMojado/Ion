#include <string>
#include <fstream>
#include "compiler.hpp"

int main(int argc, char *argv[]){
    for (int i = 1; i < argc; ++i) {
        bool is_ion = false;
        for(int j = 0; argv[i][j] != '\0'; ++j) {
            if (argv[i][j] == '.') {
                if (argv[i][j + 1] != 'i' || argv[i][j + 2] != 'o' || argv[i][j + 3] != 'n') {
                    std::cerr << "ERR: File format not recognized\n";
                    return 1;
                }
                is_ion = true;
            }
        }
        if(!is_ion) {
            return 1;
        }

        std::ifstream file(argv[i]);
        if (!file.is_open()) {
            std::cerr << "ERR: File not found\n";
            return 1;
        }

        std::string program, line;
        while (std::getline(file, line)) {
            program += line + '\n';
        }

        compile(argv[i], program);
    }

    return 0;
}
