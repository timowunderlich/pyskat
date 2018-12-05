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
        virtual Cards::Card query_policy() = 0 ;
    protected:
        std::vector<Cards::Card> m_cards;
        PlayerState m_last_state;
        Cards::Card m_last_action;
        std::vector<Transition> m_transitions;
};

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
        Game(int const max_rounds = 1000, bool const retry_on_illegal_action = false) : max_rounds(max_rounds), retry_on_illegal(retry_on_illegal_action) {
            BOOST_LOG_TRIVIAL(debug) << "Constructing new fully random Game.";
            players[0] = std::make_shared<RandomPlayer>();
            players[1] = std::make_shared<RandomPlayer>();
            players[2] = std::make_shared<RandomPlayer>();
            reset_cards();
            reset_players();
        }
        
        Game(std::shared_ptr<Player> first_player, int const max_rounds = 1000, bool retry_on_illegal_action = false) : max_rounds(max_rounds), retry_on_illegal(retry_on_illegal_action) {
            players[0] = std::move(first_player);
            players[1] = std::make_shared<RandomPlayer>();
            players[2] = std::make_shared<RandomPlayer>();
            reset_cards();
            reset_players();
        }

        ObservableState get_observable_state() const {
            return ObservableState(won_cards, trick, dealer, declarer);
        }

        std::vector<Cards::Card> get_legal_cards(std::vector<Cards::Card> const& players_cards) const {
            std::vector<Cards::Card> copy = players_cards;
            if (trick.empty()) { // No card played yet
                return copy;
            }
            Cards::Color sc = trick.front().color; // First card determines color to be played
            Cards::Rank sr = trick.front().rank;
            // First, determine cards that would follow suit
            std::vector<Cards::Card> legals = copy;
            if ((sr == Cards::Rank::Jack) or (sc == trump)) { // Trump is single suit
                legals.erase(std::remove_if(legals.begin(), legals.end(), [this](Cards::Card const& c) {
                    return ((c.rank != Cards::Rank::Jack) and (c.color != this->trump));
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

        bool trump_in_trick() const {
            for (auto const& c: trick) {
                if ((c.color == trump) or (c.rank == Cards::Rank::Jack)) {
                    return true;
                }
            }
            return false;
        }

        int get_trick_winner() {
            if (trick.size() != 3) {
                throw std::runtime_error("Trick is not full yet");
            }
            if (trick.size() > 3) {
                throw std::runtime_error("Trick is too full");
            }
            // First, update trick hierarchy
            for (auto& c: trick_hierarchy) {
                if (c.rank != Cards::Rank::Jack) {
                    if (trump_in_trick()) {
                        c.color = trump;
                    }
                    else {
                        c.color = trick.front().color;
                    }
                }
                // Find highest card in trick
                std::vector<Cards::Card>::iterator it;
                it = std::find(trick.begin(), trick.end(), c);
                if (it != trick.end()) {
                    int winner = it->played_by;
                    BOOST_LOG_TRIVIAL(debug) << "Determined winning card to be: " << c;
                    BOOST_LOG_TRIVIAL(debug) << "Determined winning player to be: " << std::to_string(winner);
                    return winner;
                }
            }

            throw std::runtime_error("Failed to find winning card");
        }

        bool declarer_has_won_round() const {
            int declarer_points;
            declarer_points = Cards::get_card_points(won_cards[declarer]);
            return (declarer_points >= 61);
        }

        int get_game_winner() const { 
            if (round <= max_rounds) { 
                BOOST_LOG_TRIVIAL(error) << "Game is not finished yet";
                return -1;
            }
            return std::distance(points.begin(), std::max_element(points.begin(), points.end()));
        }

        int get_game_value(std::vector<Cards::Card> const& cards) const {
            int base_value = Cards::get_suit_base_value(trump);
            return base_value * get_game_level(cards);
        }

        int get_game_level(std::vector<Cards::Card> const& cards) const {
            assert(cards.empty() == false);
            int game_level = 1;
            Cards::Card const clubs_jack{Cards::Color::Clubs, Cards::Rank::Jack};
            bool has_jack_clubs = (std::find(cards.begin(), cards.end(), clubs_jack) != cards.end());
            std::array<Cards::Card, 10> const cards_to_find {{{Cards::Color::Spades, Cards::Rank::Jack}, 
                {Cards::Color::Hearts, Cards::Rank::Jack},
                {Cards::Color::Diamonds, Cards::Rank::Jack},
                {trump, Cards::Rank::Ace},
                {trump, Cards::Rank::Ten},
                {trump, Cards::Rank::King},
                {trump, Cards::Rank::Queen},
                {trump, Cards::Rank::Nine},
                {trump, Cards::Rank::Eight},
                {trump, Cards::Rank::Seven}}};
            bool found;
            for (auto c: cards_to_find) {
                found = (std::find(cards.begin(), cards.end(), c) != cards.end());
                if ((found and has_jack_clubs) or (not found and not has_jack_clubs)) {
                    game_level++;
                }
                else {
                    return game_level; // Straight broken
                }
            }
            return game_level;
        }

        void step_by_trick() {
            BOOST_LOG_TRIVIAL(debug) << "====================================================================================";
            BOOST_LOG_TRIVIAL(debug) << "Performing game step in round " << std::to_string(round);
            // First, determine input for player
            bool current_player_is_declarer;
            if (current_player == declarer) {
                // No players are friendly
                current_player_is_declarer = true;
            }
            else {
                // Some players are friendly
                current_player_is_declarer = false;
            }
            BOOST_LOG_TRIVIAL(debug) << "Current round: " <<  round;
            BOOST_LOG_TRIVIAL(debug) << "Current trick: " <<  trick;
            BOOST_LOG_TRIVIAL(debug) << "Trick played by: ";
            for (auto c : trick) {
                BOOST_LOG_TRIVIAL(debug) << std::to_string(c.played_by) << " ";
            }
            BOOST_LOG_TRIVIAL(debug) << "Current player: " <<  std::to_string(current_player);
            BOOST_LOG_TRIVIAL(debug) << "Current dealer: " <<  std::to_string(dealer);
            BOOST_LOG_TRIVIAL(debug) << "Current declarer: " <<  std::to_string(declarer);
            BOOST_LOG_TRIVIAL(debug) << "Current player is declarer: " <<  current_player_is_declarer;
            BOOST_LOG_TRIVIAL(debug) << "First player hand: " << players[0]->m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Second player hand: " << players[1]->m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Third player hand: " << players[2]->m_cards;
            BOOST_LOG_TRIVIAL(debug) << "First player won: " << won_cards[0];
            BOOST_LOG_TRIVIAL(debug) << "Second player won: " << won_cards[1];
            BOOST_LOG_TRIVIAL(debug) << "Third player won: " << won_cards[2];
            // Get card player wants to play
            Cards::Card played_card;
            std::vector<Cards::Card> legal_cards;
            bool in_legals = false;
            state_before = get_observable_state();
            while (not in_legals) {
                played_card = players[current_player]->get_action(state_before, current_player);
                BOOST_LOG_TRIVIAL(debug) << "Player wants to play " << played_card;
                // Check if legal move
                legal_cards = get_legal_cards(players[current_player]->m_cards);
                in_legals = (std::find(legal_cards.begin(), legal_cards.end(), played_card) != legal_cards.end());
                BOOST_LOG_TRIVIAL(debug) << "This move is legal: " << in_legals;
                if (not retry_on_illegal) break;
            }
            remove_card_from_current_player(played_card);
            played_card.played_by = current_player;
            if (not in_legals) { // Abort game, illegal move
                state = early_abort;
                players[current_player]->put_transition(-1, get_observable_state(), current_player);
                BOOST_LOG_TRIVIAL(debug) << "Early game abort, resetting cards";
                reset_cards();
                return;
            }
            trick.push_back(played_card);

            if (trick.size() == 3) { // End of trick reached
                BOOST_LOG_TRIVIAL(debug) << "End of trick reached: " << trick;
                // Determine winner and manage cards
                int winner = get_trick_winner();
                won_cards[winner].insert(won_cards[winner].end(), trick.begin(), trick.end());
                trick.clear();
                tricks_played++;
                current_player = winner;
                state_after = get_observable_state();
                // Provide state transitions to players 
                if ((round != max_rounds) and (tricks_played != cards_per_player)) { // Only if game isn't over
                    for (size_t i=0; i<players.size(); i++) {
                        players[i]->put_transition(0, state_after, i);
                    }
                }
            } 
            else { // Trick moves on 
                BOOST_LOG_TRIVIAL(debug) << "It is the next player's turn";
                current_player = (current_player + 1) % 3;
            }
            BOOST_LOG_TRIVIAL(debug) << "====================================================================================";
            return;
        }

        void step_by_round() {
            int starting_round = round;
            while (round == starting_round) {
                step_by_trick();
                if (state == early_abort) {
                    return;
                }
                if (tricks_played == cards_per_player) { // Round finished
                    BOOST_LOG_TRIVIAL(debug) << "End of round reached";
                    // Declarer receives the Skat
                    won_cards[declarer].insert(won_cards[declarer].end(), skat.begin(), skat.end());
                    bool declarer_win = declarer_has_won_round();
                    BOOST_LOG_TRIVIAL(debug) << "Declarer has won: " << declarer_win;
                    int game_value = get_game_value(won_cards[declarer]);
                    BOOST_LOG_TRIVIAL(debug) << "Calculated game value: " << std::to_string(game_value);
                    if (declarer_win) {
                        points[declarer] += game_value;
                    }
                    else {
                        points[declarer] -= 2*game_value;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "New game points: " << std::to_string(points[0]) << ", " << std::to_string(points[1]) << ", " << std::to_string(points[2]);
                    round++;
                    // Reset cards and move player designations if game isn't finished 
                    if (round <= max_rounds) {
                        reset_cards();
                        // Set declarer, dealer to next player
                        declarer = (declarer + 1) % 3;
                        dealer = (dealer + 1) % 3;
                        current_player = (dealer + 1) % 3;
                        tricks_played = 0;
                    }
                }
            }
            return;
        }

        void run_new_game() {
            reset_cards();
            reset_points();
            reset_players();
            state = ongoing;
            game_winner = -1;
            round = 0;
            tricks_played = 0;
            step_by_game();
        }

        void step_by_game() { 
            while (state == ongoing) {
                step_by_round();
                if (state == early_abort) {
                    return;
                }
                if (round > max_rounds) {
                    game_winner = get_game_winner();
                    int not_winner = (game_winner + 1) % 3;
                    int also_not_winner = (game_winner + 2) % 3;
                    players[game_winner]->put_transition(+1, state_after, game_winner);
                    players[not_winner]->put_transition(-1, state_after, not_winner);
                    players[also_not_winner]->put_transition(-1, state_after, also_not_winner);
                    state = finished;
                    BOOST_LOG_TRIVIAL(debug) << "Game finished -- winner: " << std::to_string(game_winner);
                }
            }
            return;
        }

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
        void reset_points() {
            for (auto& p: points) {
                p = 0;
            }
        }
        void reset_players() {
            std::uniform_int_distribution<> distr(0, 2);
            declarer = distr(rng);
            dealer = distr(rng);
            current_player = (dealer+1) % 3;
        }
        void reset_cards() {
            trick.clear();
            for (auto& wc: won_cards) {
                wc.clear();
            }
            // Generate cards
            std::vector<Cards::Card> cards = Cards::get_full_shuffled_deck();
            assert(cards.size() == (cards_per_player*3 + cards_in_skat));
            players[0]->m_cards.resize(cards_per_player);
            players[1]->m_cards.resize(cards_per_player);
            players[2]->m_cards.resize(cards_per_player);
            skat.resize(cards_in_skat);
            std::move(cards.begin(), cards.begin() + cards_per_player, players[0]->m_cards.begin());
            std::move(cards.begin() + cards_per_player, cards.begin() + 2*cards_per_player, players[1]->m_cards.begin());
            std::move(cards.begin() + 2*cards_per_player, cards.begin() + 3*cards_per_player, players[2]->m_cards.begin());
            std::move(cards.begin() + 3*cards_per_player, cards.end(), skat.begin());
            BOOST_LOG_TRIVIAL(debug) << "First player: " << players[0]->m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Second player: " << players[1]->m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Third player: " << players[2]->m_cards;
            BOOST_LOG_TRIVIAL(debug) << "Skat: " << skat;
        }
        
        void remove_card_from_current_player(Cards::Card const& card) {
            for(auto it = players[current_player]->m_cards.begin(); it != players[current_player]->m_cards.end(); ++it) {
                if (*it == card) {
                    players[current_player]->m_cards.erase(it);
                    break;
                }
            }
        }
};

} // namespace HalfSkat
