#pragma once

#include "node.h"
#include "table.h"

class Layer
{
public:
    Table table;
    std::vector<Node> data;
    size_t width;
public:
    void init(size_t width);
    void add(Node& node);
};