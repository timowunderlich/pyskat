#include <iostream>
#include <stdexcept>

#include "halfskat.hpp"
#include "cards.hpp"


using namespace HalfSkat;

Cards::Card Player::get_action(ObservableState const& state, int player_id) {
    m_last_state = PlayerState(state, m_cards, player_id);
    Cards::Card action = query_policy();
    m_last_action = action;
    return action;
}

void Player::put_transition(int const reward, ObservableState const& new_state, int player_id) {
    BOOST_LOG_TRIVIAL(debug) << "Putting new transition for player " << std::to_string(player_id) << ", reward: " << std::to_string(reward) << ", action: " << m_last_action;
    m_transitions.push_back(Transition(m_last_state, PlayerState(new_state, m_cards, player_id), reward, m_last_action));
}

Cards::Card RandomPlayer::query_policy() {
    assert(m_cards.empty() == false);
    std::uniform_int_distribution<> distr(0, m_cards.size()-1);
    int card = distr(rng);
    return m_cards[card];
}

Cards::Card HumanPlayer::query_policy() {
    std::cout << "\nCurrent trick: " << m_last_state.trick;
    std::cout << "\nPlayed by: ";
    for (auto c : m_last_state.trick) {
        BOOST_LOG_TRIVIAL(debug) << std::to_string(c.played_by) << " ";
    }
    std::cout << "\nYou are declarer: " << m_last_state.is_declarer;
    std::cout << "\nYour current cards: " << m_cards;
    std::cout << "\nEnter card to play: ";
    std::string input;
    bool valid_input = false;
    while (not valid_input) {
        std::getline(std::cin, input);
        size_t num = std::stoi(input);
        if ((num > m_cards.size()) or (num < 1)) {
            std::cout << "\nEnter valid number. Try again: ";
        }
        else {
            return m_cards.at(num-1);
        }
    }
    throw std::runtime_error("\nCould not get action.");
}

