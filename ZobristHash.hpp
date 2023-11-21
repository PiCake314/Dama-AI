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

#include "Piece.hpp"

class ZobristHash{
    // there are 8x8 squares, 2 turns, 2 colors, 2 types of pieces
    std::array<std::array<std::array<std::array<uint64_t, 4>, 2>, 8>, 8> table;


    int indexOf(const Piece& p){
        int index = 0;
        if(p.shaikh) index += 1;
        if(p.color == Piece::Color::Black) index += 2;

        return index;
    }
public:

    ZobristHash(){
        std::uniform_int_distribution<uint> dist{0, static_cast<std::uniform_int_distribution<unsigned int>::result_type>(UINT64_MAX)};
        std::mt19937 mt(01234567);

        for(int i = 0; i < 8; ++i){
            for(int j = 0; j < 8; ++j){
                for(int l = 0; l < 2; ++l){
                    for(int k = 0; k < 4; ++k){
                        table[i][j][l][k] = dist(mt);
                    }
                }
            }
        }
    }

    uint64_t hash(const std::array<std::array<Piece, BOARD_WIDTH>, BOARD_HEIGHT>& board, Piece::Color turn_color){
        uint64_t hash = 0;

        const int turn = static_cast<int>(turn_color);
        for(const auto& row : board){
            for(const auto& p : row){
                if(p.color != Piece::Color::None){
                    hash ^= table[p.position.y][p.position.x][turn][indexOf(p)];
                }
            }
        }

        return hash;
    }
};
