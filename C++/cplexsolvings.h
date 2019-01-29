#include "instance.h"
#include "path.h"
#include "infosolution.h"

InfoSolution dualization(const Instance& instance,  int TimeLim);
InfoSolution dualization(const Instance& instance,  int TimeLim, Path path);
/*
InfoSolution cuttingplanes(const Instance& instance,  int TimeLim);
InfoSolution cuttingplanes(const Instance& instance,  int TimeLim, Path path);

InfoSolution branchandcut(const Instance& instance,  int TimeLim);
InfoSolution branchandcut(const Instance& instance,  int TimeLim, Path path);*/