#ifndef __VANILLA_DBSCAN__
#define __VANILLA_DBSCAN__

#include <point.h>
#include <globals.h>

void VanillaDBSCAN(dataset*);
std::vector<point*> getEpsNeighbours(std::vector<point> &, point&);

#endif
