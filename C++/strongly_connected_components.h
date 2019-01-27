#ifndef STRONGLY_CONNECTED_COMPONENTS_H_INCLUDED
#define STRONGLY_CONNECTED_COMPONENTS_H_INCLUDED

#include <vector>
#include <stack>

struct SCCUtils {
    const std::vector< std::vector<int> >& adj;
    std::vector<bool> onstack;
    std::vector<int> index;
    std::vector<int> lowlink;
    std::vector<int> scc;
    std::stack<int> stack;
    int next, nextgroup;

    std::vector< std::vector<int> > groups;
};

std::vector< std::vector<int> > sconnect(const std::vector< std::vector<int> >& neighbors, int node);

#endif // STRONGLY_CONNECTED_COMPONENTS_H_INCLUDED
