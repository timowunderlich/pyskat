//#pragma once

#include <memory>
#include <vector>
#include <pybind11/pybind11.h>
#include <boost/log/trivial.hpp>
#include <cassert>
#include <algorithm>
#include <random>
#include <chrono>
#include <stdexcept>

#include "cards.hpp"

namespace HalfSkat {

static auto rng = std::default_random_engine {};
static const int cards_per_player = 10;
static const int cards_in_skat = 2;
enum GameState { ongoing = 0, early_abort = -1, finished = 1 };

// Player independent state of game which everyone can observe
struct ObservableState { 
    std::array<std::vector<Cards::Card>, 3> won_cards; // Cards won previously by players
    std::vector<Cards::Card> trick; // Current trick
    int dealer; // Identifies current dealer
    int declarer; // Identifies current declarer
    ObservableState(std::array<std::vector<Cards::Card>, 3> const& won_cards, std::vector<Cards::Card> const& trick, int const dealer, int const declarer) : won_cards(won_cards), trick(trick), dealer(dealer), declarer(declarer) { }
    ObservableState() {}
};

// State of game from the perspective of a player
struct PlayerState {
    std::vector<Cards::Card> hole_cards;
    std::vector<Cards::Card> trick;
    std::vector<Cards::Card> trick_friendly; // Part of trick that was played by me or friendly player
    std::vector<Cards::Card> trick_hostile; // Part of trick that was played by hostile players
    std::vector<Cards::Card> won_friendly; // Cards won by me or the friendly party
    std::vector<Cards::Card> won_hostile; // Cards won by hostile players
    bool is_declarer; // Indicates whether player is the declarer
    PlayerState() = default;
    // Construct PlayerState from ObservableState, hole cards and player identifier
    PlayerState(ObservableState const& public_state, std::vector<Cards::Card> const& hole_cards, int player_id) : hole_cards(hole_cards) {
        is_declarer = (public_state.declarer == player_id);
        trick = public_state.trick;
        if (is_declarer) {
            int other_id = (player_id+1)%3;
            int also_other_id = (player_id+2)%3;
            won_hostile.insert(won_hostile.end(), public_state.won_cards[other_id].begin(), public_state.won_cards[other_id].end());
            won_hostile.insert(won_hostile.end(), public_state.won_cards[also_other_id].begin(), public_state.won_cards[also_other_id].end());
            won_friendly.insert(won_friendly.end(), public_state.won_cards[player_id].begin(), public_state.won_cards[player_id].end());
        }
        else {
            int other_id = (public_state.declarer+1)%3;
            int also_other_id = (public_state.declarer+2)%3;
            won_hostile.insert(won_hostile.end(), public_state.won_cards[public_state.declarer].begin(), public_state.won_cards[public_state.declarer].end());
            won_friendly.insert(won_friendly.end(), public_state.won_cards[other_id].begin(), public_state.won_cards[other_id].end());
            won_friendly.insert(won_friendly.end(), public_state.won_cards[also_other_id].begin(), public_state.won_cards[also_other_id].end());
        }
        // Find player cards in trick
        for (auto c : public_state.trick) {
            if (is_declarer) {
                if (c.played_by == player_id) {
                    trick_friendly.push_back(c);
                }
                else {
                    trick_hostile.push_back(c);
                }
            }
            else {
                if (c.played_by == public_state.declarer) {
                    trick_hostile.push_back(c);
                }
                else {
                    trick_friendly.push_back(c);
                }
            }
        }
    }
};

struct Transition {
    PlayerState before;
    PlayerState after;
    int reward;
    Cards::Card action;
    Transition(PlayerState const& before, PlayerState const& after, int reward, Cards::Card const& action) : before(before), after(after), reward(reward), action(action) {}
};

class Player {
    friend class Game;
    public:
        Player() = default;
        virtual ~Player() = default;
        Cards::Card get_action(ObservableState const& state, int player_id);
        void put_transition(int const reward, ObservableState const& new_state, int player_id);
        std::vector<Cards::Card> get_cards() { return m_cards; }
        PlayerState get_last_state() { return m_last_state; }
        Cards::Card get_last_action() { return m_last_action; }
        std::vector<Transition> get_transitions() { return m_transitions; }
        void clear_transitions() { m_transitions.clear(); }
        virtual Cards::Card query_policy() = 0 ;
    protected:
        std::vector<Cards::Card> m_cards;
        PlayerState m_last_state;
        Cards::Card m_last_action;
        std::vector<Transition> m_transitions;
};

// Trampoline class to enable overriding from Python
class PyPlayer : public Player {
    public:
        using Player::Player;
        Cards::Card query_policy() override {
            PYBIND11_OVERLOAD_PURE(
                Cards::Card,
                Player,
                query_policy
            );
        }
};

class RandomPlayer : public Player {
    public:
        RandomPlayer() { rng.seed(std::chrono::system_clock::now().time_since_epoch().count()); } // Seed with current time
        Cards::Card query_policy() override;
};

class HumanPlayer : public Player {
    public:
        using Player::Player;
        Cards::Card query_policy() override;
};

class Game {
    public:
        Game(int const max_rounds = 1000, bool const retry_on_illegal_action = false);
        Game(std::shared_ptr<Player> first_player, std::shared_ptr<Player> second_player, std::shared_ptr<Player> third_player, int const max_rounds = 1000, bool retry_on_illegal_action = false);

