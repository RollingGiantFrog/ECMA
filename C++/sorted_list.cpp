#include "sorted_list.h"

#include <list>

bool SortedList::addValue(int weight, int value, int pred) {
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
