#include "Converter/GrammaToNFA.h"
#include "stdafx.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " grammar.txt output.csv";
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];

    try
    {
        GrammarToNFA converter;
        converter.readGrammarFromFile(inputFile);
        converter.printGrammarAndSets();
        converter.convertToNFA(outputFile);
    } catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
