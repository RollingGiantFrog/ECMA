#include "preprocess_nodes.h"

#include <set>
#include <list>

#include <iostream>

std::set<Node> preprocessNodes(const Instance& instance, float supBound, Node s, Node t,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_s,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_t) {

    std::set<Node> removedNodes;

    for (unsigned int i = 0; i < instance.n; ++i) {
        if (i == s || i == t) continue;

        if (staticSCP_s.table[i].ways.empty() || staticSCP_t.table[i].ways.empty())
            removedNodes.insert(i);

        else {
            bool broken = false;
            for (std::list<Pathway>::iterator it1 = staticSCP_s.table[i].ways.begin(); it1 != staticSCP_s.table[i].ways.end(); ++it1) {
                Pathway way_s = (*it1);
                for (std::list<Pathway>::iterator it2 = staticSCP_t.table[i].ways.begin(); it2 != staticSCP_t.table[i].ways.end(); ++it2) {
                    Pathway way_t = (*it2);
                    if (way_s.weight + way_t.weight - instance.p[i] + std::min(2,instance.d2)*instance.ph[i] < instance.S
                        && way_s.value + way_t.value < supBound) {
                        broken = true;
                        break;
                    }
                    if (broken) break;
                }
            }
            if (!broken) removedNodes.insert(i);
        }
    }
    return removedNodes;
}

std::set<Edge> preprocessEdges(const Instance& instance, float supBound, Node s, Node t,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_s,
                          ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_t) {

    std::set<Edge> removedEdges;
    for (unsigned int i = 0; i < instance.n; ++i) {
        for (unsigned int j = 0; j < instance.n; ++j) {
            if (!instance.adj[i][j] || i == s || j == t) continue;

            if (staticSCP_s.table[i].ways.empty() || staticSCP_t.table[j].ways.empty())
                removedEdges.insert(std::make_pair(i,j));

            else {
                bool broken = false;
                for (std::list<Pathway>::iterator it1 = staticSCP_s.table[i].ways.begin(); it1 != staticSCP_s.table[i].ways.end(); ++it1) {
                    Pathway way_s = (*it1);
                    for (std::list<Pathway>::iterator it2 = staticSCP_t.table[j].ways.begin(); it2 != staticSCP_t.table[j].ways.end(); ++it2) {
                        Pathway way_t = (*it2);
                        int ph_min = std::min(instance.ph[i],instance.ph[j]);
                        int ph_max = std::max(instance.ph[i],instance.ph[j]);
                        int dev_max = min(2, instance.d2);
                        int dev_min = min(2, instance.d2-dev_max);
                        if (way_s.weight + way_t.weight + dev_max*ph_max + dev_min*ph_min < instance.S
                            && way_s.value + way_t.value + instance.d[i][j]*(1. + instance.D[i][j]) < supBound) {
                                broken = true;
                                break;
                            }
                    }
                    if (broken) break;
                }
                if (!broken) removedEdges.insert(std::make_pair(i,j));
            }
        }
    }
    return removedEdges;
}

Instance preprocessInstance(Instance instance) {
    bool modified = true;

    ShortestCapacitedPath<SemiWorstCaseNodeMetric,SemiWorstCaseEdgeMetric> semiWorstCaseSCP(instance, instance.s, instance.t, SemiWorstCaseNodeMetric(instance), SemiWorstCaseEdgeMetric(instance), false);
    Path shortestPath = semiWorstCaseSCP.extractPathNodes(instance.s,instance.t,semiWorstCaseSCP.table[instance.t].size()-1);
    float supBound = shortestPath.worstDist;

    while (modified) {
        ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_s(instance, instance.s, instance.t, StaticNodeMetric(instance), StaticEdgeMetric(instance), false);
        ShortestCapacitedPath<StaticNodeMetric,StaticEdgeMetric> staticSCP_t(instance, instance.t, instance.s, StaticNodeMetric(instance), StaticEdgeMetric(instance), true);

        std::set<Node> removedNodes = preprocessNodes(instance,supBound,instance.s,instance.t,staticSCP_s,staticSCP_t);
        std::set<Edge> removedEdges = preprocessEdges(instance,supBound,instance.s,instance.t,staticSCP_s,staticSCP_t);

        float c = (float) instance.m;

        modified = (removedNodes.size() + removedEdges.size()) > 0;
        instance = instance.restrict(removedNodes,removedEdges);

        //cout << "Remaining nodes : " << instance.n << endl;
        //cout << "Remaining edges : " << instance.m << " (" << (int) (100*(removedEdges.size()/c)) << "% removed edges)" << endl;
    }

    return instance;
}
