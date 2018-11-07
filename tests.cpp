#include <gtest/gtest.h>
#include "cards.hpp"
#include "halfskat.hpp"


using namespace Cards;
using namespace HalfSkat;

TEST(CardsTest, AllCardsThere) {
    std::vector<Card> deck = get_full_shuffled_deck();
    ASSERT_EQ(32, deck.size());
    for (const Color c : AllColors) {
        for (const Rank r : AllRanks) {
            bool found = false;
            for (auto card : deck) {
                if (card.color == c && card.rank == r) found = true;
            }
            ASSERT_EQ(found, true);
        }
    }
};

TEST(CardsTest, CardPointsCorrect) {
    std::vector<Card> cards = {{Clubs, Jack}, {Hearts, Ten}, {Diamonds, Nine}, {Diamonds, Ten}, {Spades, King}};
    int expected_points_sum = 2 + 10 + 0 + 10 + 4;
    int points = get_card_points(cards);
    ASSERT_EQ(points, expected_points_sum);
};

TEST(CardsTest, CardPrintCorrect) {
    std::vector<Card> cards = {{Clubs, Jack}, {Hearts, Ten}, {Diamonds, Nine}, {Diamonds, Ten}, {Spades, King}};
    std::string expected_str = "1. ♣J, 2. ♥T, 3. ♦9, 4. ♦T, 5. ♠K";
    std::stringstream os; 
    os << cards;
    ASSERT_EQ(expected_str, os.str());
}

TEST(HalfSkatTest, AIGameLegalActions) {
    AIGame game;
    std::vector<Card> cards = {{Clubs, Jack}, {Hearts, Ten}, {Diamonds, Nine}, {Clubs, Ten}, {Spades, King}};
    std::vector<Card> legals = game.get_legal_cards(cards);
    // All cards should be legal
    for (auto l : legals) {
        bool found = false;
        for (auto c : cards) {
            if (c == l) {
                found = true;
            }
        }
        ASSERT_EQ(found, true);
    }
    /*game.trick.emplace_back(game.trump, Ten);
    legals = game.get_legal_cards(cards);  
    for (auto l : legals) {
        bool is_trump = (l.color == game.trump) || (l.rank == Jack);
        ASSERT_EQ(is_trump, true);
    }*/
}

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}