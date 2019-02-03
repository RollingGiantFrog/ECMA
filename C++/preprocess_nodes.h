#ifndef PREPROCESS_NODES_H_INCLUDED
#define PREPROCESS_NODES_H_INCLUDED

#include "node.h"
#include "instance.h"
#include "shortest_capacited_path.h"
#include <set>
#include <utility>

// Preprocess the nodes of an instance, using a feasible solution of value supBound to eliminate nodes
// It also uses the exact best paths for the static problem from s to any node and from any node to t
// The best is the feasible solution, the more nodes will be eliminated
std::set<Node> preprocessNodes(const Instance& instance, float supBound, Node s, Node t,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_s,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_t);

// Preprocess the edges of an instance, using a feasible solution of value supBound to eliminate edges
// It also uses the exact best paths for the static problem from s to any node and from any node to t
// The best is the feasible solution, the more edges will be eliminated
std::set<Edge> preprocessEdges(const Instance& instance, float supBound, Node s, Node t,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_s,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_t);

// Preprocess fully an instance, i.e eliminating all possible nodes and edges until there is nothing left to remove
// It uses the semi worst case metric to compute a feasible solution
Instance preprocessInstance(Instance instance);
#endif // PREPROCESS_NODES_H_INCLUDED
