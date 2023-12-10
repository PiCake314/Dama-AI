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

class FixedSizeCache{
    struct Hashed{
        int score;
        Piece::Color turn;
    };

    std::unordered_map<uint64_t, Hashed> table;
    std::vector<uint64_t> keys;
    const size_t maxSize;
    
public:
    FixedSizeCache(size_t maxSize) : maxSize{maxSize} {}

    void insert(uint64_t key, int value, Piece::Color turn){
        if(table.size() >= maxSize){
            table.erase(keys.front());
            keys.erase(keys.begin());
        }

        if(table.contains(key)){
            table[key] = {value, turn};

            auto it = std::find(keys.begin(), keys.end(), key);
            keys.erase(it);
            keys.push_back(key);

            return;
        }

        table[key] = {value, turn};
        keys.push_back(key);
    }

    bool contains(uint64_t key){
        return table.contains(key);
    }

    Hashed get(uint64_t key){
        return table.at(key);
    }

    size_t size(){
        return table.size();
    }

    size_t capacity(){
        return maxSize;
    }
};
