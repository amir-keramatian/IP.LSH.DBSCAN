#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <vector>
#include <point.h>
#include <thread>

extern std::vector<point*> corePoints;

extern double (*distanceFunc)(const point & a, const point & b);
double angularDistance(const point & a, const point & b);
double squaredEuclideanDistance(const point & a, const point & b);
double euclideanDistance(const point & a, const point & b);

extern int (*hashFunc)(const point & a, const BasePoint & b);
int euclideanHash(const point & a, const BasePoint & b);
int angularHash(const point & a, const BasePoint & b);

extern double epsilon;
extern double epsilon_original;
extern size_t minPts;

enum Metric
  {
    euclidean,
    angular
  };

extern Metric metric;

struct Parameters
{
  std::string fileName;
  std::string baselineFileName;
  size_t numberOfHashTables = 10;
  size_t numberOfHyperplanesPerTable = 10;
  size_t numberOfThreads = std::thread::hardware_concurrency();
};

void parseTheArguments(int, char* argv[], Parameters&);
#endif
