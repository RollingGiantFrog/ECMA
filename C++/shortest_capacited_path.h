#ifndef SHORTEST_CAPACITED_PATH_H_INCLUDED
#define SHORTEST_CAPACITED_PATH_H_INCLUDED

#include "instance.h"
#include "path.h"
#include "sorted_list.h"

class NodeMetric {
public:
    NodeMetric() {};
    virtual double operator() (int node) = 0;
};

class EdgeMetric {
public:
    EdgeMetric() {};
    virtual double operator() (int node1, int node2) = 0;
};

class ShortestCapacitedPath {
public:
    const Instance& instance;
    const NodeMetric* nodeMetric;
    const EdgeMetric* edgeMetric;

    std::vector<SortedList> table;
    unsigned int iter;

    Path shortestPath;
    Path lightestPath;

    ShortestCapacitedPath(const Instance& instance, int initNode, int endNode, NodeMetric* nodeMetric, EdgeMetric* edgeMetric, bool reverse = false);
};

#endif // SHORTEST_CAPACITED_PATH_H_INCLUDED
