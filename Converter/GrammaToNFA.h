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
        std::string MyEpsilon = "ε";

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
    std::map<std::string, std::vector<std::string>> grammar;
    std::vector<std::string> nonTerminals;
    std::string firstNonTerminal;
    std::vector<std::string> terminals;
    std::map<std::string, std::string> stateMap;
    std::vector<std::string> m_states;
    std::vector<std::string> m_outputSymbols;
    std::vector<std::string> m_inputSymbols;
    std::vector<std::vector<std::string>> m_transitions;
    const std::regex leftLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?(?:[\w]|ε)(?:\s*\|\s*(?:<\w+>\s+)?(?:[\w]|ε))*)\s*$)");
    const std::regex rightLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*((?:[\w]|ε)(?:\s+<\w+>)?(?:\s*\|\s*(?:[\w]|ε)(?:\s+<\w+>)?)*)\s*$)");
    const std::regex transitionRegex = std::regex(R"(^\s*(?:<(\w+)>)?\s*([\w]|ε)?\s*(?:<(\w+)>)?\s*$)");

    void GetMooreFromLeftGramm()
    {


    }

    void GetMooreFromRightGramm()
    {
        m_states.clear();
        m_outputSymbols.clear();
        m_transitions.clear();
        std::vector<std::vector<std::string>> transitions(terminals.size(), std::vector<std::string>(nonTerminals.size() + 1, ""));
        m_transitions = transitions;
        for (size_t i = 0; i < nonTerminals.size(); ++i) {
            m_states.push_back("q" + std::to_string(i));
            m_outputSymbols.emplace_back("");
        }
        m_states.push_back("q" + std::to_string(nonTerminals.size()));
        m_outputSymbols.emplace_back("F");

        for (const auto& nonTerminal : nonTerminals) {

            // Поиск значения в карте grammar
            auto it = grammar.find(nonTerminal);
            if (it != grammar.end()) {
                std::cout << "Rules:\n";
                std::smatch match;
                for (const auto& rule : it->second) {
                    if (std::regex_match(rule, match, transitionRegex))
                    {
                        AddTransition(nonTerminal, match[2], match[3]);
                    }
                    else
                    {
                        std::cout << rule;
                    }
                }
            } else {
                std::cout << "No rules found for " << nonTerminal << "\n";
            }
            std::cout << "\n";
        }
    }

    void AddTransition(const std::string& state, const std::string& symbol, const std::string& nextState)
    {
        auto j = std::distance(nonTerminals.begin(), std::find(nonTerminals.begin(), nonTerminals.end(), state));
        auto i = std::distance(terminals.begin(), std::find(terminals.begin(), terminals.end(), symbol));
        std::string nState;
        if (nextState.empty())
        {
            nState = "q" + std::to_string(nonTerminals.size());
        }
        else
        {
            nState = "q" + std::to_string(std::distance(nonTerminals.begin(), std::find(nonTerminals.begin(), nonTerminals.end(), nextState)));
        }
        if (i >= m_transitions.size()) {
            m_transitions.resize(i + 1);
        }


        if (j >= m_transitions[i].size()) {
            m_transitions[i].resize(j + 1);
        }
        m_transitions[i][j] = nState;
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

        for (size_t i = 0; i < terminals.size(); ++i)
        {
            file << terminals[i];
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
                if (symbol[0] != '<')
                {
                    if (std::find(terminals.begin(), terminals.end(), symbol) == terminals.end())
                    {
                        terminals.push_back(symbol);
                    }
                }
            }
            grammar[nonTerminal].push_back(option);

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
