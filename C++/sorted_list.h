#ifndef SORTED_LIST_H_INCLUDED
#define SORTED_LIST_H_INCLUDED

#include <list>
#include <vector>

struct Pathway {
    int weight;
    int value;
    int pred;
};

class SortedList {
public:
    std::list<Pathway> ways;

    SortedList() {};
    bool addValue(int weight, int value, int pred);
    bool empty() const;
    unsigned int size() const;
    Pathway operator[] (unsigned int i) const;
    std::vector<Pathway> getList();
};

#endif // SORTED_LIST_H_INCLUDED
