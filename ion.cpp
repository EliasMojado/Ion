/**
 * @file ion.cpp
 * @brief This file contains the main function for the Ion compiler.
 */

#include <string>
#include <fstream>
#include "compiler.hpp"

/**
 * @brief The main function of the Ion compiler.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 * @return int The exit status of the program.
 */
int main(int argc, char *argv[]){
    // Iterate through the command-line arguments
    for (int i = 1; i < argc; ++i) {
        bool is_ion = false;
        // Check if the file extension is ".ion"
        for(int j = 0; argv[i][j] != '\0'; ++j) {
            if (argv[i][j] == '.') {
                if (argv[i][j + 1] != 'i' || argv[i][j + 2] != 'o' || argv[i][j + 3] != 'n') {
                    std::cerr << "ERR: File format not recognized\n";
                    return 1;
                }
                is_ion = true;
            }
        }
        // If the file extension is not ".ion", exit with an error
        if(!is_ion) {
            return 1;
        }

        // Open the file
        std::ifstream file(argv[i]);
        if (!file.is_open()) {
            std::cerr << "ERR: File not found\n";
            return 1;
        }

        std::string program, line;
        // Read the contents of the file into a string
        while (std::getline(file, line)) {
            program += line + '\n';
        }

        // Compile the Ion program
        compile(argv[i], program);
    }

    return 0;
}
