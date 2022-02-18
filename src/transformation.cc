#include <algorithm>
#include <iomanip>
#include <cassert>
#include <transformation.h>
#include <globals.h>
#include <cmath>

HashTable::HashTable(dataset* ds_,
		     size_t numberOfHyperplanes_,
		     RandGenerator* gen_)
{
  this->ds  = ds_;
  this->gen = gen_;
  numberOfHyperplanes = numberOfHyperplanes_;
  this->initializeHashTable(numberOfHyperplanes_);
}

HashTable::HashTable(dataset* ds_,
		     std::string& fileName)
{
  this->ds = ds_;
  initializeHashTable(fileName);
}

void HashTable::populateHashTable()
{
  assert(ds != NULL);

  for (auto & point : ds->points)
    {
      HashedPoint hashedPoint;

      std::transform(hyperplanes.cbegin(), hyperplanes.cend(),
		     std::back_inserter(hashedPoint.features),
		     [&point](const Hyperplane & h)->double
		     {
		       return hashFunc(point, h);
		     });
      myMap[hashedPoint].push_back(&point);
    }
}

void HashTable::populateHashTable(std::vector<point>::iterator begin,
				  std::vector<point>::iterator end)
{
  assert((end - begin) > 0);

  for (std::vector<point>::iterator pointIter = begin;
       pointIter < end;
       pointIter++)
    {
      HashedPoint hashedPoint; hashedPoint.features.reserve(numberOfHyperplanes);

      std::transform(hyperplanes.cbegin(), hyperplanes.cend(),
		     std::back_inserter(hashedPoint.features),
		     [pointIter](const Hyperplane & h)->int
		     {
		       return hashFunc(*pointIter, h);
		     });
      myMap[hashedPoint].push_back(&(*pointIter));
    }
}

void HashTable::identifyCoreBuckets()
{
  for (auto & element : myMap)
    {
      if (element.second.size() >= minPts)
	{
	  std::vector<point*> coreBucketElements;
	  coreBucketElements.reserve(element.second.size());

	  std::copy(element.second.begin(), element.second.end(),
		    std::back_inserter(coreBucketElements));

	  point* core = coreBucketElements[getMedian(coreBucketElements)];
	  size_t cnt = 0;
	  
	  std::for_each(coreBucketElements.begin(),
			coreBucketElements.end(),
			[core, this, &cnt](point* p)
			{
			  if ( (p->findRoot() == p) &&
			       (distanceFunc(*core, *p)  <= epsilon)
			       )
			    {
			      p->link(core);
			      cnt++;
			    }
			}
			);

	  if (cnt >= minPts)
	    {
	      core->setAsCorePoint();
	      
	      CoreBucket coreBucket;
	      coreBucket.representative = core;
	      coreBucket.members = coreBucketElements;
	      coreBuckets.push_back(coreBucket);
	    }
	}
    }
}

size_t HashTable::getMedian(std::vector<point*> & vec) const
{
  assert(vec.size() != 0);
  std::vector<double> projVals;
  projVals.reserve(vec.size());

  Hyperplane ones;
  ones.features.resize(vec[0]->features.size(), 1.0);

  std::transform(vec.begin(), vec.end(),
		 std::back_inserter(projVals),
		 [&ones](const point* p)->double
		 {
		   return p->innerProduct(ones);
		 }
		 );

  std::vector<size_t> indices;
  
  for (size_t i = 0; i < vec.size(); i++)
    {
      indices.push_back(i);
    }
  
  std::sort(indices.begin(), indices.end(),
	    [&projVals](size_t leftIndex, size_t rightIndex)
	    {
	      return projVals[leftIndex] < projVals[rightIndex];
	    }
	    );

  return indices[vec.size()/2];
}

