#pragma once

#include <iostream>
#include <utility>
#include <array>
#include <vector>
#include <optional>
#include <algorithm>
#include <limits>
#include <random>
#include <unordered_map>

#define width 8
#define height 8

#define north 0
#define east 1
#define south 2
#define west 3

// #define MID_PAWN 10
// #define MID_SHAIKH 20

#define PAWN_VALUE 50
#define PAWN_ADVANCED_VALUE 70
#define SHAIKH_VALUE 100

constexpr std::array<int, 4> directions{north, east, south, west};
std::array<std::array<std::array<int, 4>, width>, height> data{};

#define INF 100000
#define NEG_INF -INF



struct Position{
    int x{}, y{};

    const static Position nullPosition;

    bool operator==(const Position& other) const = default;

    operator bool() const{
        return !operator==(nullPosition);
    }
};

const Position Position::nullPosition{-1, -1};


struct Piece{
    enum class Color{
        None = -1,
        Yellow,
        Black
    };



    Color otherColor() const{
        return color == Color::Black ? Color::Yellow : Color::Black;
    }

    Position position;
    Color color = Color::None;
    bool shaikh;

    constexpr int value() const{
        if(shaikh) return SHAIKH_VALUE;
        return PAWN_VALUE;
    }


    const static Piece nullPiece;
};

const Piece Piece::nullPiece = Piece{Position::nullPosition, Piece::Color::None, false};



struct Move{
    std::vector<Position> positions;
    std::vector<Piece> eaten;
    bool eat;

    int guess{};

    Move(Position from, Position to, bool eats = false)
    : positions{from, to}, eat{eats} {}

    Move(std::vector<Position> positions, bool eats = false)
    : positions{positions}, eat{eats} {}

    // bool operator==(const Move& other) const{
    //     if(positions.size() != other.positions.size()) return false;

    //     for(int i = 0; i < positions.size(); ++i){
    //         if(positions[i].x != other.positions[i].x || positions[i].y != other.positions[i].y){
    //             return false;
    //         }
    //     }

    //     return true;
    // }

    // operator bool() const{
    //     return !operator==(nullMove);
    // }

    const static Move nullMove;
};

const Move Move::nullMove = Move{{}};

