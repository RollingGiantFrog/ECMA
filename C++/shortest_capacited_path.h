#ifndef SHORTEST_CAPACITED_PATH_H_INCLUDED
#define SHORTEST_CAPACITED_PATH_H_INCLUDED

#include <iostream>
#include <algorithm>
#include <vector>
#include <deque>
#include <assert.h>

#include "instance.h"
#include "path.h"
#include "sorted_list.h"

// Metrics definition

// Static metric is the weight and distance for the static problem
class StaticNodeMetric {
public:
    const Instance& instance;
    StaticNodeMetric(const Instance& instance) : instance(instance) {};
    int operator() (Node u) const { return instance.p[u]; };
};

class StaticEdgeMetric {
public:
    const Instance& instance;
    StaticEdgeMetric(const Instance& instance) : instance(instance) {};
    int operator() (Node u, Node v) const { return instance.d[u][v]; };
};

// Semi worst case metric is the weight penalized by the deviation on weight, and the distance penalized by the relative deviation on distance
class SemiWorstCaseNodeMetric {
public:
    const Instance& instance;
    SemiWorstCaseNodeMetric(const Instance& instance) : instance(instance) {};
    int operator() (Node u) const { return instance.p[u] + instance.ph[u]; };
};

class SemiWorstCaseEdgeMetric {
public:
    const Instance& instance;
    SemiWorstCaseEdgeMetric(const Instance& instance) : instance(instance) {};
    double operator() (Node u, Node v) const { return instance.d[u][v]*(1. + instance.D[u][v]); };
};

// Class used to compute dynamic programming w.r.t some metric
// It can be done on the reversed graph too
template<typename NodeMetric, typename EdgeMetric>
class ShortestCapacitedPath {
public:
    const Instance& instance;

    // nodes we want to link by a best path
    Node initNode;
    Node endNode;

    // Metrics considered to measure node weights and edge distances
    const NodeMetric& nodeMetric;
    const EdgeMetric& edgeMetric;

    // Dynamic table where all the best possible couples of weight and distance are stored
    // along with the predecessor in the matching path
    std::vector<SortedList> table;

    // Iteration count to end computation
    unsigned int iter;

    // Wether computation has been done on the reversed graph
    bool reverseMode;

    ShortestCapacitedPath(const Instance& instance, Node initNode, Node endNode, const NodeMetric& nodeMetric, const EdgeMetric& edgeMetric, bool reverseMode = false) :
    instance(instance),
    initNode(initNode),
    endNode(endNode),
    nodeMetric(nodeMetric),
    edgeMetric(edgeMetric),
    iter(0),
    reverseMode(reverseMode) {

        for (int i = 0; i < instance.n; ++i) table.push_back(SortedList());

        // Table initialization with the initial node
        table[initNode].addValue(nodeMetric(initNode),0,-1);

        // Starting the propagation with the initial node
        std::deque<Node> nodes;
        nodes.push_back(initNode);

        iter = 0;

        if (!reverseMode) {
            for (int k = 0; k < instance.n; ++k) {
                // Keeping track of the modified nodes so that we don't add a node twice in the queue
                bool modified[instance.n] = {false};

                // At the kth step, we iterate over all nodes that are reachable by path of length k
                unsigned int nodesCount = nodes.size();
                while (nodesCount > 0) {
                    nodesCount -= 1;
                    Node u = nodes.front();
                    nodes.pop_front();

                    // Propagating node u, iterating over its successors
                    for (unsigned int i = 0; i < instance.neighbors[u].size(); ++i) {
                        Node v = instance.neighbors[u][i];
                        bool mod = false;
                        for (std::list<Pathway>::iterator ways_it = table[u].ways.begin(); ways_it != table[u].ways.end(); ++ways_it) {
                            Pathway way = (*ways_it);
                            int weight = way.weight + nodeMetric(v);
                            if (weight <= instance.S) mod = table[v].addValue(weight,way.value + edgeMetric(u,v),u) || mod;
                            iter += 1;
                        }
                        // v can be reached by a path of length k+1 and has been modified (i.e has something new to propagate)
                        if (mod && !modified[v]) {
                            nodes.push_back(v);
                            modified[v] = true;
                        }
                    }
                }

                // We stop early if there is no new modified nodes left
                if (nodes.size() == 0) break;
            }
        }

        else {
            for (int k = 0; k < instance.n; ++k) {
                // Keeping track of the modified nodes so that we don't add a node twice in the queue
                bool modified[instance.n] = {false};

                // At the kth step, we iterate over all nodes that are reachable by path of length k
                unsigned int nodesCount = nodes.size();
                while (nodesCount > 0) {
                    nodesCount -= 1;
                    Node u = nodes.front();
                    nodes.pop_front();

                    // Propagating node u, iterating over its successors
                    for (unsigned int i = 0; i < instance.predecessors[u].size(); ++i) {
                        Node v = instance.predecessors[u][i];
                        bool mod = false;
                        for (std::list<Pathway>::iterator ways_it = table[u].ways.begin(); ways_it != table[u].ways.end(); ++ways_it) {
                            Pathway way = (*ways_it);
                            int weight = way.weight + nodeMetric(v);
                            if (weight <= instance.S)
                                mod = table[v].addValue(weight,way.value + edgeMetric(v,u),u) || mod;
                            iter += 1;
                        }

                        // v can be reached by a path of length k+1 and has been modified (i.e has something new to propagate)
                        if (mod && !modified[v]) {
                            nodes.push_back(v);
                            modified[v] = true;
                        }
                    }
                }

                // We stop early if there is no new modified nodes left
                if (nodes.size() == 0) break;
            }
        }
    }

    // Extract a specific optimal st-path w.r.t the used metric
    // As there are several best possible paths (not dominated paths), it extracts the path at position pathIdx
    // Given that the first path (idx = 0) is the lightest and longest,
    // and the last path (idx = table[t].size()-1 or simply idx = -1) is the heaviest and the shortest
    Path extractPathNodes(Node s, Node t, int pathIdx) const {
        assert(!table[t].empty());

        while(pathIdx < 0) pathIdx += table[t].size();

        std::vector<Node> nodes;
        nodes.push_back(t);

        std::vector<Pathway> ways = (table[t].getList());
        int weight = ways[pathIdx].weight - nodeMetric(t);
        Node u = ways[pathIdx].pred;

        while (u != s) {
            nodes.push_back(u);
            ways = table[u].getList();
            for (unsigned int i = 0; i < ways.size(); ++i) {
                Pathway way = ways[i];
                if (way.weight == weight) {
                    weight = weight - nodeMetric(u);
                    u = way.pred;
                }
            }
        }

        nodes.push_back(s);

        std::reverse(nodes.begin(),nodes.end());
        return Path(instance, nodes);
    }
};

#endif // SHORTEST_CAPACITED_PATH_H_INCLUDED
