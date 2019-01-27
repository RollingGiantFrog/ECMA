#ifndef SHORTEST_CAPACITED_PATH_H_INCLUDED
#define SHORTEST_CAPACITED_PATH_H_INCLUDED

#include <iostream>
#include <algorithm>
#include <vector>
#include <assert.h>

#include "instance.h"
#include "path.h"
#include "sorted_list.h"

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
            std::vector<Node> order;
            unsigned int countDone = 0;
            bool done[instance.n] = {false};
            bool inOrder[instance.n] = {false};
            std::vector< std::vector<Node> > sccs = sconnect(instance.neighbors, initNode);
            for (std::vector< std::vector<Node> >::reverse_iterator it1 = sccs.rbegin(); it1 != sccs.rend(); ++it1) {
                std::vector<Node>& scc = (*it1);
                for (std::vector<Node>::reverse_iterator it2 = scc.rbegin(); it2 != scc.rend(); ++it2) {
                    order.push_back(*it2);
                }
            }

            for (unsigned int i = 0; i < order.size(); ++i) inOrder[order[i]] = true;
            for (int u = 0; u < instance.n; ++u)
                if (!inOrder[u]) order.push_back(u);

            while (instance.n > countDone) {
                for (unsigned int i = 0; i < order.size(); ++i) {
                    Node u = order[i];
                    if (!done[u]) {
                        done[u] = true;
                        countDone = countDone + 1;
                        for (std::list<Pathway>::iterator it = table[u].ways.begin(); it != table[u].ways.end(); ++it) {
                            Pathway way = (*it);
                            for (unsigned int k = 0; k < instance.neighbors[u].size(); ++k) {
                                Node v = instance.neighbors[u][k];
                                int weight = way.weight + nodeMetric(v);
                                if (weight <= instance.S) {
                                    if (table[v].addValue(weight,way.value + edgeMetric(u,v),u)) {
                                        if (done[v]) {
                                            done[v] = false;
                                            countDone = countDone - 1;
                                        }
                                    }
                                }
                                iter = iter + 1;
                                //if ((iter % 200000) == 0) std::cout << "Nodes to process : " << instance.n - countDone << std::endl;
                            }
                        }
                    }
                }
            }
        }

        else {
            std::vector<Node> order;
            unsigned int countDone = 0;
            bool done[instance.n] = {false};
            bool inOrder[instance.n] = {false};
            std::vector< std::vector<Node> > sccs = sconnect(instance.predecessors, initNode);
            for (std::vector< std::vector<Node> >::reverse_iterator it1 = sccs.rbegin(); it1 != sccs.rend(); ++it1) {
                std::vector<Node>& scc = (*it1);
                for (std::vector<Node>::reverse_iterator it2 = scc.rbegin(); it2 != scc.rend(); ++it2) {
                    order.push_back(*it2);
                }
            }

            for (unsigned int i = 0; i < order.size(); ++i) inOrder[order[i]] = true;
            for (int u = 0; u < instance.n; ++u)
                if (!inOrder[u]) order.push_back(u);

            while (instance.n > countDone) {
                for (unsigned int i = 0; i < order.size(); ++i) {
                    Node u = order[i];
                    if (!done[u]) {
                        done[u] = true;
                        countDone = countDone + 1;
                        for (std::list<Pathway>::iterator it = table[u].ways.begin(); it != table[u].ways.end(); ++it) {
                            Pathway way = (*it);
                            for (unsigned int k = 0; k < instance.predecessors[u].size(); ++k) {
                                Node v = instance.predecessors[u][k];
                                int weight = way.weight + nodeMetric(v);
                                if (weight <= instance.S) {
                                    if (table[v].addValue(weight,way.value + edgeMetric(v,u),u)) {
                                        if (done[v]) {
                                            done[v] = false;
                                            countDone = countDone - 1;
                                        }
                                    }
                                }
                                iter = iter + 1;
                                if ((iter % 200000) == 0) std::cout << "Nodes to process : " << instance.n - countDone << std::endl;
                            }
                        }
                    }
                }
            }
        }

        if (!table[endNode].empty()) {
            //shortestPath = extractPathNodes(initNode,endNode,table[endNode].size()-1);
            //lightestPath = extractPathNodes(initNode,endNode,0);
        }
    }

    Path extractPathNodes(Node s, Node t, unsigned int pathIdx) const {
        assert(!table[t].empty());

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
