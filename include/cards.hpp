#pragma once

#include <array>
#include <map>
#include <iostream>
#include <string>

namespace Cards {

enum Color {Diamonds, Hearts, Spades, Clubs};
static const Color AllColors[] = {Diamonds, Hearts, Spades, Clubs};

enum Rank {Seven, Eight, Nine, Queen, King, Ten, Ace, Jack};
static const Rank AllRanks[] = {Seven, Eight, Nine, Queen, King, Ten, Ace, Jack};

static const std::map<Rank, int> rank_points = {{Jack, 2}, {Ace, 11}, {Ten, 10}, 
    {King, 4}, {Queen, 3}, {Nine, 0}, {Eight, 0}, {Seven, 0}};

static const std::map<Color, int> color_base_values = {{Clubs, 12}, {Spades, 11}, {Hearts, 10}, {Diamonds, 9}};

static const std::map<Color, std::string> color_symbols = {{Clubs, u8"♣"}, {Spades, u8"♠"}, {Hearts, u8"♥"}, {Diamonds, u8"♦"}}; 
static const std::map<Rank, std::string> rank_symbols = {{Jack, "J"}, {Ace, "A"}, {Ten, "T"}, {King, "K"}, 
    {Queen, "Q"}, {Nine, "9"}, {Eight, "8"}, {Seven, "7"}}; 

struct Card {
    Color color = Diamonds;
    Rank rank = Seven; 
    int played_by = -1;
    Card(Color const c, Rank const r) : color(c), rank(r) {};
    Card() {};
    Card(std::array<bool, 32> const onehot);
    std::array<bool, 32> to_one_hot() const;
    bool operator==(Card const& card) const { return ((color == card.color) && (rank == card.rank)); }
};

static const std::array<Card, 32> AllCards = {{{Clubs, Seven}, {Clubs, Eight}, {Clubs, Nine}, {Clubs, Ten}, {Clubs, Jack}, {Clubs, Queen}, {Clubs, King}, {Clubs, Ace}, 
{Spades, Seven}, {Spades, Eight}, {Spades, Nine}, {Spades, Ten}, {Spades, Jack}, {Spades, Queen}, {Spades, King}, {Spades, Ace}, 
{Hearts, Seven}, {Hearts, Eight}, {Hearts, Nine}, {Hearts, Ten}, {Hearts, Jack}, {Hearts, Queen}, {Hearts, King}, {Hearts, Ace}, 
{Diamonds, Seven}, {Diamonds, Eight}, {Diamonds, Nine}, {Diamonds, Ten}, {Diamonds, Jack}, {Diamonds, Queen}, {Diamonds, King}, {Diamonds, Ace}}};

std::ostream& operator<< (std::ostream& os, Card const& card);
std::ostream& operator<< (std::ostream& os, std::vector<Card> const& cards);

std::vector<Card> get_full_shuffled_deck();
int get_card_points(std::vector<Card> const& cards);
int get_suit_base_value(Card const& card);
int get_suit_base_value(Color const& color);
std::array<bool, 32> get_multi_hot(std::vector<Card> const& cards);

} // namespace Cards