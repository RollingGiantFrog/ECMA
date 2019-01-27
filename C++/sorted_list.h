#ifndef SORTED_LIST_H_INCLUDED
#define SORTED_LIST_H_INCLUDED

#include <list>
#include <vector>

#include "instance.h"

struct Pathway {
    int weight;
    int value;
    Node pred;
};

class SortedList {
public:
    std::list<Pathway> ways;

    SortedList() {};
    bool addValue(int weight, int value, Node pred);
    bool empty() const;
    unsigned int size() const;
    Pathway operator[] (unsigned int i) const;
    std::vector<Pathway> getList() const;
};

#endif // SORTED_LIST_H_INCLUDED
