#ifndef __RANDOM_GEN_H__
#define __RANDOM_GEN_H__

#include <random>

class RandGenerator
{
 private:
  std::default_random_engine engine;
  std::normal_distribution<double>* dist;
 public:
  RandGenerator();
  double getRandVal();
};


#endif
