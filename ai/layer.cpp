#include "layer.h"

void Layer::init(size_t width)
{
    this->width = width;
    this->table = Table();
    this->data.reserve(this->width);
    this->data.clear();
};

void Layer::add(Node& node)
{
    u64 hash = this->table.get_hash(node.field);

    if (!this->table.set_entry(hash)) {
        return;
    }

    if (this->data.size() < this->width) {
        this->data.push_back(node);
        if (this->data.size() == this->width) {
            std::make_heap(this->data.begin(), this->data.end(), [&] (Node& a, Node& b) { return b < a; });
        }
        return;
    }
    
    if (this->data[0] < node) {
        std::pop_heap(this->data.begin(), this->data.end(), [&] (Node& a, Node& b) { return b < a; });
        this->data.back() = node;
        std::push_heap(this->data.begin(), this->data.end(), [&] (Node& a, Node& b) { return b < a; });
    }
};