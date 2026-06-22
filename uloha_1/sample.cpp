#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "sample.h"
#endif

std::multimap <std::string, uint8_t> STATE_TO_SYMBOL;

std::string generateId(char ch) {
    static size_t counter = 145;
    return /*"khj`[]1!hj_" +*/ ch + std::to_string(counter++);
}

class CNode {
public:
    // could be set
    std::set<std::string> beginnings;
    std::set<std::string> ends;

    // could use multimap to be faster
    std::multimap<std::string, std::string> neighbors;
    bool emptyString;

    void resolveState(std::string stateID, uint8_t symbol, std::shared_ptr<std::set<std::string>> newStates) {
        // pro kazdyho souseda stavu stateID se podivam, pokud ma v mape symbol, pokud ano, pridam souseda do newStates
        auto range_state = neighbors.equal_range(stateID);
        for (auto it1 = range_state.first; it1 != range_state.second; ++it1)
        {
            auto range_second = STATE_TO_SYMBOL.equal_range(it1->second);
            for (auto it2 = range_second.first; it2 != range_second.second; ++it2) {
                if (it2->second == symbol) {
                    newStates->insert(it1->second);
                }
            }
        }
    }

    bool stringBelongsToAutomaton(Word word) {
        std::shared_ptr<std::set<std::string>> currentStates = std::make_shared<std::set<std::string>>();
        currentStates->insert(beginnings.begin(), beginnings.end());

        for (auto & symbol : word) {
            std::shared_ptr<std::set<std::string>> currentStatesNew = std::make_shared<std::set<std::string>>();

            for (auto & state : *currentStates)
                resolveState(state, symbol, currentStatesNew);

            currentStates = currentStatesNew;
        }

        for (auto state: *currentStates) {
            auto it = std::find(ends.begin(), ends.end(), state);
            if (it != ends.end())
                return true;

        }
        return false;
    }

    void addInitialState() {
        std::string initialState = generateId('S');
        for (const auto & begin : beginnings)
            neighbors.insert({initialState, begin});

        beginnings.clear();
        beginnings.insert(initialState);

        if (emptyString)
            ends.insert(initialState);
    }


};


std::shared_ptr<CNode> solveNode(const regexp::RegExp& n) {
    std::shared_ptr<CNode> node = std::make_shared<CNode>();
    
        std::visit(overloaded{
            [node](const std::unique_ptr<regexp::Alternation>& arg) {//+
                std::shared_ptr<CNode> left = solveNode(arg->m_left);
                std::shared_ptr<CNode> right = solveNode(arg->m_right);

                // begins
                node->beginnings.insert(left->beginnings.begin(), left->beginnings.end());
                node->beginnings.insert(right->beginnings.begin(), right->beginnings.end());

                //ends
                node->ends.insert(left->ends.begin(), left->ends.end());
                node->ends.insert(right->ends.begin(), right->ends.end());

                //neighbors
                // node->neighbors.insert(node->neighbors.end(),
                // left->neighbors.begin(), left->neighbors.end());
                // node->neighbors.insert(node->neighbors.end(),
                // right->neighbors.begin(), right->neighbors.end());

                for (const auto& pair : left->neighbors)
                    node->neighbors.insert(pair);

                for (const auto& pair : right->neighbors)
                    node->neighbors.insert(pair);


                //emptyString
                node->emptyString = (left->emptyString || right->emptyString);

            },
            [node](const std::unique_ptr<regexp::Concatenation>& arg) { //.
                std::shared_ptr<CNode> left = solveNode(arg->m_left);
                std::shared_ptr<CNode> right = solveNode(arg->m_right);

                //begins
                node->beginnings.insert(left->beginnings.begin(), left->beginnings.end());
                if (left->emptyString) {
                    node->beginnings.insert(right->beginnings.begin(), right->beginnings.end());
                }

                // ends
                node->ends.insert(right->ends.begin(), right->ends.end());
                if (right->emptyString) {
                    node->ends.insert(left->ends.begin(), left->ends.end());
                }

                // neighbors
                // node->neighbors.insert(node->neighbors.end(),
                //     left->neighbors.begin(), left->neighbors.end());
                // node->neighbors.insert(node->neighbors.end(),
                //     right->neighbors.begin(), right->neighbors.end()
                // );
                for (const auto& pair : left->neighbors)
                    node->neighbors.insert(pair);

                for (const auto& pair : right->neighbors)
                    node->neighbors.insert(pair);


                //left ends + right beginnings
                for (auto & end : left->ends) {
                    for (auto & begin : right->beginnings) {
                        node->neighbors.insert({end, begin});
                    }
                }


                // emptyString
                node->emptyString = (left->emptyString && right->emptyString);

            },
            [node](const std::unique_ptr<regexp::Iteration>& arg) {//*
                std::shared_ptr<CNode> heir = solveNode(arg->m_node);

                // beginings
                node->beginnings.insert(heir->beginnings.begin(), heir->beginnings.end());

                // ends
                node->ends.insert(heir->ends.begin(), heir->ends.end());

                // neighbors
                // node->neighbors.insert(node->neighbors.end(),
                //     heir->neighbors.begin(), heir->neighbors.end()
                // );
                for (const auto& pair : heir->neighbors)
                    node->neighbors.insert(pair);



                // ends + beginnings
                for (auto & end : heir->ends) {
                    for (auto & begin : heir->beginnings) {
                        node->neighbors.insert({end, begin});
                    }
                }


                // emptyString
                node->emptyString = true;
            },
            [node](const std::unique_ptr<regexp::Symbol>& arg) {
                std::string id = generateId(arg->m_symbol);

                STATE_TO_SYMBOL.insert({id, arg->m_symbol});

                node->beginnings.insert(node->beginnings.end(), id);
                node->ends.insert(node->ends.end(), id);
                node->emptyString = false;


            },
            [node](const std::unique_ptr<regexp::Epsilon>& arg) {
                node->emptyString = true;
            },
            [node](const std::unique_ptr<regexp::Empty>& arg) {
                node->emptyString = false;
            },
            },
        n);
        return node;
    }


