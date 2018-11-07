//#pragma once

#include <memory>
#include <vector>
#include <boost/log/trivial.hpp>
#include <cassert>
#include <algorithm>

#include "cards.hpp"

namespace HalfSkat {

static const int cards_per_player = 10;
static const int cards_in_skat = 2;

class Player {
    template<class FirstPlayerT, class SecondPlayerT, class ThirdPlayerT>
    friend class Game;
    public:
        virtual Cards::Card get_action(std::vector<Cards::Card> const& trick) = 0;
        int get_points() { return m_points; }
    protected:
        std::vector<Cards::Card> m_cards;
        int m_points = 0;
};

class AIPlayer : public Player {
    
    public:
        using Player::Player;
        Cards::Card get_action(std::vector<Cards::Card> const& trick) override;
};

class HumanPlayer : public Player {
    public:
        using Player::Player;
        Cards::Card get_action(std::vector<Cards::Card> const& trick) override;
};


template<class FirstPlayerT, class SecondPlayerT, class ThirdPlayerT>
class Game {
    public:
        Game(int const max_rounds = 10) : max_rounds(max_rounds) {
            BOOST_LOG_TRIVIAL(debug) << "Constructing new Game.";
            std::vector<Cards::Card> cards = Cards::get_full_shuffled_deck();
            assert(cards.size() == (cards_per_player*3 + cards_in_skat));
            first_player.m_cards.resize(cards_per_player);
            second_player.m_cards.resize(cards_per_player);
            third_player.m_cards.resize(cards_per_player);
            skat.resize(cards_in_skat);
            std::move(cards.begin(), cards.begin() + cards_per_player, first_player.m_cards.begin());
            std::move(cards.begin() + cards_per_player, cards.begin() + 2*cards_per_player, second_player.m_cards.begin());
            std::move(cards.begin() + 2*cards_per_player, cards.begin() + 3*cards_per_player, third_player.m_cards.begin());
            std::move(cards.begin() + 3*cards_per_player, cards.end(), skat.begin());
            BOOST_LOG_TRIVIAL(debug) << "First player: " << first_player.m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Second player: " << second_player.m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Third player: " << third_player.m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Skat: " << skat;
        }
        std::vector<Cards::Card> get_legal_cards(std::vector<Cards::Card> const& players_cards) {
            std::vector<Cards::Card> copy = players_cards;
            if (trick.empty()) {
                return copy;
            }
            Cards::Color sc = trick.front().color; // First card determines color to be played
            Cards::Rank sr = trick.front().rank;
            // First, determine cards that would follow suit
            std::vector<Cards::Card> legals = copy;
            if ((sr == Cards::Rank::Jack) || (sc == trump)) { // Trump is single suit
                legals.erase(std::remove_if(legals.begin(), legals.end(), [this](Cards::Card const& c) {
                    return ((c.rank != Cards::Rank::Jack) && (c.color != this->trump));
                }), legals.end());
            }
            else {
                legals.erase(std::remove_if(legals.begin(), legals.end(), [sc](Cards::Card const& c) {
                    return (c.color != sc);
                }), legals.end());
            }
            // Check if player can follow suit
            bool legal_found = !legals.empty();
            if (legal_found) { // Player can follow suit
                return legals;
            }
            else { // Player cannot follow suit
                return copy; 
            }
        }
        Cards::Color const trump = Cards::Color::Clubs;
        std::vector<Cards::Card> get_trick() { return trick; }
        void step();
        bool new_round = true;
        int round = 0;

    protected:
        FirstPlayerT first_player;
        SecondPlayerT second_player;
        ThirdPlayerT third_player;
        std::vector<Cards::Card> skat;
        std::vector<Cards::Card> trick;
        int max_rounds;
};

class AIGame : public Game<AIPlayer, AIPlayer, AIPlayer> {
    public:
        using Game<AIPlayer, AIPlayer, AIPlayer>::Game;
};


} // namespace HalfSkat