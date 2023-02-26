#pragma once

#include "../core/core.h"
#include "../lib/xxh/xxh3.h"
#include "../lib/parallel_hashmap/phmap.h"

constexpr u32 TTABLE_SIZE_DEFAUT = 1 << 12;

template <typename T>
class TTable
{
public:
    phmap::flat_hash_map<u64, T>* bucket;
    u32 size;
public:
    TTable();
    ~TTable();
public:
    void init(u32 init_size = TTABLE_SIZE_DEFAUT);
    void destroy();
public:
    u64 get_hash(Field& field, u8 depth);
    u32 get_index(u64 hash);
public:
    bool get_entry(u64 hash, T& entry);
    void set_entry(u64 hash, T& entry);
};

template <typename T>
TTable<T>::TTable()
{
    this->bucket = nullptr;
    this->size = 0;
};

template <typename T>
TTable<T>::~TTable()
{
    this->destroy();
};

template <typename T>
void TTable<T>::init(u32 init_size)
{
    assert((init_size & (init_size - 1)) == 0);
    if (this->bucket != nullptr) {
        assert(false);
        return;
    }
    this->size = init_size;
    this->bucket = new phmap::flat_hash_map<u64, T>[this->size];
};

template <typename T>
void TTable<T>::destroy()
{
    if (this->bucket == nullptr) {
        return;
    }
    delete[] this->bucket;
};

template <typename T>
u64 TTable<T>::get_hash(Field& field, u8 depth)
{
    alignas(16) u8 v[16 * Cell::COUNT + 1];
    for (u8 i = 0; i < Cell::COUNT; ++i) {
        _mm_store_si128((__m128i*)(v + i * 16), field.data[i].data);
    }
    v[16 * Cell::COUNT] = depth;
    return xxh3_64((const void*)v, 16 * Cell::COUNT + 1);
};

template <typename T>
u32 TTable<T>::get_index(u64 hash)
{
    return hash & (this->size - 1);
};

template <typename T>
bool TTable<T>::get_entry(u64 hash, T& entry)
{
    u32 index = this->get_index(hash);
    auto it = this->bucket[index].find(hash);
    if (it == this->bucket[index].end()) {
        return false;
    }
    entry = it->second;
    return true;
};

template <typename T>
void TTable<T>::set_entry(u64 hash, T& entry)
{
    u32 index = this->get_index(hash);
    this->bucket[index][hash] = entry;
};