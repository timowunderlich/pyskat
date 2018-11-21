#include "halfskat.hpp"
#include "cards.hpp"


using namespace HalfSkat;

Cards::Card RandomPlayer::get_action(std::array<std::vector<Cards::Card>, 3> const& won_cards, std::vector<Cards::Card> const& trick, std::vector<bool> const& played_by_declarer, bool is_declarer) {
    assert(m_cards.empty() == false);
    std::uniform_int_distribution<> distr(0, m_cards.size()-1);
    int card = distr(rng);
    return m_cards[card];
}

