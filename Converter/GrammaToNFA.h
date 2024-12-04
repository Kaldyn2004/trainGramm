#ifndef GRAMMAR_TO_NFA_H
#define GRAMMAR_TO_NFA_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <iterator>

enum class GrammarType
{
    LEFT_LINEAR,
    RIGHT_LINEAR,
    UNKNOWN
};

class GrammarToNFA
{
public:
    void readGrammar(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        std::string line;
        std::string accumLine;
        while (std::getline(file, line))
        {
            accumLine += line;
            if (line.back() != '|')
            {
                processLine(accumLine);
                accumLine = "";
            }
        }

        if (grammarType == GrammarType::UNKNOWN)
        {
            throw std::runtime_error("Could not determine grammar type from rules.");
        }
    }

    void convertToNFA(const std::string& outputFile)
    {
        PrepareStates();
        ProcessTransitions();
        saveNFA(outputFile);
    }

    void printGrammar() const
    {
        std::cout << "Grammar:\n";
        for (const auto& [nonTerminal, rules] : grammar)
        {
            std::cout << "<" << nonTerminal << "> -> ";
            for (size_t i = 0; i < rules.size(); ++i) {
                std::cout << rules[i] << (i < rules.size() - 1 ? " | " : "");
            }
            std::cout << "\n";
        }
    }

private:
    GrammarType grammarType = GrammarType::UNKNOWN;
    std::map<std::string, std::vector<std::string>> grammar;
    std::vector<std::string> nonTerminals;
    std::vector<std::string> terminals;
    std::vector<std::string> states;
    std::string finalStates;
    std::vector<std::vector<std::string>> transitions;

    const std::regex leftLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*((?:<\w+>\s+)?(?:[\w]|ε)(?:\s*\|\s*(?:<\w+>\s+)?(?:[\w]|ε))*)\s*$)");
    const std::regex rightLinearRegex = std::regex(R"(^\s*<(\w+)>\s*->\s*((?:[\w]|ε)(?:\s+<\w+>)?(?:\s*\|\s*(?:[\w]|ε)(?:\s+<\w+>)?)*)\s*$)");

    void processLine(const std::string& line)
    {
        std::smatch match;
        if (std::regex_match(line, match, leftLinearRegex))
        {
            if (grammarType == GrammarType::UNKNOWN) grammarType = GrammarType::LEFT_LINEAR;
            parseRule(match[1], match[2]);
        } else if (std::regex_match(line, match, rightLinearRegex))
        {
            if (grammarType == GrammarType::UNKNOWN) grammarType = GrammarType::RIGHT_LINEAR;
            parseRule(match[1], match[2]);
        } else {
            throw std::runtime_error("Invalid rule format: " + line);
        }
    }

    void parseRule(const std::string& nonTerminal, const std::string& rules)
    {
        if (std::find(nonTerminals.begin(), nonTerminals.end(), nonTerminal) == nonTerminals.end())
        {
            nonTerminals.push_back(nonTerminal);
        }

        std::istringstream ruleStream(rules);
        std::string option;
        while (std::getline(ruleStream, option, '|'))
        {
            grammar[nonTerminal].push_back(option);

            for (const auto& symbol : splitString(option))
            {
                if (!symbol.empty() && !isNonTerminal(symbol) && std::find(terminals.begin(), terminals.end(), symbol) == terminals.end())
                {
                    terminals.push_back(symbol);
                }
            }
        }
    }

    void PrepareStates()
    {
        states.clear();
        transitions.assign(terminals.size(), std::vector<std::string>(nonTerminals.size() + 1, ""));
        for (size_t i = 0; i <= nonTerminals.size(); ++i)
        {
            states.push_back("q" + std::to_string(i));
        }
        (grammarType == GrammarType::RIGHT_LINEAR)
            ? finalStates = "q" + std::to_string(nonTerminals.size())
            : finalStates = "q0";
    }

    void ProcessTransitions()
    {
        for (size_t i = 0; i < nonTerminals.size(); ++i)
        {
            const auto& nonTerminal = nonTerminals[i];
            const auto& rules = grammar[nonTerminal];

            for (const auto& rule : rules)
            {
                const auto& parts = splitString(rule);
                std::string symbol = parts[0];
                std::string nextState;
                if (parts.size() > 1)
                {
                    auto start = parts[1].find('<');
                    auto end = parts[1].find('>');
                    nextState = (parts.size() > 1 ? parts[1].substr(start + 1, end - start - 1): "");
                }

                size_t terminalIndex = findIndex(terminals, symbol);
                size_t stateIndex = findIndex(nonTerminals, nextState);
                std::cout << symbol << std::endl << nextState << "<" << std::endl;
                if (terminalIndex < terminals.size() && stateIndex < states.size())
                {
                    addTransition(terminalIndex, i, "q" + std::to_string(stateIndex));
                }
                else if (terminalIndex < terminals.size())
                {
                    addTransition(terminalIndex, i, "q" + std::to_string(nonTerminals.size()));
                }
            }
        }
    }

    void addTransition(size_t terminalIndex, size_t stateIndex, const std::string& targetState)
    {
        if (transitions[terminalIndex][stateIndex].empty())
        {
            transitions[terminalIndex][stateIndex] = targetState;
        }
        else
        {
            transitions[terminalIndex][stateIndex] += "," + targetState;
        }
    }

    void saveNFA(const std::string& outputFile) const
    {
        std::ofstream file(outputFile);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + outputFile);
        }

        // Печать выходных символов
        for (const auto& state : states)
        {
            file << ((state == finalStates) ? ";F": ";");
        }
        file << std::endl;

        // Печать состояний
        for (const auto& state : states)
        {
            file << ";" << state;
        }
        file << std::endl;

        // Печать переходов
        for (size_t i = 0; i < terminals.size(); ++i)
        {
            file << terminals[i];
            for (const auto& transition : transitions[i])
            {
                file << ";" << (transition.empty() ? "" : transition);
            }
            file << std::endl;;
        }

        file.close();
    }

    static bool isNonTerminal(const std::string& symbol)
    {
        return !symbol.empty() && symbol.front() == '<' && symbol.back() == '>';
    }

    static std::vector<std::string> splitString(const std::string& str)
    {
        std::istringstream stream(str);
        return {std::istream_iterator<std::string>{stream}, std::istream_iterator<std::string>{}
        };
    }

    static size_t findIndex(const std::vector<std::string>& vec, const std::string& value)
    {
        auto it = std::find(vec.begin(), vec.end(), value);
        return it != vec.end() ? std::distance(vec.begin(), it) : vec.size();
    }
};

#endif // GRAMMAR_TO_NFA_H
