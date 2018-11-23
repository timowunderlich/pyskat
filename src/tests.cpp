#include <gtest/gtest.h>
#include <cmath>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include "cards.hpp"
#include "halfskat.hpp"

namespace logging = boost::log;
using namespace Cards;
using namespace HalfSkat;

TEST(CardsTest, AllCardsThere) {
    std::vector<Card> deck = get_full_shuffled_deck();
    ASSERT_EQ(32, deck.size());
    for (const Color c : AllColors) {
        for (const Rank r : AllRanks) {
            bool found = false;
            for (auto card : deck) {
                if (card.color == c && card.rank == r) {
                    found = true;
                    break;
                }
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

TEST(CardsTest, ColorRankingCorrect) {
    ASSERT_LT(Spades, Clubs);
    ASSERT_LT(Hearts, Spades);
    ASSERT_LT(Diamonds, Hearts);
}

TEST(CardsTest, RankRankingCorrect) {
    ASSERT_LT(Ten, Ace);
    ASSERT_LT(King, Ten);
    ASSERT_LT(Queen, King);
    ASSERT_LT(Nine, Queen);
    ASSERT_LT(Eight, Nine);
    ASSERT_LT(Seven, Eight);
}

TEST(HalfSkatTest, RandomGameLegalActions) {
    Game game;
    std::vector<Card> cards = {{Clubs, Jack}, {Hearts, Ten}, {Diamonds, Nine}, {Clubs, Ten}, {Spades, King}};
    std::vector<Card> legals = game.get_legal_cards(cards);
    // All cards should be legal
    for (auto l : legals) {
        bool found = false;
        for (auto c : cards) {
            if (c == l) {
                found = true;
                break;
            }
        }
        ASSERT_EQ(found, true);
    }
    game.step_by_trick();
    legals = game.get_legal_cards(cards);  
    Card trick_card = game.get_trick()[0];
    bool need_trump;
    if ((trick_card.rank == Jack) or (trick_card.color == game.trump)) {
        need_trump = true;
    }
    else {
        need_trump = false;
    }
    bool is_correct = false;
    for (auto l : legals) {
        if (need_trump) {
            is_correct = ((l.rank == Jack) or (l.color == game.trump));
        }
        else {
            is_correct = (trick_card.color == l.color);
        }
        ASSERT_EQ(is_correct, true);
    }
}

TEST(HalfSkatTest, RandomGameTricksWork) {
    Game game;
    int initial_round = game.get_round();
    game.step_by_trick();
    ASSERT_EQ(game.get_round(), initial_round);
    ASSERT_EQ(game.get_trick().size(), 1);
    game.step_by_trick();
    ASSERT_EQ(game.get_round(), initial_round);
    ASSERT_EQ(game.get_trick().size(), 2);
    game.step_by_trick(); // Should be final step
    ASSERT_EQ(game.get_round(), initial_round);
    ASSERT_EQ(game.get_trick().size(), 0);
}

TEST(HalfSkatTest, RandomGameWholeGame) { 
    Game game;
    int winner = game.run_whole_game();
    ASSERT_GE(winner, 0);
    ASSERT_EQ(game.get_round(), game.get_max_rounds()+1);
}

TEST(HalfSkatTest, RandomNoPlayerBias) {
    int winner;
    constexpr int n = 1e2;
    std::array<int, 3> games_won = {{-n/3, -n/3, -n/3}};
    int const expected_var = std::round(n * (2/9.));
    int const expected_sigma = std::sqrt(expected_var);
    for (int i=0; i<n; i++) {
        Game game;
        winner = game.run_whole_game();
        games_won[winner]++;
    }
    EXPECT_LE(games_won[0], 2*expected_sigma);
    EXPECT_LE(games_won[1], 2*expected_sigma);
    EXPECT_LE(games_won[2], 2*expected_sigma);
    EXPECT_GE(games_won[0], -2*expected_sigma);
    EXPECT_GE(games_won[1], -2*expected_sigma);
    EXPECT_GE(games_won[2], -2*expected_sigma);
}

GTEST_API_ int main(int argc, char **argv) {
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}