#pragma once

#include "../core/core.h"
#include "../lib/xxh/xxh3.h"
#include "../lib/parallel_hashmap/phmap.h"
#include "node.h"

class Table
{
public:
    phmap::flat_hash_set<u64> set;
public:
    Table();
public:
    u64 get_hash(Field& field);
public:
    bool get_entry(u64 hash);
    bool set_entry(u64 hash);
};

Table::Table()
{
    this->set = phmap::flat_hash_set<u64>();
    this->set.reserve(1 << 12);
};

u64 Table::get_hash(Field& field)
{
    alignas(16) u8 v[16 * Cell::COUNT];
    for (u8 i = 0; i < Cell::COUNT; ++i) {
        _mm_store_si128((__m128i*)(v + i * 16), field.data[i].data);
    }
    return xxh3_64((const void*)v, 16 * Cell::COUNT);
};

bool Table::get_entry(u64 hash)
{
    return this->set.contains(hash);
};

bool Table::set_entry(u64 hash)
{
    if (this->set.contains(hash)) {
        return false;
    }
    this->set.insert(hash);
    return true;
};