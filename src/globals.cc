#include <cmath>
#include <cassert>
#include <globals.h>
#include <unistd.h>

double epsilon_original = 0.001;
double epsilon = epsilon_original * epsilon_original;
size_t minPts = 500;

std::vector<point*> corePoints;

Metric metric = euclidean;
int (*hashFunc)(const point & a, const BasePoint & b) = &euclideanHash;
double (*distanceFunc)(const point & a, const point & b) = &euclideanDistance;

double angularDistance(const point & a, const point & b)
{
  double result = a.innerProduct(b);

  result = result / (a.norm()*b.norm());

  if(result > 1)
    {
      assert( (result - 1) < 0.00001);
      result = 1;
    }
  
  result = acos(result);

  result = result / 3.14;
  
  return result;
}

double squaredEuclideanDistance(const point & a, const point & b)
{
  return a.squaredEuclideanDistance(b);
}

double euclideanDistance(const point & a, const point & b)
{
  return sqrt(a.squaredEuclideanDistance(b));
}

int euclideanHash(const point & a, const BasePoint & b)
{
  double result = a.innerProduct(b);
  result = result / epsilon_original;
  result = std::floor(result);
  return static_cast<int> (result);
}

int angularHash(const point & a, const BasePoint & b)
{
  return (a.innerProduct(b) >= 0 ? 1.0 : -1.0);
}

void parseTheArguments(int argc, char* argv[], Parameters& params)
{
  int opt;

  while ( (opt=getopt(argc, argv, "am:e:t:f:L:M:b:")) != -1  )
    {
      switch(opt)
	{
	case 'a':
	  metric = angular;
	  hashFunc = angularHash;
	  distanceFunc = angularDistance;
	  break;
	case 'm':
	  minPts = std::stoi(optarg);
	  break;
	case 'e':
	  epsilon_original = std::stod(optarg);
	  epsilon = epsilon_original;
	  break;
	case 't':
	  params.numberOfThreads = std::stoi(optarg);
	  break;
	case 'f':
	  params.fileName = std::string(optarg);
	  break;
	case 'L':
	  params.numberOfHashTables = std::stoi(optarg);
	  break;
	case 'M':
	  params.numberOfHyperplanesPerTable = std::stoi(optarg);
	  break;
	case 'b':
	  params.baselineFileName = std::string(optarg);
	  break;
	default:
	  std::cout << "Usage: " << argv[0] << " "
		    << "-f inputFile "
		    << "[-L #HashTables] "
		    << "[-M #HyperPlanesPerHashTable] "
		    << "[-t #threads] "
		    << "[-e eps] " 
		    << "[-m minPts] "
		    << "[-a angularDistance] "
		    << "[-b baselineClusteringFile]"
		    << std::endl;

	  exit(EXIT_FAILURE);
	}
    }
  if (params.fileName.empty())
    {
      std::cerr << "-f inputFile is mandatory" << std::endl;
      exit(EXIT_FAILURE);
    }
  if (metric == euclidean)
    {
      epsilon = epsilon_original * epsilon_original;
      distanceFunc = squaredEuclideanDistance;
    }
}