void HashTable::identifyMergeTasks()
{
  for (const auto & coreBucket : coreBuckets)
    {
      std::vector<point *> toGetMerged;

      for (const auto element : coreBucket.members)
	{
	  if (element->isCore())
	    {
	      toGetMerged.push_back(element);
	    }
	}

      assert(toGetMerged.size() >= 1);
      
      point* corePoint_0 = toGetMerged.back();
      toGetMerged.pop_back();
      
      while (toGetMerged.size() > 0)
	{
	  point* corePoint = toGetMerged.back();
	  toGetMerged.pop_back();

	  mergeTasks.push_back({corePoint_0, corePoint});
	}
    }
}

void HashTable::identifyMergeTasks_V2()
{
  for (const auto & coreBucket : coreBuckets)
    {
      std::vector<point *> toGetMerged;

      for (const auto element : coreBucket.members)
	{
	  if (element->isCore())
	    {
	      toGetMerged.push_back(element);
	    }
	}

      assert(toGetMerged.size() >= 1);

      size_t i, j;

      for ( i = 0; i < toGetMerged.size(); i++)
	{
	  for ( j = i+1; j < toGetMerged.size(); j++)
	    {
	      if (distanceFunc(*toGetMerged[i], *toGetMerged[j]) <= epsilon)
		{
		  mergeTasks.push_back({toGetMerged[i], toGetMerged[j]});
		}
	    }
	}
    }
}

void HashTable::identifyMergeTasks_V2(tbb::concurrent_vector<std::pair<std::pair<point*, point*>, bool>> &globalMergeTasks)
{
  for (auto & coreBucket : coreBuckets)
    {
      point* core = coreBucket.representative; assert(core != NULL);
      std::vector<point *> toGetMerged;

      for (const auto element : coreBucket.members)
	{
	  if (element->isCore())
	    {
	      if ( (distanceFunc(*core, *element) <= epsilon)
		   &&
		   (core->findRoot() != element->findRoot())
		   )
		globalMergeTasks.push_back({{&*core, &*element}, false});
	    }
	}
    }
}

void HashTable::identifyAndPerformMergeTasks()
{
  for (auto & coreBucket : coreBuckets)
    {
      point* core = coreBucket.representative; assert(core != NULL);

      for (const auto element : coreBucket.members)
	{
	  if ( element->isCore() ) 
	    {
	      if ( (core->findRoot() != element->findRoot())
		   &&
		   (distanceFunc(*core, *element) <= epsilon)
		   )
		element->link(core);
	    }
	}
    }
}


void HashTable::initializeHashTable(size_t numberOfHyperplanes_)
{
  for (size_t i = 0; i < numberOfHyperplanes_; i++)
    {
      Hyperplane h;

      for (size_t d = 0; d < ds->numberOfDimensions; d++)
	{
	  h.features.push_back(gen->getRandVal());
	}

      h.normalize(false);/*without aux dimension*/
      hyperplanes.push_back(h);
    }
}

void HashTable::initializeHashTable(std::string& fileName)
{
  std::string line;
  
  std::ifstream f(fileName);

  if (!f)
    {
      std::cerr << "Could not initialize hash table from file" << std::endl;
      exit(EXIT_FAILURE);
    }
  
  while (std::getline(f, line))
    {
      std::istringstream strStream(line);
      Hyperplane h(strStream);
      hyperplanes.push_back(h);

      if (h.features.size() != ds->numberOfDimensions)
	{
	  std::cerr << "Error while reading hyperplanes from file: "
		    << "Inconsistent dimensionality between points and hyperplanes.\n";
	  exit(EXIT_FAILURE);
	}
    }
}

void HashTable::performMergeTasks()
{
  for (const auto & task : mergeTasks)
    {
      task.first->link(task.second);
    }
}

std::ostream & HashTable::print(std::ostream & stream, char deli) const
{
  std::cout << "Hashed dataset's size is: " << hashedPoints.size() << std::endl;

  for ( const auto & p : this->hashedPoints)
    p.print(stream, deli) << std::endl;

  return stream;
}

std::ostream& HashTable::printCoreBuckets(std::ostream& stream, char deli) const
{
  size_t num = 0;
  for (const auto & coreBucket : coreBuckets)
    {
      stream << "Corebucket " << std::setw(2) << num++ << ":" << deli;
      for (const auto & element : coreBucket.members)
	{
	  stream << std::setw(3) << element->id
		 << std::setw(3) << (element->isCore() ? "(c) " : " ");
	}
      stream << std::endl;
    }
  return stream;
}

