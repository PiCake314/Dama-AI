#include <iostream>
#include <utility>
#include <array>
#include <vector>
#include <optional>
#include <algorithm>
#include <limits>
#include <random>
#include <chrono>
#include <thread>
#include <unordered_map>

using namespace std::chrono_literals;

#include "Piece.hpp"
#include "ZobristHash.hpp"
#include "FixedSizeHashTable.hpp"
#include "Game.hpp"

#include <atomic>

std::atomic_bool stop{false};

void timer(std::chrono::milliseconds ms){
    std::this_thread::sleep_for(ms);
    stop = true;
}


int main(){
    assert(std::numeric_limits<double>::is_iec559);
    std::srand(std::time(nullptr));

    // endgame: 7Y/8/1B6/8/1B6/8/1B6/7Y

    Game b{};
    b.turn = Piece::Color::Black;

    b.print();

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

            // if(moves.size() != 1)
            // for(const auto& m : moves){
            //     bool promotion = b.makeMove(m);
            //     int score = -b.alphaBeta(6);
            //     b.unmakeMove(m, promotion);

            //     if(score >= best){ // using >= instead of > to avoid the first move being the same every time
            //         best = score;
            //         bestMove = m;
            //     }
            // }

            if(moves.size() != 1){
                const int depth_limit = 100;
                stop = false;

                std::thread t{timer, std::chrono::milliseconds{2000ms}};

                t.detach();
                int d = 1;
                for(int depth = 1; depth <= depth_limit; ++depth, ++d){ // iterative deepening

                    if(b.best_moves.size())
                    moves.insert(moves.begin(), b.best_moves.front());

                    for(const auto& m : moves){
                        bool promotion = b.makeMove(m);
                        int score = -b.alphaBeta(depth);
                        b.unmakeMove(m, promotion);

                        if(score >= best){ // using >= instead of > to avoid the first move being the same every time
                            best = score;
                            bestMove = m;
                        }

                        if(stop) break;
                    }

                    if(stop) break;

                    b.best_moves.push_back(bestMove);
                }
                b.best_moves.clear();
                
                std::cout << "reached depth: " << d << '\n';
            }


            std::cout << "Yellow's best: " << best << '\n';
            for(int i = 0; i < bestMove.positions.size() - 1; ++i){
                std::cout << bestMove.positions[i].x << ' ' << bestMove.positions[i].y << " -> ";
            }
            std::cout << bestMove.positions.back().x << ' ' << bestMove.positions.back().y << '\n';

            b.play(bestMove);
        }
        else{

            std::clog << moves.size() << " moves:\n";
            // int jj = 0;
            // for(const auto& m : moves){
            //     std::cout << '[' << jj++ << "]: ";
            //     for(int i = 0; i < m.positions.size() - 1; ++i){
            //             std::cout << m.positions[i].x << ' ' << m.positions[i].y << " -> ";
            //     }
            //     std::cout << m.positions.back().x << ' ' << m.positions.back().y << '\n';

            //     std::puts("Eats:");
            //     for(const auto& e : m.eaten){
            //         std::cout << e.position.x << ' ' << e.position.y << '\n';
            //     }
            // }
            // jj = 0;

            // int best = -INF;
            // Move bestMove = moves.front();
            
        //     if(moves.size() != 1)
        //     for(const auto& m : moves){
        //         bool promotion = b.makeMove(m);
        //         int score = -b.alphaBeta(4);
        //         b.unmakeMove(m, promotion);

        //         if(score > best){
        //             best = score;
        //             std::puts("New best:");
        //             bestMove = m;
        //         }
        //     }

        //     std::cout << "Black's best: " << best << '\n';
        //     for(int i = 0; i < bestMove.positions.size() - 1; ++i){
        //         std::cout << bestMove.positions[i].x << ' ' << bestMove.positions[i].y << " -> ";
        //     }
        //     std::cout << bestMove.positions.back().x << ' ' << bestMove.positions.back().y << '\n';

            Move bestMove = moves.at(rand() % moves.size());

            b.play(bestMove);
        }


        std::puts("Done!");
        // b.play(moves[0]);
        b.print();
        std::cout << b.cache.size() << " entries in the cache out of " << b.cache.capacity() << '\n';
        std::getchar();

    }

}