std::set<size_t> wordsMatch(const regexp::RegExp& regexp, const std::vector<Word>& words)
{
    std::set<size_t> result;

    auto rootNode = std::make_shared<CNode>();
    rootNode = solveNode(regexp);
    rootNode->addInitialState();



    for (size_t i = 0; i < words.size(); i++) {
        if (rootNode->stringBelongsToAutomaton(words[i])) {
            result.insert(i);
        }
    }

    STATE_TO_SYMBOL.clear();
    return result;
}

#ifndef __PROGTEST__


void test() {
    regexp::RegExp re0 =
                    std::make_unique<regexp::Alternation>(
                        std::make_unique<regexp::Symbol>('a'),
                        std::make_unique<regexp::Symbol>('b'));

    assert((wordsMatch(re0, {Word{'a'}}) == std::set<size_t>{0}));

    // basic test 1
    regexp::RegExp re1 = std::make_unique<regexp::Iteration>(
        std::make_unique<regexp::Concatenation>(
            std::make_unique<regexp::Concatenation>(
                std::make_unique<regexp::Concatenation>(
                    std::make_unique<regexp::Iteration>(
                        std::make_unique<regexp::Alternation>(
                            std::make_unique<regexp::Symbol>('a'),
                            std::make_unique<regexp::Symbol>('b'))),
                    std::make_unique<regexp::Symbol>('a')),
                std::make_unique<regexp::Symbol>('b')),
            std::make_unique<regexp::Iteration>(
                std::make_unique<regexp::Alternation>(
                    std::make_unique<regexp::Symbol>('a'),
                    std::make_unique<regexp::Symbol>('b')))));
    assert(wordsMatch(re1, {Word{}}) == std::set<size_t>{0});
    assert(wordsMatch(re1, {Word{'a', 'b'}}) == std::set<size_t>{0});
    assert(wordsMatch(re1, {Word{'a'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 'a', 'a'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 'a', 'c'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 0x07, 'c'}}) == std::set<size_t>{});
    assert(wordsMatch(re1, {Word{'a', 'a', 'b'}}) == std::set<size_t>{0});
    assert(wordsMatch(re1, {Word{'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b'}}) == std::set<size_t>{0});
    assert((wordsMatch(re1, {Word{}, Word{'a', 'b'}, Word{'a'}, Word{'a', 'a', 'a', 'a'}, Word{'a', 'a', 'a', 'c'}, Word{'a', 'a', 0x07, 'c'}, Word{'a', 'a', 'b'}, Word{'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b', 'a', 'a', 'b'}}) == std::set<size_t>{0, 1, 6, 7}));

    // basic test 2
    regexp::RegExp re2 = std::make_unique<regexp::Concatenation>(
        std::make_unique<regexp::Concatenation>(
            std::make_unique<regexp::Iteration>(
                std::make_unique<regexp::Concatenation>(
                    std::make_unique<regexp::Concatenation>(
                        std::make_unique<regexp::Iteration>(
                            std::make_unique<regexp::Alternation>(
                                std::make_unique<regexp::Symbol>('a'),
                                std::make_unique<regexp::Symbol>('b'))),
                        std::make_unique<regexp::Iteration>(
                            std::make_unique<regexp::Alternation>(
                                std::make_unique<regexp::Symbol>('c'),
                                std::make_unique<regexp::Symbol>('d')))),
                    std::make_unique<regexp::Iteration>(
                        std::make_unique<regexp::Alternation>(
                            std::make_unique<regexp::Symbol>('e'),
                            std::make_unique<regexp::Symbol>('f'))))),
            std::make_unique<regexp::Empty>()),
        std::make_unique<regexp::Iteration>(
            std::make_unique<regexp::Alternation>(
                std::make_unique<regexp::Symbol>('a'),
                std::make_unique<regexp::Symbol>('b'))));
    assert(wordsMatch(re2, {Word{}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b'}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b', 'c', 'd'}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b', 'c', 'd', 'e', 'f'}}) == std::set<size_t>{});
    assert(wordsMatch(re2, {Word{'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}}) == std::set<size_t>{});
    assert((wordsMatch(re2, {Word{}, Word{'a', 'b'}, Word{'a', 'b', 'c', 'd'}, Word{'a', 'b', 'c', 'd', 'e', 'f'}, Word{'a', 'b', 'c', 'd', 'e', 'f', 'a', 'b'}}) == std::set<size_t>{}));

    // basic test 3
    regexp::RegExp re3 = std::make_unique<regexp::Concatenation>(
        std::make_unique<regexp::Concatenation>(
            std::make_unique<regexp::Concatenation>(
                std::make_unique<regexp::Symbol>('0'),
                std::make_unique<regexp::Symbol>('1')),
            std::make_unique<regexp::Symbol>('1')),
        std::make_unique<regexp::Iteration>(
            std::make_unique<regexp::Alternation>(
                std::make_unique<regexp::Alternation>(
                    std::make_unique<regexp::Concatenation>(
                        std::make_unique<regexp::Concatenation>(
                            std::make_unique<regexp::Symbol>('0'),
                            std::make_unique<regexp::Symbol>('1')),
                        std::make_unique<regexp::Symbol>('1')),
                    std::make_unique<regexp::Concatenation>(
                        std::make_unique<regexp::Concatenation>(
                            std::make_unique<regexp::Symbol>('1'),
                            std::make_unique<regexp::Iteration>(
                                std::make_unique<regexp::Symbol>('0'))),
                        std::make_unique<regexp::Symbol>('1'))),
                std::make_unique<regexp::Symbol>('0'))));
    assert(wordsMatch(re3, {Word{'0', '1'}}) == std::set<size_t>{});
    assert(wordsMatch(re3, {Word{'0', '1', '1'}}) == std::set<size_t>{0});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0'}}) == std::set<size_t>{0});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '0'}}) == std::set<size_t>{});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1'}}) == std::set<size_t>{0});
    assert(wordsMatch(re3, {Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1', '0'}}) == std::set<size_t>{0});
    assert((wordsMatch(re3, {Word{'0', '1'}, Word{'0', '1', '1'}, Word{'0', '1', '1', '0'}, Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '0'}, Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1'}, Word{'0', '1', '1', '0', '1', '1', '1', '0', '0', '1', '0'}}) == std::set<size_t>{1, 2, 4, 5}));
    std::cout << "All initial tests passed!" << std::endl;
}

void test2() {
    regexp::RegExp re0 =
        std::make_unique<regexp::Concatenation>(
                std::make_unique<regexp::Alternation>(
                    std::make_unique<regexp::Epsilon>(),
                    std::make_unique<regexp::Iteration>(
                        std::make_unique<regexp::Alternation>(
                            std::make_unique<regexp::Symbol>('a'),
                            std::make_unique<regexp::Epsilon>()))),
                std::make_unique<regexp::Epsilon>());

    // auto tmp = wordsMatch(re0, {Word{'a'}});
    // assert((wordsMatch(re0, {Word{'a'}, Word{'a', 'a'}, Word{''}}) == std::set<size_t>{0, 1}));
}

int main()
{
    test();

    // TODO - nekde se cyklime
    test2();
    return 0;
}
#endif
