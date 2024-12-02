#ifndef TRAINGRAMM_GRAMMATONFA_H
#define TRAINGRAMM_GRAMMATONFA_H
#include "../stdafx.h"

enum class GrammarType {
    LEFT_LINEAR,
    RIGHT_LINEAR,
    UNKNOWN
};

class GrammarToNFA {
private:
    GrammarType grammarType = GrammarType::UNKNOWN;
    std::map<std::string, std::vector<std::vector<std::string>>> grammar;
    std::set<std::string> nonTerminals;
    std::set<std::string> terminals;
    std::map<std::string, std::string> stateMap; // Сопоставление нетерминалов состояниям НКА
    int stateCounter = 0;
    const std::regex leftLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?[\wε](?:\s*\|\s*(?:<\w+>\s+)?[\wε])*)\s*$)");
    const std::regex rightLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*([\wε](?:\s+<\w+>)?(?:\s*\|\s*[\wε](?:\s+<\w+>)?)*)\s*$)");

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

        while (std::getline(file, line))
        {
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

    void convertToNFA(const std::string& outputFile)
    {
        assignStates();

        std::ofstream out(outputFile);
        if (!out.is_open()) {
            throw std::runtime_error("Error to open file: " + outputFile);
        }

        // Пишем заголовок таблицы
        out << ";;;;;;F\n";
        out << ";";
        for (const auto& [nonTerminal, state] : stateMap) {
            out << state << ";";
        }
        out << "\n";

        for (const auto& [nonTerminal, productions] : grammar) {
            for (const auto& production : productions) {
                std::string fromState = stateMap[nonTerminal];
                std::string toState = production.size() > 1 ? stateMap[production.back()] : "F";
                std::string symbol = production[0] == "ε" ? "ε" : production[0];

                out << symbol << ";";
                for (const auto& [nt, state] : stateMap) {
                    if (state == fromState || state == toState) {
                        out << state << ";";
                    } else {
                        out << ";";
                    }
                }
                out << "\n";
            }
        }

        out.close();
    }

private:
    void parseRule(const std::string& nonTerminal, const std::string& rules)
    {
        std::stringstream ruleStream(rules);
        std::string option;
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
                } else if (symbol != "ε")
                {
                    terminals.insert(symbol);
                }
            }

            grammar[nonTerminal].push_back(production);
        }
        nonTerminals.insert(nonTerminal);
    }

    void assignStates() {
        for (const auto& nonTerminal : nonTerminals) {
            stateMap[nonTerminal] = "q" + std::to_string(stateCounter++);
        }
        stateMap["F"] = "q" + std::to_string(stateCounter++); // Финальное состояние
    }

    static bool endsWithPipe(const std::string& line) {
        auto it = std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        });

        return (it != line.rend() && *it == '|');
    }
};

#endif //TRAINGRAMM_GRAMMATONFA_H
