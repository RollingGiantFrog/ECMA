#include "strongly_connected_components.h"

#include <vector>
#include <iostream>

void tarjan(SCCUtils& sccu, int v) {
    sccu.index[v] = sccu.next;
    sccu.lowlink[v] = sccu.next;
    sccu.next += 1;
    sccu.stack.push(v);
    sccu.onstack[v] = true;

    for (unsigned int i = 0; i < sccu.adj[v].size(); ++i) {
        int w = sccu.adj[v][i];
        if (sccu.index[w] == -1) {
            tarjan(sccu, w);
            sccu.lowlink[v] = std::min(sccu.lowlink[v], sccu.lowlink[w]);
        }
        else if (sccu.onstack[w]) {
            sccu.lowlink[v] = std::min(sccu.lowlink[v], sccu.index[w]);
        }
    }

    if (sccu.index[v] == sccu.lowlink[v]) {
        std::vector<int> com;
        while (true) {
            int w = sccu.stack.top();
            sccu.stack.pop();
            sccu.onstack[w] = false;
            com.push_back(w);
            if (w == v) break;
        }
        sccu.groups.push_back(com);
    }
}

std::vector< std::vector<int> > sconnect(const std::vector< std::vector<int> >& neighbors, int node) {
    unsigned int N = neighbors.size();
    SCCUtils sccu = {neighbors, std::vector<bool>(N,false), std::vector<int>(N,-1), std::vector<int>(N,-1), std::vector<int>(N,-1), std::stack<int>(), 0, 0, std::vector< std::vector<int> >()};

    for (unsigned int v = 0; v < N; ++v) {
        if (sccu.index[v] == -1) tarjan(sccu, v);
    }

    return sccu.groups;
}
