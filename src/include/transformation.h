#ifndef __TRANSFORMATION_H__
#define __TRANSFORMATION_H__

#include <point.h>
#include <randomGen.h>
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_vector.h"

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

class MyHasher
{
 public:
  size_t operator() (HashedPoint const & p) const noexcept
  {
    std::size_t seed = 0;

    for (const auto & elem : p.features)
      hash_combine(seed, std::hash<int>() (elem)); 
      
    //    std::size_t h = std::hash<std::string>() (strRep);

    return seed;
  }
};

/*
class MyHasher
{
 public:
  size_t operator() (HashedPoint const & p) const noexcept
  {
    std::string strRep = "";

    for (const auto & elem : p.features)
      strRep +=std::to_string((int)elem);
    //    std::cout << strRep << std::endl;
      
    std::size_t h = std::hash<std::string>() (strRep);

    return h;
  }
};
*/

class CoreBucket
{
 public:
  std::vector<point*> members;
  point* representative = NULL;
};

class HashTable
{
 private:
  dataset* ds = NULL;
  RandGenerator* gen;
  
  void initializeHashTable(size_t);
  void initializeHashTable(std::string&);
  
  std::ostream & print(std::ostream & stream, char deli) const;
  std::vector<HashedPoint> hashedPoints;

  void identifyMergeTasks();
  size_t numberOfHyperplanes;
public:
  HashTable(dataset*, size_t, RandGenerator*);
  HashTable(dataset*, std::string&);
  
  std::vector<Hyperplane> hyperplanes;
  tbb::concurrent_unordered_map<HashedPoint, tbb::concurrent_vector<point*>, MyHasher> myMap;

  std::vector<CoreBucket> coreBuckets;
  std::vector<std::pair<point *, point *>> mergeTasks;

  void populateHashTable();
  void identifyCoreBuckets();
  void identifyCoreBuckets_densityStyle();
  void identifyMergeTasks_V2();
  void identifyMergeTasks_V2(tbb::concurrent_vector<std::pair<std::pair<point*, point*>, bool>>&);
  void identifyAndPerformMergeTasks();
  void performMergeTasks();
  size_t getMedian(std::vector<point*> &) const;

  void populateHashTable(std::vector<point>::iterator, std::vector<point>::iterator);
  size_t getClosestToMean(std::vector<point*> & vec) const;
  
  std::ostream& printTable(std::ostream & stream, char deli) const;
  std::ostream& printCoreBuckets(std::ostream&, char) const;
  std::ostream& printMergeTasks(std::ostream & stream, char deli) const;

  std::vector<point*> getEpsNeighbours(point &);/* used in ValillaDBSCAN+LSH */
  std::vector<point*> getEpsNeighbours(point &, HashedPoint &);/* used in ValillaDBSCAN+LSH */
};

#endif
