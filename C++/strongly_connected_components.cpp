#include "strongly_connected_components.h"

#include <vector>
#include <iostream>

/*void tarjan(SCCUtils& sccu, Node v) {
    std::stack< std::pair<Node,unsigned int> > work;
    work.push(std::make_pair(v,0));
    bool recurse;
    unsigned int i;
    while (!work.empty()) {
        v = work.top().first;
        i = work.top().second;
        work.pop();
        if (i == 0) {
            sccu.index[v] = sccu.next;
            sccu.lowlink[v] = sccu.next;
            sccu.next = sccu.next + 1;
            sccu.stack.push(v);
            sccu.onstack[v] = true;
        }
        recurse = false;
        for (unsigned int j = i; j < sccu.adj[v].size(); ++j) {
            Node w = sccu.adj[v][j];
            if (sccu.index[w] == -1) {
                work.push(std::make_pair(v, j+1));
                work.push(std::make_pair(w, 0));
                recurse = true;
                break;
            }
            else if (sccu.onstack[w])
                sccu.lowlink[v] = std::min(sccu.lowlink[v],sccu.index[w]);
        }
        if (recurse) continue;
        if (sccu.index[v] == sccu.lowlink[v]) {
            std::vector<Node> com;
            while (true) {
                Node w = sccu.stack.top();
                sccu.stack.pop();
                sccu.onstack[w] = false;
                com.push_back(w);
                if (w == v) break;
            }
            sccu.groups.push_back(com);
        }
        if (!work.empty()) {
            Node w = v;
            v = work.top().first;
            sccu.lowlink[v] = std::min(sccu.lowlink[v],sccu.lowlink[w]);
        }

    }
}*/

void tarjan(SCCUtils& sccu, Node v) {
    sccu.index[v] = sccu.next;
    sccu.lowlink[v] = sccu.next;
    sccu.next += 1;
    sccu.stack.push(v);
    sccu.onstack[v] = true;

    for (unsigned int i = 0; i < sccu.adj[v].size(); ++i) {
        Node w = sccu.adj[v][i];
        if (sccu.index[w] == -1) {
            tarjan(sccu, w);
            sccu.lowlink[v] = std::min(sccu.lowlink[v], sccu.lowlink[w]);
        }
        else if (sccu.onstack[w]) {
            sccu.lowlink[v] = std::min(sccu.lowlink[v], sccu.index[w]);
        }
    }

    if (sccu.index[v] == sccu.lowlink[v]) {
        std::vector<Node> com;
        while (true) {
            Node w = sccu.stack.top();
            sccu.stack.pop();
            sccu.onstack[w] = false;
            com.push_back(w);
            if (w == v) break;
        }
        sccu.groups.push_back(com);
    }
}

std::vector< std::vector<Node> > sconnect(const std::vector< std::vector<Node> >& neighbors, Node node) {
    unsigned int N = neighbors.size();
    SCCUtils sccu = {neighbors, std::vector<bool>(N,false), std::vector<int>(N,-1), std::vector<int>(N,-1), std::stack<Node>(), 0, std::vector< std::vector<Node> >()};

    /*for (unsigned int v = 0; v < N; ++v) {
        if (sccu.index[v] == -1) tarjan(sccu, v);
    }*/
    tarjan(sccu, node);

    return sccu.groups;
}
