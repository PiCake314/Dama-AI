#pragma once

#include <iostream>
#include <string_view>
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


struct Game{
    constexpr static std::string_view startingFen = "8/yyyyyyyy/yyyyyyyy/8/8/bbbbbbbb/bbbbbbbb/8";
    Piece::Color turn = Piece::Color::Black;
    std::array<std::array<Piece, width>, height> board;
    FixedSizeCache cache{1'000'000};
    ZobristHash zobristHash;

    int numYellow{}, numShaikhYellow{};
    int numBlack{}, numShaikhBlack{};

    Game(std::string_view fen = startingFen){
        int x = 0;
        int y = 0;

        for(char c : fen){
            switch(c){
                case 'y':
                    ++numYellow;
                    board[y][x] = Piece{{x, y}, Piece::Color::Yellow, false};
                    ++x;
                    break;
                case 'Y':
                    ++numYellow; ++numShaikhYellow;
                    board[y][x] = Piece{{x, y}, Piece::Color::Yellow, true};
                    ++x;
                    break;
                case 'b':
                    ++numBlack;
                    board[y][x] = Piece{{x, y}, Piece::Color::Black, false};
                    ++x;
                    break;
                case 'B':
                    ++numBlack; ++numShaikhBlack;
                    board[y][x] = Piece{{x, y}, Piece::Color::Black, true};
                    ++x;
                    break;
                case '/':
                    ++y;
                    x = 0;
                    break;
                default:
                    int num = c - '0';
                    x += num;
                    break;
            }
        }
    }


    bool hasntEnded(){
        return numYellow > 0 && numBlack > 0;
    }


    void play(const Move& move){
        Piece p = board[move.positions[0].y][move.positions[0].x];
        board[move.positions[0].y][move.positions[0].x] = Piece::nullPiece;

        for(const auto& p : move.eaten){
            if(p.color == Piece::Color::Black){
                if(p.shaikh) --numShaikhBlack;
                board[p.position.y][p.position.x] = Piece::nullPiece;
                --numBlack;
            }
            else /*if(p.color == Piece::Color::Yellow)*/{
                if(p.shaikh) --numShaikhYellow;
                board[p.position.y][p.position.x] = Piece::nullPiece;
                --numYellow;
            }
        }

        p.position = move.positions.back();

        // promotion
        if(!p.shaikh && (p.position.y == 0 || p.position.y == 7)){ // might need to check color too
            p.shaikh = true;
            if(p.color == Piece::Color::Black) ++numShaikhBlack;
            else ++numShaikhYellow;
        }

        board[p.position.y][p.position.x] = p;


        turn = p.otherColor();
    }


    std::vector<Position> canEatShaikh(const Piece& p1, const Piece& p2){
        if((p2.position.x != p1.position.x && p2.position.y != p1.position.y) || p1.color == p2.color)
            return {};

        // check if this piece can jump over the other piece even if it is a long jump
        if(p2.position.x == p1.position.x){
            if(p2.position.y == 0 || p2.position.y == 7) return {};
            
            int dist = p2.position.y - p1.position.y;
            int dir = dist > 0 ? 1 : -1;
            dist = std::abs(dist);

            for(int i = 1; i < dist; ++i){
                if(board[p1.position.y + i * dir][p1.position.x].color != Piece::Color::None){
                    return {};
                }
            }

            if(board[p2.position.y + dir][p2.position.x].color != Piece::Color::None) return {};

            std::vector<Position> positions;

            for(int i = 1;
                board[p2.position.y + (i*dir)][p2.position.x].color == Piece::Color::None
                && (p2.position.y + (i*dir)) >= 0
                && (p2.position.y + (i*dir)) < height
                ; ++i){
                positions.push_back(Position{p2.position.x, p2.position.y + (i * dir)});
            }

            return positions;
        }
        else{ // if p2.position.y == p1.position.y
            if(p2.position.x == 0 || p2.position.x == 7) return {};

            int dist = p2.position.x - p1.position.x;
            int dir = dist > 0 ? 1 : -1;
            dist = std::abs(dist);

            if(board[p2.position.y][p2.position.x + dir].color != Piece::Color::None) return {};

            for(int i = 1; i < dist; ++i){
                if(board[p1.position.y][p1.position.x + i * dir].color != Piece::Color::None){
                    return {};
                }
            }

            std::vector<Position> positions;

            // for(int i = dist+1; board[p1.position.y][p1.position.x + (i * dir)].color == Piece::Color::None; ++i){
            for(int i = 1;
                board[p2.position.y][p2.position.x + (i*dir)].color == Piece::Color::None // taken from the other if
                && (p2.position.x + (i*dir)) >= 0
                && (p2.position.x + (i*dir)) < width;
                ++i){
                positions.push_back(Position{p2.position.x + (i*dir), p2.position.y});
            }

            return positions;
        }
    }



