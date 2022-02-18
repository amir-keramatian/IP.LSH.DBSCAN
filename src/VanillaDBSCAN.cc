#include <VanillaDBSCAN.h>
#include <point.h>
#include <globals.h>
#include <cassert>

void VanillaDBSCAN(dataset* ds)
{
  std::cout << "Performing Vanilla DBSCAN" << std::endl;
  size_t j = 0;
  
  for (auto & p : ds->points)
    {
      if (p.processed == false)
	{
	  p.processed = true;
	  std::vector<point*> pointsToExplore = getEpsNeighbours(ds->points, p);

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
		      std::vector<point*> neighbours_of_q = getEpsNeighbours(ds->points, *q);

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

std::vector<point*> getEpsNeighbours(std::vector<point> & points, point& query)
{
  std::vector<point*> result;
  
  for (auto & p : points)
    {
      //std::cout << "p.id: " << p.id << "\t\t query.id: " << query.id << std::endl;
      if ( (distanceFunc(query, p) <= epsilon)
	   //	   && (p.id != query.id)
	   )
	{
	  result.push_back(&p);
	}
    }
  //std::cout << "size: " << result.size() << std::endl;
  return result;
}
