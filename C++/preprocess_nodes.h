#ifndef PREPROCESS_NODES_H_INCLUDED
#define PREPROCESS_NODES_H_INCLUDED

#include "node.h"
#include "instance.h"
#include "shortest_capacited_path.h"
#include <set>
#include <utility>

std::set<Node> preprocessNodes(const Instance& instance, float supBound, Node s, Node t,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_s,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_t);

std::set<Edge> preprocessEdges(const Instance& instance, float supBound, Node s, Node t,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_s,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_t);

Instance preprocessInstance(Instance instance);
#endif // PREPROCESS_NODES_H_INCLUDED