    Position canEatPawn(const Piece& p1, const Piece& p2){
        if((p2.position.x != p1.position.x && p2.position.y != p1.position.y) || p1.color == p2.color)
            return Position::nullPosition;

        int result = p1.color == Piece::Color::Yellow ? 1 : -1;

        // check if the distance between the two pieces is more than one square
        if((p2.position.x == p1.position.x && p2.position.y - p1.position.y != result)
        || (p2.position.y == p1.position.y && std::abs(p2.position.x - p1.position.x) != 1))
        return Position::nullPosition;


        // check if this piece can jump over the other piece
        if(p2.position.x == p1.position.x){
            if(p2.position.y == 0 || p2.position.y == 7) return Position::nullPosition;

            if(board[p2.position.y + result][p2.position.x].color == Piece::Color::None){
                return Position{p2.position.x, p2.position.y + result};
            }
        }
        else{
            // check both left and right
            if(p2.position.x == 0 || p2.position.x == 7) return Position::nullPosition;
        
            if(board[p2.position.y][p2.position.x + 1].color == Piece::Color::None){ // right
                return Position{p2.position.x + 1, p2.position.y};
            }   

            if(board[p2.position.y][p2.position.x - 1].color == Piece::Color::None){ // left
                return Position{p2.position.x - 1, p2.position.y};
            }   
        }



        return Position::nullPosition;
    }

    void makeSingleMove(Piece& p, const Piece& eaten, const Position& pos){
        board[eaten.position.y][eaten.position.x] = Piece::nullPiece;
        board[p.position.y][p.position.x] = Piece::nullPiece;
        board[pos.y][pos.x] = p;
        p.position = pos;
    }

    void unmakeSingleMove(Piece& p, const Piece& eaten, const Position& oldPos){
        board[eaten.position.y][eaten.position.x] = eaten;
        board[p.position.y][p.position.x] = Piece::nullPiece;
        p.position = oldPos;
        board[p.position.y][p.position.x] = p;
    }

    std::vector<Move> generateMovesShaikh(Piece p, bool eating = false){
        std::vector<Move> moves;

        for(auto directionIndex : directions){
            for(int num_square = 1; num_square <= data[p.position.y][p.position.x][directionIndex]; ++num_square){
                if(directionIndex == north && p.position.y == 0) continue;
                if(directionIndex == east && p.position.x == 7) continue;
                if(directionIndex == south && p.position.y == 7) continue;
                if(directionIndex == west && p.position.x == 0) continue;

                int x = p.position.x;
                int y = p.position.y;
                int x2 = p.position.x;
                int y2 = p.position.y;

                switch(directionIndex){
                    case north:
                        y -= num_square;
                        y2 -= (num_square + 1);
                        break;
                    case east:
                        x += num_square;
                        x2 += (num_square + 1);
                        break;
                    case south:
                        y += num_square;
                        y2 += (num_square + 1);
                        break;
                    case west:
                        x -= num_square;
                        x2 -= (num_square + 1);
                        break;
                }

                if(board[y][x].color == Piece::Color::None && !eating){
                    moves.push_back(Move{p.position, {x, y}});
                }
                else if(board[y][x].color != p.color){
                    if(x2 < 0 || x2 >= width || y2 < 0 || y2 >= height) continue;;
                    if(board[y2][x2].color != Piece::Color::None) continue;;

                    auto positions = canEatShaikh(p, board[y][x]);

                    for(const auto& pos : positions){

                        const Position oldPos = p.position;
                        const Piece eaten = board[y][x];

                        Move move{p.position, pos, true};
                        move.eaten.push_back(eaten);


                        makeSingleMove(p, eaten, pos);
                        auto moves2 = generateMovesShaikh(p, true);
                        unmakeSingleMove(p, eaten, oldPos);


                        // add the eaten piece to the move
                        for(const auto& m : moves2){
                            Move m2 = move;

                            for(int i = 1; i < m.positions.size(); ++i){
                                m2.positions.push_back(m.positions[i]);
                            }

                            m2.eat = true;
                            m2.eaten.insert(m2.eaten.end(), m.eaten.begin(), m.eaten.end());
                            moves.push_back(m2);
                        }

                        moves.push_back(move);
                    }
                }
                else break;
            }
        }

        return moves;
    }


