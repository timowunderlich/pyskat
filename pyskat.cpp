#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <Python.h>

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
        .def_readwrite("rank", &Cards::Card::rank);
    m.def("get_full_shuffled_deck", &Cards::get_full_shuffled_deck);
    m.def("get_card_points", &Cards::get_card_points);
    m.def("get_suit_base_value", (int (*)(Cards::Card const&)) &Cards::get_suit_base_value);
    m.def("get_suit_base_value", (int (*)(Cards::Color const&)) &Cards::get_suit_base_value);

    // HalfSkat bindings
    py::class_<HalfSkat::Player, std::shared_ptr<HalfSkat::Player>>(m, "Player")
        .def(py::init<>())
        .def("get_action", &HalfSkat::Player::get_action);
    py::class_<HalfSkat::RandomPlayer, std::shared_ptr<HalfSkat::RandomPlayer>>(m, "RandomPlayer")
        .def(py::init<>())
        .def("get_action", &HalfSkat::Player::get_action);
    py::class_<HalfSkat::HumanPlayer, std::shared_ptr<HalfSkat::HumanPlayer>>(m, "HumanPlayer")
        .def(py::init<>())
        .def("get_action", &HalfSkat::Player::get_action);
    py::class_<HalfSkat::Game>(m, "Game")
        .def(py::init<int const>(), py::arg("max_rounds") = 1000)
        .def(py::init<std::shared_ptr<HalfSkat::Player>, int const>(), py::arg("first_player"), py::arg("max_rounds") = 1000)
        .def(py::init<std::shared_ptr<HalfSkat::RandomPlayer>, int const>(), py::arg("first_player"), py::arg("max_rounds") = 1000)
        .def(py::init<std::shared_ptr<HalfSkat::HumanPlayer>, int const>(), py::arg("first_player"), py::arg("max_rounds") = 1000)
        .def("step_by_trick", &HalfSkat::Game::step_by_trick)
        .def("step_by_round", &HalfSkat::Game::step_by_round)
        .def("run_whole_game", &HalfSkat::Game::run_whole_game)
        .def("get_trick", &HalfSkat::Game::get_trick)
        .def("get_points", &HalfSkat::Game::get_points)
        .def("get_round", &HalfSkat::Game::get_round)
        .def("get_max_rounds", &HalfSkat::Game::get_max_rounds)
        .def_readonly("trump", &HalfSkat::Game::trump);
}