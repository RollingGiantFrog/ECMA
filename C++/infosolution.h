#ifndef INFOSOLUTION_H
#define INFOSOLUTION_H

struct InfoSolution
{
	float solution;
	float bestbound;
	float t;
	bool optimal;
	float tWithoutClosing;
	char* method;
	
};

#endif