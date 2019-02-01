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

class SemiWorstCaseNodeMetric {
public:
    const Instance& instance;
    float penalty;
    SemiWorstCaseNodeMetric(const Instance& instance, float penalty) : instance(instance), penalty(penalty) {};
    int operator() (Node u) const { return instance.p[u] + penalty*instance.ph[u]; };
};

class SemiWorstCaseEdgeMetric {
public:
    const Instance& instance;
    float penalty;
    SemiWorstCaseEdgeMetric(const Instance& instance, float penalty) : instance(instance), penalty(penalty) {};
    double operator() (Node u, Node v) const { return instance.d[u][v]*(1. + penalty*instance.D[u][v]); };
};

template<typename NodeMetric, typename EdgeMetric>
class ShortestCapacitedPath {
public:
    const Instance& instance;
    Node initNode;
    Node endNode;

    const NodeMetric& nodeMetric;
    const EdgeMetric& edgeMetric;

    std::vector<SortedList> table;
    unsigned int iter;

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

        table[initNode].addValue(nodeMetric(initNode),0,-1);

        if (!reverseMode) {
            iter = 0;
            std::deque<Node> nodes;
            nodes.push_back(initNode);
            for (int k = 0; k < instance.n; ++k) {
                //std::cout << "Step : " << k << " (" << nodes.size() << ")" << std::endl;
                bool modified[instance.n] = {false};
                unsigned int nodesCount = nodes.size();
                while (nodesCount > 0) {
                    nodesCount -= 1;
                    Node u = nodes.front();
                    nodes.pop_front();
                    for (unsigned int i = 0; i < instance.neighbors[u].size(); ++i) {
                        Node v = instance.neighbors[u][i];
                        bool mod = false;
                        for (std::list<Pathway>::iterator ways_it = table[u].ways.begin(); ways_it != table[u].ways.end(); ++ways_it) {
                            Pathway way = (*ways_it);
                            int weight = way.weight + nodeMetric(v);
                            if (weight <= instance.S)
                                mod = table[v].addValue(weight,way.value + edgeMetric(u,v),u) || mod;
                            iter += 1;
                        }
                        if (mod && !modified[v]) {
                            nodes.push_back(v);
                            modified[v] = true;
                        }
                    }
                }

                if (nodes.size() == 0) break;
            }
        }
        else {
            iter = 0;
            std::deque<Node> nodes;
            nodes.push_back(initNode);
            for (int k = 0; k < instance.n; ++k) {
                //std::cout << "Step : " << k << " (" << nodes.size() << ")" << std::endl;
                bool modified[instance.n] = {false};
                unsigned int nodesCount = nodes.size();
                while (nodesCount > 0) {
                    nodesCount -= 1;
                    Node u = nodes.front();
                    nodes.pop_front();
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
                        if (mod && !modified[v]) {
                            nodes.push_back(v);
                            modified[v] = true;
                        }
                    }
                }
                if (nodes.size() == 0) break;
            }
        }
    }

    Path extractPathNodes(Node s, Node t, int pathIdx) const {
        assert(!table[t].empty());

        std::vector<Node> nodes;
        nodes.push_back(t);
        std::vector<Pathway> ways = (table[t].getList());
        while(pathIdx < 0) pathIdx += table[t].size();
        int weight = ways[pathIdx].weight - nodeMetric(t);
        Node u = ways[pathIdx].pred;
        while (u != s) {
            nodes.push_back(u);
            ways = table[u].getList();
        std::cout << ways.size() << std::endl;
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