    std::vector<Move> generateMovesPawn(Piece p, bool eating = false){
        std::vector<Move> moves;

        for(auto directionIndex : directions){
            if(directionIndex == north && p.color == Piece::Color::Yellow) continue;
            if(directionIndex == south && p.color == Piece::Color::Black) continue;

            if(directionIndex == east && p.position.x == 7) continue;
            if(directionIndex == west && p.position.x == 0) continue;


            int x = p.position.x;
            int y = p.position.y;

            switch(directionIndex){
                case north:
                    y -= 1;
                    break;
                case east:
                    x += 1;
                    break;
                case south:
                    y += 1;
                    break;
                case west:
                    x -= 1;
                    break;
            }

            if(board[y][x].color == Piece::Color::None && !eating){
                moves.push_back(Move{p.position, {x, y}});
            }
            else if(board[y][x].color != p.color){
                Position pos = canEatPawn(p, board[y][x]);

                if(pos){

                    Move move{p.position, pos, true};
                    move.eaten.push_back(board[y][x]);

                    const Piece eaten = board[y][x];
                    const Position oldPos = p.position;

                    makeSingleMove(p, eaten, pos);
                    auto moves2 = generateMovesPawn(p, true);
                    unmakeSingleMove(p, eaten, oldPos);


                    // add the eaten piece to the move
                    for(auto m : moves2){
                        Move m2 = move;

                        for(int i = 1; i < m.positions.size(); ++i){
                            m2.positions.push_back(m.positions[i]);
                        }

                        m2.eat = true;
                        m2.eaten.insert(m2.eaten.end(), m.eaten.begin(), m.eaten.end());

                        moves.push_back(m2);
                    }

                    moves.push_back(move);
                }
            }
        }

        return moves;
    }


    auto generateMoves(){
        std::vector<Move> moves;

        for(const auto& row : board){
            for(Piece p : row){
                if(p.color == turn){
                    std::vector<Move> moves2 = p.shaikh ? generateMovesShaikh(p) : generateMovesPawn(p);

                    moves.insert(moves.end(), moves2.begin(), moves2.end());
                }
            }
        }


        bool eating = std::any_of(moves.begin(), moves.end(), [](const Move& m){ return m.eat; });
        if(eating){
            const int max = std::ranges::max_element(moves, [](const Move& m1, const Move& m2){
                return m1.positions.size() < m2.positions.size();
            })->positions.size();

            if(max == 2) std::erase_if(moves, [](const Move& m){ return !m.eat; });
            else /* max > 2 */ std::erase_if(moves, [max](const Move& m){ return m.positions.size() < max; });
        }

        return moves;
    }

     bool makeMove(const Move& move){
        // make the move and return the eaten pieces
        // std::vector<Piece> eaten;
        bool promotion = false;

        Piece p = board[move.positions[0].y][move.positions[0].x];
        board[move.positions[0].y][move.positions[0].x] = Piece::nullPiece;

        for(const auto& p : move.eaten){
            if(p.color == Piece::Color::Black){
                board[p.position.y][p.position.x] = Piece::nullPiece;
                --numBlack;
            }
            else /*if(p.color == Piece::Color::Yellow)*/{
                board[p.position.y][p.position.x] = Piece::nullPiece;
                --numYellow;
            }
        }

        p.position = move.positions.back();

        // promotion
        if(!p.shaikh && (p.position.y == 0 || p.position.y == 7)){ // might need to check color too
            promotion = true;
            p.shaikh = true;
            if(p.color == Piece::Color::Black) ++numShaikhBlack;
            else ++numShaikhYellow;
        }
        
        board[p.position.y][p.position.x] = p;

        turn = p.otherColor();

        return promotion;
    }


    void unmakeMove(const Move& move, bool promotion){
        Piece p = board[move.positions.back().y][move.positions.back().x];
        board[move.positions.back().y][move.positions.back().x] = Piece::nullPiece;

        for(const auto& e : move.eaten){
            board[e.position.y][e.position.x] = e;
            if(e.color == Piece::Color::Black) ++numBlack;
            else /*if e.color == Yellow*/ ++numYellow;
        }

        if(promotion){
            p.shaikh = false;
            if(p.color == Piece::Color::Black) --numShaikhBlack;
            else --numShaikhYellow;
        }
        p.position = move.positions[0];
        board[p.position.y][p.position.x] = p;

        turn = p.otherColor();
    }