        ObservableState get_observable_state() const;
        std::vector<Cards::Card> get_legal_cards(std::vector<Cards::Card> const& players_cards) const;
        bool trump_in_trick() const;
        int get_trick_winner();
        bool declarer_has_won_round() const;
        int get_game_winner() const;
        int get_game_value(std::vector<Cards::Card> const& cards) const;
        int get_game_level(std::vector<Cards::Card> const& cards) const;
        void step_by_trick();
        void step_by_round();
        void run_new_game();
        void step_by_game();

        std::vector<Cards::Card> get_trick() const { return trick; }
        std::array<int, 3> get_points() const { return points; }
        int get_round() const { return round; }
        int get_max_rounds() const { return max_rounds; }
        GameState get_state() const { return state; }
        Cards::Color const trump = Cards::Color::Clubs;
    protected:
        GameState state = ongoing;
        int max_rounds;
        bool retry_on_illegal;
        int tricks_played = 0;
        int game_winner = -1;
        int round = 0;
        int dealer = 0;
        int declarer = 0;
        int current_player = 1;
        ObservableState state_before;
        ObservableState state_after;
        std::array<std::shared_ptr<Player>, 3> players;
        std::array<std::vector<Cards::Card>, 3> won_cards;
        std::array<int, 3> points = {{0, 0, 0}};
        std::array<Cards::Card, 11> trick_hierarchy {{{Cards::Color::Clubs, Cards::Rank::Jack}, {Cards::Color::Spades, Cards::Rank::Jack},
            {Cards::Color::Hearts, Cards::Rank::Jack}, {Cards::Color::Diamonds, Cards::Rank::Jack}, {Cards::Color::Spades, Cards::Rank::Ace},
            {Cards::Color::Spades, Cards::Rank::Ten}, {Cards::Color::Spades, Cards::Rank::King}, {Cards::Color::Spades, Cards::Rank::Queen},
            {Cards::Color::Spades, Cards::Rank::Nine}, {Cards::Color::Spades, Cards::Rank::Eight}, {Cards::Color::Spades, Cards::Rank::Seven}}};
        std::vector<Cards::Card> skat;
        std::vector<Cards::Card> trick;
        std::uniform_int_distribution<> rand_distr{0, 2};
        void reset_points();
        void reset_players();
        void reset_cards();
        void remove_card_from_current_player(Cards::Card const& card);
};

} // namespace HalfSkat
