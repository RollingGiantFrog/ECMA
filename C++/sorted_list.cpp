#include "sorted_list.h"

#include <list>
#include <vector>

bool SortedList::addValue(int weight, int value, Node pred) {
    std::list<Pathway>::reverse_iterator it_ = ways.rend();
    std::list<Pathway>::reverse_iterator it = ways.rbegin();

    while (it != ways.rend() && weight < (*it).weight) {
        if ((*it).value >= value) {
            it_ = it;
            break;
        }
        ++it;
    }

    while (it != ways.rend() && weight < (*it).weight) {
        ++it;
    }

    if (it_ == ways.rend()) it_ = it;

    if (it == ways.rend()) {
        ways.erase(ways.begin(),(it_).base());
        ways.push_front({weight, value, pred});
    }
    else {
        if ((*it).value <= value) return false;

        if ((*it).weight == weight) {
            it++;
            ways.erase(it.base(),it_.base());
            ways.insert(it_.base(),{weight, value, pred});
        }
        else {
            ways.erase(it.base(),it_.base());
            ways.insert(it_.base(),{weight, value, pred});
        }
    }
    return true;
}

bool SortedList::empty() const {
    return ways.empty();
}

unsigned int SortedList::size() const {
    return ways.size();
}

std::vector<Pathway> SortedList::getList() const {
    std::vector<Pathway> v;
    v.insert(v.end(),ways.begin(),ways.end());
    return v;
}
