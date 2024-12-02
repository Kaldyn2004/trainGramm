#ifndef TRAINGRAMM_GRAMMATONFA_H
#define TRAINGRAMM_GRAMMATONFA_H
#include "../stdafx.h"

enum class GrammarType {
    LEFT_LINEAR,
    RIGHT_LINEAR,
    UNKNOWN
};

class GrammarToNFA {
public:
    void printGrammarAndSets()
    {
        std::cout << "Gramm:\n";
        for (const auto& [nonTerminal, rules] : grammar) {
            std::cout << "<" << nonTerminal << "> -> ";
            for (size_t i = 0; i < rules.size(); ++i) {
                for (const auto& symbol : rules[i]) {
                    std::cout << symbol << " ";
                }
                if (i < rules.size() - 1) {
                    std::cout << "| ";
                }
            }
            std::cout << "\n";
        }

        std::cout << "\nNot terminal:\n";
        for (const auto& nt : nonTerminals) {
            std::cout << "<" << nt << ">\n";
        }

        std::cout << "\nterminals:\n";
        for (const auto& t : terminals) {
            std::cout << t << "\n";
        }

        std::cout << (grammarType == GrammarType::LEFT_LINEAR);
    }

    void readGrammarFromFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Error to open file: " + filename);
        }

        std::string line;
        std::string currentRule;
        std::string epsilon = "ε";
        std::string MyEpsilon = "`";


        while (std::getline(file, line))
        {
            line = std::regex_replace(line, std::regex(epsilon), MyEpsilon);
            currentRule += line;
            if (endsWithPipe(currentRule))
            {
                currentRule += " ";
                continue;
            }
            else
            {
                ProcessRule(currentRule);
                currentRule = "";
            }
        }

        file.close();
        if (grammarType == GrammarType::UNKNOWN)
        {
            throw std::runtime_error("Could not determine grammar type.");
        }
    }

    void convertToNFA(const std::string& outputFile)
    {
        grammarType == GrammarType::LEFT_LINEAR
        ? GetMooreFromLeftGramm()
        : GetMooreFromRightGramm();

        PrintMoore(outputFile);
    }

private:
    GrammarType grammarType = GrammarType::UNKNOWN;
    std::map<std::string, std::vector<std::vector<std::string>>> grammar;
    std::vector<std::string> nonTerminals;
    std::string firstNonTerminal;
    std::vector<std::string> terminals;
    std::map<std::string, std::string> stateMap;
    std::vector<std::string> m_states;
    std::vector<std::string> m_outputSymbols;
    std::vector<std::string> m_inputSymbols;
    std::vector<std::vector<std::string>> m_transitions;
    const std::regex leftLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?[\w`](?:\s*\|\s*(?:<\w+>\s+)?[\w`])*)\s*$)");
    const std::regex rightLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*([\w`](?:\s+<\w+>)?(?:\s*\|\s*[\w`](?:\s+<\w+>)?)*)\s*$)");

    void GetMooreFromLeftGramm()
    {
        size_t size = nonTerminals.size();
        m_states[0] = "q0";
        for (size_t i = 0; i < nonTerminals.size(); ++i) {
            m_states[i + 1] = "q" + std::to_string(i + 1);
            m_outputSymbols[i] = nonTerminals[i] == firstNonTerminal ? "F" : "";
        }

    }

    void GetMooreFromRightGramm()
    {
        for (size_t i = 0; i < nonTerminals.size(); ++i) {
            m_states[i] = "q" + std::to_string(i);
            m_outputSymbols[i] = "";
        }
        m_states[nonTerminals.size() + 1] = "q" + std::to_string(nonTerminals.size() + 1);
        m_outputSymbols[nonTerminals.size() + 1] = "F";




    }

    void PrintMoore(const std::string& fileName)
    {
        std::ofstream file(fileName);
        if (!file.is_open())
        {
            std::cerr << "Error: Unable to write to file " << fileName << std::endl;
            exit(1);
        }

        for (const std::string &outputSymbol : m_outputSymbols)
        {
            file<< ";" << outputSymbol;
        }
        file << std::endl;

        for (const std::string &state : m_states)
        {
            file << ";" << state;
        }
        file << std::endl;

        for (size_t i = 0; i < m_inputSymbols.size(); ++i)
        {
            file << m_inputSymbols[i];
            for (const auto &transition : m_transitions[i]) {
                file << ";" << transition;
            }
            file << std::endl;
        }

        file.close();
    }

    void parseRule(const std::string& nonTerminal, const std::string& rules)
    {
        std::stringstream ruleStream(rules);
        std::string option;
        nonTerminals.push_back(nonTerminal);
        if (firstNonTerminal.empty())
        {
            firstNonTerminal = nonTerminal;
        }

        while (std::getline(ruleStream, option, '|'))
        {
            std::stringstream optionStream(option);
            std::vector<std::string> production;
            std::string symbol;

            while (optionStream >> symbol)
            {
                production.push_back(symbol);
                if (symbol[0] == '<')
                {
                } else if (symbol != "`")
                {
                    if (std::find(terminals.begin(), terminals.end(), symbol) == terminals.end())
                    {
                        terminals.push_back(symbol);
                    }
                }
            }

            grammar[nonTerminal].push_back(production);
        }
    }

    static bool endsWithPipe(const std::string& line) {
        auto it = std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        });

        return (it != line.rend() && *it == '|');
    }

    void ProcessRule(const std::string& rule) {
        std::smatch match;
        if (std::regex_match(rule, match, leftLinearRegex)) {
            if (grammarType == GrammarType::UNKNOWN) grammarType = GrammarType::LEFT_LINEAR;

            parseRule(match[1], match[2]);
        } else if (std::regex_match(rule, match, rightLinearRegex)) {
            if (grammarType == GrammarType::UNKNOWN) grammarType = GrammarType::RIGHT_LINEAR;

            parseRule(match[1], match[2]);
        } else {
            throw std::runtime_error("Invalid string format: " + rule);
        }
    }
};

#endif //TRAINGRAMM_GRAMMATONFA_H
