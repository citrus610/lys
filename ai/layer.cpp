#include "layer.h"

Layer::Layer()
{
    this->data.clear();
    this->width = 0;
};

void Layer::init(size_t width)
{
    this->width = width;
    this->data.reserve(width);
    this->data.clear();
};

void Layer::clear()
{
    this->data.clear();
};

void Layer::add(Node& node)
{
    if (this->data.size() < this->width) {
        this->data.push_back(node);

        if (this->data.size() == this->width) {
            std::make_heap(
                this->data.begin(),
                this->data.end(),
                [&] (Node& a, Node& b) { return b < a; }
            );
        }

        return;
    }

    if (this->data[0] < node) {
        std::pop_heap(
            this->data.begin(),
            this->data.end(),
            [&] (Node& a, Node& b) { return b < a; }
        );

        this->data.back() = node;

        std::push_heap(
            this->data.begin(),
            this->data.end(),
            [&] (Node& a, Node& b) { return b < a; }
        );
    }
};

void Layer::sort()
{
    if (this->data.size() < this->width) {
        std::sort(
            this->data.begin(),
            this->data.end(),
            [&] (const Node& a, const Node& b) { return b < a; }
        );

        return;
    }

    std::sort_heap(
        this->data.begin(),
        this->data.end(),
        [&] (const Node& a, const Node& b) { return b < a; }
    );
};