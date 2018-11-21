#include <iostream>
#include <stdexcept>

#include "halfskat.hpp"
#include "cards.hpp"


using namespace HalfSkat;

Cards::Card RandomPlayer::get_action(std::array<std::vector<Cards::Card>, 3> const& won_cards, std::vector<Cards::Card> const& trick, std::vector<bool> const& played_by_declarer, bool is_declarer) {
    assert(m_cards.empty() == false);
    std::uniform_int_distribution<> distr(0, m_cards.size()-1);
    int card = distr(rng);
    return m_cards[card];
}

static std::ostream& operator<<(std::ostream& os, std::vector<bool> const& vec) {
    int i = 0;
    os << std::boolalpha;
    for (auto e : vec) {
        os << std::to_string(i+1) << ". " << e;
        if (i++ < vec.size()-1) {
            os << ", ";
        }
    }
    return os;
}

Cards::Card HumanPlayer::get_action(std::array<std::vector<Cards::Card>, 3> const& won_cards, std::vector<Cards::Card> const& trick, std::vector<bool> const& played_by_declarer, bool is_declarer) {
    std::cout << "\nCurrent trick: " << trick;
    std::cout << "\nPlayed by declarer: " << played_by_declarer;
    std::cout << "\nYou are declarer: " << is_declarer;
    std::cout << "\nYour current cards: " << m_cards;
    std::cout << "\nEnter card to play: ";
    std::string input;
    bool valid_input = false;
    while (not valid_input) {
        std::getline(std::cin, input);
        int num = std::stoi(input);
        if ((num > m_cards.size()) or (num < 1)) {
            std::cout << "\nEnter valid number. Try again: ";
        }
        else {
            return m_cards.at(num-1);
        }
    }
    throw std::runtime_error("\nCould not get action.");
}

