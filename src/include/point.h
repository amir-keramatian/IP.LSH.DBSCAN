#ifndef __POINT_H__
#define __POINT_H__

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

#define NOISE -1
#define NON_NOISE -2

class BasePoint
{
  bool normalized = false;
 public:
  size_t id = -1;
  std::vector<double> features;

  BasePoint();
  BasePoint(std::istringstream &, size_t id_ = -1);
  BasePoint(std::vector<double> vec)
    {
      std::copy(vec.begin(), vec.end(), std::back_inserter(features));
    }

  void normalize(bool auxDim = true);

  BasePoint& operator+=(const BasePoint& rhs);
  BasePoint& operator-=(const BasePoint& rhs);
  BasePoint& operator/=(const double& rhs);
  
  double norm() const;
  double innerProduct(const BasePoint&) const;
  BasePoint operator+(const BasePoint & rhs) const;
  double squaredEuclideanDistance(const BasePoint &) const;
  std::ostream & print(std::ostream & stream, char deli) const;
};

class Hyperplane : public BasePoint
{
 public:
  using BasePoint::BasePoint;
  Hyperplane(){}
};

class point : public BasePoint
{
 private:
  point* parent = NULL;
  int label = NOISE;
  bool corePoint = false;
  
 public:
  using BasePoint::BasePoint;

  bool processed = false; /*used in vanilla DBSCAN*/
  void setLabel(size_t);  /*used in vanilla DBSCAN*/
  bool isNoise();  /*used in vanilla DBSCAN*/
  point* findRoot();
  const point* cFindRoot() const;
  void unsafe_compress();
  void link(point*);
  void reLabel();
  void reset();
  bool isCore();
  void setAsCorePoint();
  std::ostream & print(std::ostream & stream, char deli, bool onlyLabel = false) const;
};

class HashedPoint : public BasePoint
{
 public:
  std::vector<int> features;
  bool operator == (const HashedPoint & p) const;
};
/*
namespace std
{
  template<> struct hash<HashedPoint>
  {
    std::size_t operator() (HashedPoint const & p) const noexcept
    {
      std::ostringstream concat;
      concat.precision(1);

      for (const auto & elem : p.features)
	concat << elem << ",";
      //std::cout << concat.str() << std::endl;
      
      std::size_t h = std::hash<std::string>() (concat.str());

      return h;
    }
  };
}
*/
class dataset
{
 public:
  size_t numberOfDimensions = 0;  
  std::vector<point> points;
  
  void readData(std::string);
  void relabelData();
  void resetData();
  void normalizeData();
  void meanRemoveData();
  std::ostream & printData(std::ostream & stream, char deli, bool onlyLabel = false) const;
};


#endif
