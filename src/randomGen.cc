#include <randomGen.h>

RandGenerator::RandGenerator()
{
  dist = new std::normal_distribution<double>(0.0, 1.0);
}

double RandGenerator::getRandVal()
{
  return (*dist)(engine);
}
