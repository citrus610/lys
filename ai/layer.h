#pragma once

#include "node.h"

class Layer
{
public:
    std::vector<Node> data;
    size_t width;
public:
    Layer();
public:
    void init(size_t width = 100);
    void clear();
    void add(Node& node);
    void sort();
};