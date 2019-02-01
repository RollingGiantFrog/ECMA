#ifndef INFOSOLUTION_H
#define INFOSOLUTION_H

#include <vector>
#include "node.h"

struct InfoSolution
{
	float solution;
	float bestbound;
	float t;
	bool optimal;
	float tWithoutClosing;
	char* method;
	vector<Node> nodes;
};

#endif