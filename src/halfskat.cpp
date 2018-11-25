#include <iostream>
#include <stdexcept>

#include "halfskat.hpp"
#include "cards.hpp"


using namespace HalfSkat;

Cards::Card RandomPlayer::get_action(ObservableState const& state, int player_id) {
    assert(m_cards.empty() == false);
    std::uniform_int_distribution<> distr(0, m_cards.size()-1);
    int card = distr(rng);
    return m_cards[card];
}

static std::ostream& operator<<(std::ostream& os, std::vector<int> const& vec) {
    int i = 0;
    for (auto e : vec) {
        os << std::to_string(i+1) << ". " << e;
        if (i++ < vec.size()-1) {
            os << ", ";
        }
    }
    return os;
}

Cards::Card HumanPlayer::get_action(ObservableState const& state, int player_id) {
    std::cout << "\nCurrent trick: " << state.trick;
    std::cout << "\nPlayed by: " << state.trick_players;
    std::cout << "\nDeclarer: " << state.declarer;
    std::cout << "\nYou are: " << player_id;
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

