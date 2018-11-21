#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <Python.h>

#include "halfskat.hpp"
#include "cards.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pyskat, m) {
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
    m.def("get_full_shuffled_deck", Cards::get_full_shuffled_deck);
}