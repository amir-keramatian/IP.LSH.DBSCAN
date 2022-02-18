#ifndef __VANILLA_DBSCAN_LSH__
#define __VANILLA_DBSCAN_LSH__

#include <unordered_set>
#include <point.h>
#include <randomGen.h>
#include <transformation.h>

namespace std
{
  template<>
    struct hash<point>
    {
      std::size_t operator() (point const & p) const noexcept
      {
	std::string strRep =std::to_string(p.id);
      
	std::size_t h = std::hash<std::string>() (strRep);

	return h;
      }
    };
}

class VanillaDBSCAN_LSH
{
 public:
  dataset* ds = NULL;
  size_t numberOfHashTables;
  size_t numerOfHyperplanesPerTable;
  std::vector<HashTable> hashTables;
  RandGenerator gen;

  VanillaDBSCAN_LSH(dataset*,
		    size_t numberOfHashTables_,
		    size_t numerOfHyperplanesPerTable_);

  std::vector<point*> getEpsNeighbours(point &query);

  void performClustering();
};

#endif
