#ifndef PATH_H
#define PATH_H

#include <vector>
#include "instance.h"

using namespace std;

struct PathNode {
	Node node;
	int weight;
	float devWeight;
	float dev;
};

bool nodeCompare(PathNode node1, PathNode node2);

struct PathEdge {
	Node node1;
	Node node2;
	int dist;
	float dev;
};

bool edgeCompare(PathEdge edge1, PathEdge edge2);

class Path {
public:
	Path(const Instance& instance, vector<Node> path);

	const Instance& instance;
	vector<Node> path;

	bool isValid;

	int weight;
	float worstWeight;

	int dist;
	float worstDist;

	int length;

	vector<PathNode> nodes;
	vector<PathEdge> edges;
};

#endif
