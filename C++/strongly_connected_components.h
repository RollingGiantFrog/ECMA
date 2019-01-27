#ifndef STRONGLY_CONNECTED_COMPONENTS_H_INCLUDED
#define STRONGLY_CONNECTED_COMPONENTS_H_INCLUDED

#include <vector>
#include <stack>

#include "node.h"

struct SCCUtils {
    const std::vector< std::vector<Node> >& adj;
    std::vector<bool> onstack;
    std::vector<int> index;
    std::vector<int> lowlink;
    std::stack<Node> stack;
    int next;

    std::vector< std::vector<Node> > groups;
};

std::vector< std::vector<Node> > sconnect(const std::vector< std::vector<Node> >& neighbors, Node node);

#endif // STRONGLY_CONNECTED_COMPONENTS_H_INCLUDED
