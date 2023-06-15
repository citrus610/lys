#pragma once

#include "../core/core.h"
#include "detect.h"

namespace Form
{

constexpr u8 HEIGHT = 4;
constexpr u8 AREA = HEIGHT * 6;

struct Data
{
    u8 form[HEIGHT][6] = { 0 };
    i8 matrix[AREA][AREA] = { 0 };
};

i32 evaluate(Field& field, u8 height[6], const Data& pattern);

constexpr Data DEFAULT()
{
    Data pattern = { 0 };

    const u8 dform[HEIGHT][6] = 
    {
        {  4,  5, 10,  0,  0,  0 },
        {  1,  2,  6,  0,  0,  0 },
        {  1,  1,  3,  7,  8,  0 },
        {  2,  2,  2,  3,  3,  9 },
    };

    const i8 dmatrix[AREA][AREA] = 
    {
        { 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
        { 0,  2, -1, -1, -1,  0,  0,  0,  0,  0,  0 },
        { 0, -1,  4, -1,  0,  0,  0,  0,  0,  0,  0 },
        { 0, -1, -1,  1,  0,  0,  0, -1,  0, -1,  0 },
        { 0, -1,  0,  0,  1,  0,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0,  1,  0,  0,  0, -1 },
        { 0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0 },
        { 0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0 },
        { 0,  0,  0, -1,  0,  0,  0,  0,  0,  1,  0 },
        { 0,  0,  0,  0,  0,  0, -1,  0,  0,  0,  1 },
    };

    for (i8 x = 0; x < 6; ++x) {
        for (i8 y = 0; y < HEIGHT; ++y) {
            pattern.form[y][x] = dform[HEIGHT - 1 - y][x];
        }
    }

    for (i8 x = 0; x < AREA; ++x) {
        for (i8 y = 0; y < AREA; ++y) {
            pattern.matrix[y][x] = dmatrix[y][x];
        }
    }

    return pattern;
};

};