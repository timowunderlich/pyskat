#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <Python.h>
#include <sstream>

#include "halfskat.hpp"
#include "cards.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pyskat, m) {
    // Cards bindings
    py::enum_<Cards::Color>(m, "Color")
        .value("Diamonds", Cards::Color::Diamonds)
        .value("Hearts", Cards::Color::Hearts)
        .value("Spades", Cards::Color::Spades)
        .value("Clubs", Cards::Color::Clubs);
    py::enum_<Cards::Rank>(m, "Rank", py::arithmetic())
        .value("Seven", Cards::Rank::Seven)
        .value("Eight", Cards::Rank::Eight)
        .value("Nine", Cards::Rank::Nine)
        .value("Queen", Cards::Rank::Queen)
        .value("King", Cards::Rank::King)
        .value("Ten", Cards::Rank::Ten)
        .value("Ace", Cards::Rank::Ace)
        .value("Jack", Cards::Rank::Jack);
    py::class_<Cards::Card>(m, "Card")
        .def(py::init<>())
        .def(py::init<Cards::Color const, Cards::Rank const>())
        .def(py::init<std::array<bool, 32> const>())
        .def(py::init<int const>())
        .def(py::self == py::self)
        .def_readwrite("color", &Cards::Card::color)
        .def("__str__", [](Cards::Card const& c) { 
            std::stringstream ss;
            ss << c; 
            return ss.str(); })
        .def("__repr__", [](Cards::Card const& c) { 
            std::stringstream ss;
            ss << "<Card: "  << c << ">"; 
            return ss.str(); })
        .def_readwrite("rank", &Cards::Card::rank)
        .def_readwrite("played_by", &Cards::Card::played_by)
        .def("to_one_hot", &Cards::Card::to_one_hot);
    m.def("get_full_shuffled_deck", &Cards::get_full_shuffled_deck);
    m.def("get_card_points", &Cards::get_card_points);
    m.def("get_suit_base_value", (int (*)(Cards::Card const&)) &Cards::get_suit_base_value);
    m.def("get_suit_base_value", (int (*)(Cards::Color const&)) &Cards::get_suit_base_value);
    m.def("get_multi_hot", &Cards::get_multi_hot);

    // HalfSkat bindings
    py::class_<HalfSkat::Player, std::shared_ptr<HalfSkat::Player>, HalfSkat::PyPlayer>(m, "Player")
        .def(py::init<>())
        .def("query_policy", &HalfSkat::Player::query_policy)
        .def("get_cards", &HalfSkat::Player::get_cards)
        .def("get_last_state", &HalfSkat::Player::get_last_state)
        .def("get_last_action", &HalfSkat::Player::get_last_action)
        .def("get_transitions", &HalfSkat::Player::get_transitions);
    py::class_<HalfSkat::RandomPlayer, std::shared_ptr<HalfSkat::RandomPlayer>>(m, "RandomPlayer")
        .def(py::init<>())
        .def("query_policy", &HalfSkat::Player::query_policy)
        .def("get_cards", &HalfSkat::Player::get_cards)
        .def("get_last_state", &HalfSkat::Player::get_last_state)
        .def("get_last_action", &HalfSkat::Player::get_last_action)
        .def("get_transitions", &HalfSkat::Player::get_transitions);
    py::class_<HalfSkat::HumanPlayer, std::shared_ptr<HalfSkat::HumanPlayer>>(m, "HumanPlayer")
        .def(py::init<>());
    py::class_<HalfSkat::Game>(m, "Game")
        .def(py::init<int const, bool const>(), py::arg("max_rounds") = 1000, py::arg("retry_on_illegal_action") = false)
        .def(py::init<std::shared_ptr<HalfSkat::Player>, std::shared_ptr<HalfSkat::Player>, std::shared_ptr<HalfSkat::Player>, int const, bool const>(), py::arg("first_player"), py::arg("second_player"), py::arg("third_player"), py::arg("max_rounds") = 1000, py::arg("retry_on_illegal_action") = false)
        .def("step_by_trick", &HalfSkat::Game::step_by_trick)
        .def("step_by_round", &HalfSkat::Game::step_by_round)
        .def("step_by_game", &HalfSkat::Game::step_by_game)
        .def("run_new_game", &HalfSkat::Game::run_new_game)
        .def("get_trick", &HalfSkat::Game::get_trick)
        .def("get_points", &HalfSkat::Game::get_points)
        .def("get_round", &HalfSkat::Game::get_round)
        .def("get_max_rounds", &HalfSkat::Game::get_max_rounds)
        .def_readonly("trump", &HalfSkat::Game::trump);
    py::class_<HalfSkat::Transition>(m, "Transition")
        .def_readonly("before", &HalfSkat::Transition::before)
        .def_readonly("after", &HalfSkat::Transition::after)
        .def_readonly("reward", &HalfSkat::Transition::reward)
        .def_readonly("action", &HalfSkat::Transition::action);
    py::class_<HalfSkat::PlayerState>(m, "PlayerState")
        .def_readonly("hole_cards", &HalfSkat::PlayerState::hole_cards)
        .def_readonly("trick", &HalfSkat::PlayerState::trick)
        .def_readonly("trick_friendly", &HalfSkat::PlayerState::trick_friendly)
        .def_readonly("trick_hostile", &HalfSkat::PlayerState::trick_hostile)
        .def_readonly("won_friendly", &HalfSkat::PlayerState::won_friendly)
        .def_readonly("won_hostile", &HalfSkat::PlayerState::won_hostile)
        .def_readonly("is_declarer", &HalfSkat::PlayerState::is_declarer);
}