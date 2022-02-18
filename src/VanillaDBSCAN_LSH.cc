#include <VanillaDBSCAN_LSH.h>
#include <globals.h>
#include <cassert>

VanillaDBSCAN_LSH::VanillaDBSCAN_LSH(dataset* ds_,
				     size_t numberOfHashTables_,
				     size_t numerOfHyperplanesPerTable_)
{
  this->ds = ds_;
  this->numberOfHashTables = numberOfHashTables_;
  this->numerOfHyperplanesPerTable = numerOfHyperplanesPerTable_;

  for (size_t i = 0; i < numberOfHashTables; i++)
    {
      hashTables.emplace_back(ds,
			      numerOfHyperplanesPerTable,
			      &gen);
			      
    }

  for (auto& hashTable : hashTables)
    hashTable.populateHashTable();
}

std::vector<point*> VanillaDBSCAN_LSH::getEpsNeighbours(point &query)
{
  std::unordered_set<point*> result_set;

  for (auto& hashTable : hashTables)
    {
      std::vector<point*> temp = hashTable.getEpsNeighbours(query);
      result_set.insert(temp.begin(), temp.end());
    }
  std::vector<point*> result;
  result.reserve(result_set.size());

  std::copy(result_set.begin(), result_set.end(), std::back_inserter(result));

  /*
  std::cout << result.size() << std::endl;
  for (auto elem : result)
    {
      std::cout << elem->id << ",";
    }
  std::cout << std::endl;
  */
  
  return result;
}

void VanillaDBSCAN_LSH::performClustering()
{
  size_t j = 0;
  
  for (auto & p : ds->points)
    {
      if (p.processed == false)
	{
	  p.processed = true;
	  std::vector<point*> pointsToExplore = getEpsNeighbours(p);

	  if (pointsToExplore.size() >= minPts)
	    {
	      p.setAsCorePoint();
	      p.setLabel(++j);

	      while(!pointsToExplore.empty())
		{
		  point *q = pointsToExplore.back();
		  pointsToExplore.pop_back();

		  if (q->isNoise())
		    q->setLabel(j);		  

		  if (q->processed == false)
		    {
		      q->processed = true;
		      std::vector<point*> neighbours_of_q = getEpsNeighbours(*q);

		      if (neighbours_of_q.size() >= minPts)
			{
			  q->setAsCorePoint();

			  for (auto kirNeighb : neighbours_of_q)
			    {
			      if (kirNeighb->isNoise())
				kirNeighb->setLabel(j);
			      if (kirNeighb->processed == false)
				pointsToExplore.push_back(kirNeighb);
			    }
			}
		    }
		}
	    }
	}
    }
}
