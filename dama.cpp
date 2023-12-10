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

    const int num_games = 100;
    int AI_wins = 0;
    int RANDOM_wins = 0;

    for(int game_num = 0; game_num < num_games; ++game_num){

        std::cout << "Game " << game_num << '\n';


        Game b{};
        b.turn = Piece::Color::Black;

        data = precomputeData();

        for(; b.hasntEnded(); ){

            auto moves = b.generateMoves();

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

                double best = -INF;
                Move bestMove = moves.front();

                if(moves.size() != 1)
                for(const auto& m : moves){
                    bool promotion = b.makeMove(m);
                    double score = -b.alphaBeta(1);
                    b.unmakeMove(m, promotion);

                    if(score >= best){ // using >= instead of > to avoid the first move being the same every time
                        best = score;
                        bestMove = m;
                    }
                }

                // std::cout << "Black's best: " << best << '\n';
                // for(int i = 0; i < bestMove.positions.size() - 1; ++i){
                //     std::cout << bestMove.positions[i].x << ' ' << bestMove.positions[i].y << " -> ";
                // }
                // std::cout << bestMove.positions.back().x << ' ' << bestMove.positions.back().y << '\n';

                b.play(bestMove);
            }
            else{

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
                Move best_move = moves.at(index);

                b.play(best_move);
            }


            // std::puts("Done!");
            // b.play(moves[0]);
            // b.print(); puts("====================================");
            // std::cout << b.cache.size() << " entries in the cache out of " << b.cache.capacity() << '\n';
            // std::getchar();

        }

        if(b.getNumPieces(Piece::Color::Black) > b.getNumPieces(Piece::Color::Yellow)){
            std::cout << "Black wins!\n";
            ++RANDOM_wins;
        }
        else{
            std::cout << "Yellow wins!\n";
            ++AI_wins;
        }
    }

    std::cout << "AI wins: " << AI_wins << '\n';
    std::cout << "Random wins: " << RANDOM_wins << '\n';
    std::cout << "Ratio: " << (AI_wins*100.) / num_games << "%\n";
}