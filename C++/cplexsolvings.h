#include "instance.h"
#include "path.h"
#include "infosolution.h"

// Resolution du problème par dualisation du probleme robuste
InfoSolution dualization(const Instance& instance,  int TimeLim);
// Meme algorithme avec en entrée une solution réalisable dans path
InfoSolution dualization(const Instance& instance,  int TimeLim, const Path& path);

// Résolution du problème par plans coupants
InfoSolution cuttingplanes(const Instance& instance,  int TimeLim);
// Meme algorithme avec en entrée une solution réalisable dans path
InfoSolution cuttingplanes(const Instance& instance,  int TimeLim, const Path& path);

// Résolution du problème par branch and cut
InfoSolution branchandcut(const Instance& instance,  int TimeLim);
// Meme algorithme avec en entrée une solution réalisable dans path
InfoSolution branchandcut(const Instance& instance,  int TimeLim, const Path& path);

double get_time();