    int countMaterial(Piece::Color color, bool endgame = false){
        int total = 0;

        if(endgame){
            // calculate the sum of the distances of every piece to the other pieces of opposite color
            for(const auto& row : board){
                for(const auto& p : row){
                    if(p.color == color){
                        int sum = 0;
                        for(const auto& row2 : board){
                            for(const auto& p2 : row2){
                                if(p2.color == p.otherColor()){
                                    // sum += std::abs(p.position.x - p2.position.x) + std::abs(p.position.y - p2.position.y);
                                    // sum += std::max(std::abs(p.position.x - p2.position.x), std::abs(p.position.y - p2.position.y));
                                    // calculating the distance using the pythagorean theorem
                                    sum += std::sqrt(std::pow(p.position.x - p2.position.x, 2) + std::pow(p.position.y - p2.position.y, 2));
                                }
                            }
                        }

                        total += sum;
                    }
                }
            }

            if(color == Piece::Color::Black) // minimize the distance if have more shaikhs
                return numShaikhBlack >= numShaikhYellow ? -total : total;
            else // if(color == Piece::Color::Yellow)
                return numShaikhYellow >= numShaikhBlack ? -total : total;
        }
        else{
            auto respectivePawnBias = color == Piece::Color::Black ? // avoiding double check
            [](const Piece& p){ return height - p.position.y +1; } : [](const Piece& p){ return p.position.y +1; };

            auto respectiveUpperHalf = color == Piece::Color::Black ? // avoiding double check
            [](const Piece& p){ return p.position.y < 4; } : [](const Piece& p){ return p.position.y > 3; };

            const int respective_shaikh_bias = height + 2; // not a function since it doesn't depend on the piece

            for(const auto& row : board){
                for(const auto& p : row){
                    if(p.color == color){
                        if(p.shaikh) total += (SHAIKH_VALUE + respective_shaikh_bias);
                        else if(respectiveUpperHalf(p)) total += (PAWN_ADVANCED_VALUE + respectivePawnBias(p));
                        else total += PAWN_VALUE + respectivePawnBias(p);
                    }
                }
            }
        }

        return total;
    }

    int evaluate(){
        const bool endgame = numBlack == numShaikhBlack && numYellow == numShaikhYellow;
        int blackMaterial = countMaterial(Piece::Color::Black, endgame);
        int yellowMaterial = countMaterial(Piece::Color::Yellow, endgame);

        const int prespective = turn == Piece::Color::Black ? 1 : -1;
        const int material = blackMaterial - yellowMaterial;
        const int evaluation = material / (numBlack + numYellow);

        return evaluation * prespective;
    }

    void orderMoves(std::vector<Move>& moves){
        for(auto& m : moves){
            int guess = 0;
            for(const auto& e : m.eaten){
                guess += (10 * e.value() - board[m.positions[0].y][m.positions[0].x].value());
            }

            if(m.promotion && !board[m.positions[0].y][m.positions[0].x].shaikh) guess += 100;

            m.guess = guess;
        }

        std::ranges::sort(moves, [](const Move& m1, const Move& m2){
            // return m1.eaten.size() > m2.eaten.size();
            return m1.guess > m2.guess;
        });
    }

    int alphaBeta(int depth = 10, int alpha = -INF, int beta = INF){

        if(depth == 10 && cache.contains(zobristHash.hash(board))){
            auto c = cache.get(zobristHash.hash(board));
            return c.turn == turn ? c.score : -c.score;
        }

        if(depth == 0) return evaluate();

        auto moves = generateMoves();
        // orderMoves(moves);

        if(moves.empty()) return -INF;
        // if(moves.size() == 1) return evaluate(); // bad idea

        for(const auto& m : moves){
            bool promotion = makeMove(m);
            int score = -alphaBeta(depth -1, -beta, -alpha);
            unmakeMove(m, promotion);

            if(score >= beta) return beta;
            if(score > alpha) alpha = score;
        }

        cache.insert(zobristHash.hash(board), alpha, turn);

        return alpha;
    }

    int minimax(int depth = 10){
        if(depth == 0) return evaluate();

        auto moves = generateMoves();

        int best = -INF;

        if(moves.empty()) return best;
        // if(moves.size() == 1) return evaluate();


        for(const auto& m : moves){
            bool promotion = makeMove(m);
            int score = -minimax(depth - 1);
            unmakeMove(m, promotion);

            if(score > best) best = score;
        }

        return best;
    }

    void print(){
        for(int i = 0; i < 8; ++i){
            for(int j = 0; j < 8; ++j){
                switch(board[i][j].color){
                    case Piece::Color::None:
                        std::cout << '-';
                        break;
                    case Piece::Color::Yellow:
                        if(board[i][j].shaikh) std::cout << 'Y';
                        else std::cout << 'y';
                        break;
                    case Piece::Color::Black:
                        if(board[i][j].shaikh) std::cout << 'B';
                        else std::cout << 'b';
                        break;
                }
            }
            std::cout << '\n';
        }
    }
};



auto precomputeData(){
    std::array<std::array<std::array<int, 4>, width>, height> data{};

    for(int i = 0; i < 8; ++i){
        for(int j = 0; j < 8; ++j){
            int num_north = i;
            int num_east = width - j - 1;
            int num_south = height - i - 1;
            int num_west = j;

            data[i][j] = {num_north, num_east, num_south, num_west};
        }
    }

    return data;
}