std::ostream& HashTable::printTable(std::ostream& stream, char deli) const
{
  size_t num = 0;

  for (const auto & element : myMap)
    {
      stream << "Bucket " << std::setw(2) << num++ << ":" << std::endl;

      for (const auto & p : element.second)
	{
	  p->print(stream << '\t', ' ') << std::endl;
	}	  
    }
  return stream;
}

std::ostream& HashTable::printMergeTasks(std::ostream& stream, char deli) const
{
  size_t num = 0;

  for (const auto & element : mergeTasks)
    {
      stream << "Merge task:" << deli
	     << std::setw(4)  << element.first->id << "&"
	     << std::setw(4)  << element.second->id << std::endl;
    }
  return stream;
}

void HashTable::identifyCoreBuckets_densityStyle()
{
  for (auto & element : myMap)
    {
      if (element.second.size() >= minPts)
	{
	  std::vector<point*> candidates, coreBucketElements;
	  candidates.reserve(element.second.size()); coreBucketElements.reserve(element.second.size());

	  std::copy(element.second.begin(), element.second.end(),
		    std::back_inserter(candidates));

	  point* core = candidates[getClosestToMean(candidates)];
	  //point* core = candidates[getMedian(candidates)]; #alternative
	  size_t cnt = 0;
	  
	  std::for_each(candidates.begin(),
			candidates.end(),
			[core, this, &cnt, &coreBucketElements](point* p)
			{
			  if (distanceFunc(*core, *p)  <= epsilon)
			    {
			      cnt++;
			      coreBucketElements.push_back(p);
			    }
			}
			);
	  
	  if (cnt >= minPts)
	    {
	      CoreBucket coreBucket;
	      coreBucket.representative = core;
	      coreBucket.members = coreBucketElements;
	      
	      core->setAsCorePoint();
	      std::for_each(coreBucketElements.begin(),
			    coreBucketElements.end(),
			    [core, this, &cnt](point* p)
			    {
			      if ( (p->findRoot() == p) )
				{
				  p->link(core);
				}
			    }
			    );
	      coreBuckets.push_back(coreBucket);
	    }
	}
    }
}

size_t HashTable::getClosestToMean(std::vector<point*> & vec) const
{
  assert(vec.size() != 0);

  point mean(std::vector<double> (ds->numberOfDimensions, 0));

  for (const point * p : vec)
    mean += *p;
  mean /= vec.size();

  std::vector<double> distanceToMean;
  distanceToMean.reserve(vec.size());

  std::transform(vec.begin(), vec.end(),
		 std::back_inserter(distanceToMean),
		 [&mean](const point* p)->double
		 {
		   return distanceFunc(*p, mean);
		 }
		 );

  size_t result = 0;
  double smallestVal = distanceToMean[0];

  for (size_t i = 1; i < vec.size(); ++i)
    {
      if (distanceToMean[i] < smallestVal)
	{
	  smallestVal = distanceToMean[i];
	  result = i;
	}
    }
  return result;
}

std::vector<point*> HashTable::getEpsNeighbours(point &query) /* used in ValillaDBSCAN+LSH */
{
  std::vector<point*> result;

  HashedPoint hashedPoint;

  std::transform(hyperplanes.cbegin(), hyperplanes.cend(),
		 std::back_inserter(hashedPoint.features),
		 [&query](const Hyperplane & h)->double
		 {
		   return hashFunc(query, h);
		 });

  
  return getEpsNeighbours(query, hashedPoint);
}


std::vector<point*> HashTable::getEpsNeighbours(point &query, HashedPoint &hashedPoint) /* used in ValillaDBSCAN+LSH */
{
  std::vector<point*> result;

  for (auto neighbour : myMap[hashedPoint])
    {
      if (distanceFunc(query, *neighbour) <= epsilon)
	{
	  result.push_back(neighbour);
	}
    }
  
  return result;
}
