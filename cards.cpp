#include <algorithm>
#include <vector>
#include <random>

#include "cards.hpp"

using namespace Cards; 

std::vector<Card> Cards::get_full_shuffled_deck() {
    std::vector<Card> deck;
    for (const Color c : AllColors) {
        for (const Rank r : AllRanks) {
            deck.emplace_back(c, r);
        }
    }
    static auto rng = std::default_random_engine {};
    std::shuffle(deck.begin(), deck.end(), rng);
    return deck;
}

int Cards::get_card_points(std::vector<Card> const& cards) {
    int sum = 0;
    for (auto card: cards) {
        sum += rank_values.find(card.rank)->second;
    }
    return sum;
}

std::ostream& Cards::operator<< (std::ostream& os, Card const& card) {
    os << color_symbols.find(card.color)->second << rank_symbols.find(card.rank)->second;
    return os;
}

std::ostream& Cards::operator<< (std::ostream& os, std::vector<Card> const& cards) {
    int i = 0;
    for (auto card : cards) {
        os << std::to_string(i+1) << ". " << card;
        if (i++ < cards.size()-1) {
            os << ", ";
        }
    }
    return os;
}