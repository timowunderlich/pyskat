#pragma once

#include <array>
#include <map>
#include <iostream>
#include <string>

namespace Cards {

enum Color {Clubs, Spades, Hearts, Diamonds};
static const Color AllColors[] = {Clubs, Spades, Hearts, Diamonds};

enum Rank {Jack, Ace, Ten, King, Queen, Nine, Eight, Seven};
static const Rank AllRanks[] = {Jack, Ace, Ten, King, Queen, Nine, Eight, Seven};

static const std::map<Rank, int> rank_values = {{Jack, 2}, {Ace, 11}, {Ten, 10}, 
    {King, 4}, {Queen, 3}, {Nine, 0}, {Eight, 0}, {Seven, 0}};

static const std::map<Color, std::string> color_symbols = {{Clubs, u8"♣"}, {Spades, u8"♠"}, {Hearts, u8"♥"}, {Diamonds, u8"♦"}}; 
static const std::map<Rank, std::string> rank_symbols = {{Jack, "J"}, {Ace, "A"}, {Ten, "T"}, {King, "K"}, 
    {Queen, "Q"}, {Nine, "9"}, {Eight, "8"}, {Seven, "7"}}; 

struct Card {
    Color color;
    Rank rank; 
    Card(Color c, Rank r) : color(c), rank(r) {};
    Card() {};
    bool operator==(Card const& card) const { return ((color == card.color) && (rank == card.rank)); }
};

std::ostream& operator<< (std::ostream& os, Card const& card);
std::ostream& operator<< (std::ostream& os, std::vector<Card> const& cards);

std::vector<Card> get_full_shuffled_deck();
int get_card_points(std::vector<Card> const& cards);

} // namespace Cards