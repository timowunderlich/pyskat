#include <algorithm>
#include <vector>
#include <random>
#include <chrono>
#include <stdexcept>

#include "cards.hpp"

using namespace Cards;

static auto rng = std::default_random_engine {};

std::vector<Card> Cards::get_full_shuffled_deck() {
    rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
    std::vector<Card> deck;
    for (const Color c : AllColors) {
        for (const Rank r : AllRanks) {
            deck.emplace_back(c, r);
        }
    }
    std::shuffle(deck.begin(), deck.end(), rng);
    return deck;
}

int Cards::get_card_points(std::vector<Card> const& cards) {
    int sum = 0;
    for (auto card: cards) {
        sum += rank_points.find(card.rank)->second;
    }
    return sum;
}

int Cards::get_suit_base_value(Card const& card) {
    return color_base_values.find(card.color)->second;
}

int Cards::get_suit_base_value(Color const& color) {
    return color_base_values.find(color)->second;
}

std::ostream& Cards::operator<< (std::ostream& os, Card const& card) {
    os << color_symbols.find(card.color)->second << rank_symbols.find(card.rank)->second;
    return os;
}

std::ostream& Cards::operator<< (std::ostream& os, std::vector<Card> const& cards) {
    size_t i = 0;
    for (auto card : cards) {
        os << std::to_string(i+1) << ". " << card;
        if (i++ < cards.size()-1) {
            os << ", ";
        }
    }
    return os;
}

Card::Card(std::array<bool, 32> const onehot) {
    int const cnt = std::count(onehot.begin(), onehot.end(), true);
    if ((cnt != 1) or (onehot.size() != 32)) {
        throw std::runtime_error("Must be one-hot array of length 32.");
    }
    int const idx = std::distance(onehot.begin(), std::find(onehot.begin(), onehot.end(), true));
    Card const c = AllCards.at(idx);
    color = c.color;
    rank = c.rank;
}

Card::Card(int const card_id) {
    assert((32 > card_id) and (card_id >= 0));
    Card const c = AllCards.at(card_id);
    color = c.color;
    rank = c.rank;
}

std::array<bool, 32> Cards::Card::to_one_hot() const {
    std::array<bool, 32> result;
    result.fill(false);
    int idx = std::distance(AllCards.begin(), std::find(AllCards.begin(), AllCards.end(), *this));
    result.at(idx) = true;
    return result;
}

// Returns array that indicates which cards are presented in the given vector of cards
std::array<bool, 32> Cards::get_multi_hot(std::vector<Card> const& cards) {
    std::array<bool, 32> result;
    result.fill(false);
    for (auto const c: cards) {
        int idx = std::distance(AllCards.begin(), std::find(AllCards.begin(), AllCards.end(), c));
        result.at(idx) = true;
    }
    return result;
}