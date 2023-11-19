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


class FixedSizeCache{
    std::unordered_map<uint64_t, int> table;
    std::vector<uint64_t> keys;
    const size_t maxSize;
    
public:
    FixedSizeCache(size_t maxSize) : maxSize{maxSize} {}

    void insert(uint64_t key, int value){
        if(table.size() == maxSize){
            table.erase(keys.front());
            keys.erase(keys.begin());
        }

        table[key] = value;
        keys.push_back(key);
    }

    bool contains(uint64_t key){
        return table.contains(key);
    }

    int get(uint64_t key){
        return table[key];
    }

    size_t size(){
        return table.size();
    }

    size_t capacity(){
        return maxSize;
    }
};
