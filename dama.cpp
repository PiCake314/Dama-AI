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
#include "ZobristHash.hpp"
#include "FixedSizeHashTable.hpp"
#include "Game.hpp"



int main(){
    assert(std::numeric_limits<double>::is_iec559);
    std::srand(std::time(nullptr));

    // endgame: 7Y/8/1B6/8/1B6/8/1B6/7Y

    Game b{};
    b.turn = Piece::Color::Black;

    b.print();

    data = precomputeData();

    for(; b.hasntEnded(); ){

        auto moves = b.generateMoves();

        std::cout << "Black pieces: " << b.numBlack << ", " << b.numShaikhBlack << " of which are shaikhs\n";
        std::cout << "Yellow pieces: " << b.numYellow << ", " << b.numShaikhYellow << " of which are shaikhs\n";

        if(b.turn == Piece::Color::Yellow){

            // std::clog << moves.size() << " moves:\n";
            // for(const auto& m : moves){
            //     for(int i = 0; i < m.positions.size() - 1; ++i){
            //             std::cout << m.positions[i].x << ' ' << m.positions[i].y << " -> ";
            //     }
            //     std::cout << m.positions.back().x << ' ' << m.positions.back().y << '\n';

            //     std::puts("Eats:");
            //     for(const auto& e : m.eaten){
            //         std::cout << e.position.x << ' ' << e.position.y << '\n';
            //     }
            // }

            int best = -INF;
            Move bestMove = moves.front();

            if(moves.size() != 1)
            for(const auto& m : moves){
                bool promotion = b.makeMove(m);
                int score = -b.alphaBeta(6);
                b.unmakeMove(m, promotion);

                if(score >= best){ // using >= instead of > to avoid the first move being the same every time
                    best = score;
                    bestMove = m;
                }
            }

            std::cout << "Black's best: " << best << '\n';
            for(int i = 0; i < bestMove.positions.size() - 1; ++i){
                std::cout << bestMove.positions[i].x << ' ' << bestMove.positions[i].y << " -> ";
            }
            std::cout << bestMove.positions.back().x << ' ' << bestMove.positions.back().y << '\n';

            b.play(bestMove);
        }
        else{

            std::clog << moves.size() << " moves:\n";
            int jj = 0;
            for(const auto& m : moves){
                std::cout << '[' << jj++ << "]: ";
                for(int i = 0; i < m.positions.size() - 1; ++i){
                        std::cout << m.positions[i].x << ' ' << m.positions[i].y << " -> ";
                }
                std::cout << m.positions.back().x << ' ' << m.positions.back().y << '\n';

                std::puts("Eats:");
                for(const auto& e : m.eaten){
                    std::cout << e.position.x << ' ' << e.position.y << '\n';
                }
            }
            jj = 0;

            // int best = -INF;
            // Move bestMove = moves.front();
            
            // if(moves.size() != 1)
            // for(const auto& m : moves){
            //     bool promotion = b.makeMove(m);
            //     int score = -b.alphaBeta(4);
            //     b.unmakeMove(m, promotion);

            //     if(score > best){
            //         best = score;
            //         bestMove = m;
            //     }
            // }

            // std::cout << "Yellow's best: " << best << '\n';
            // for(int i = 0; i < bestMove.positions.size() - 1; ++i){
            //     std::cout << bestMove.positions[i].x << ' ' << bestMove.positions[i].y << " -> ";
            // }
            // std::cout << bestMove.positions.back().x << ' ' << bestMove.positions.back().y << '\n';

            int index = std::rand() % moves.size();
            std::clog << "Plating move: " << index << '\n';
            Move best_move = moves.at(index);

            b.play(best_move);
        }


        std::puts("Done!");
        // b.play(moves[0]);
        b.print();
        std::cout << b.cache.size() << " entries in the cache out of " << b.cache.capacity() << '\n';
        // std::getchar();

    }

}