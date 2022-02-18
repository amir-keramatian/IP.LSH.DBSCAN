#include <LSHDBSCAN.h>
#include <globals.h>
#include <cassert>

LSHDBSCAN::LSHDBSCAN(dataset* ds_,
		     size_t numberOfHashTables_,
		     size_t numberOfHyperplanesPerTable_,
		     bool benchmark_) : benchmark(benchmark_)
{
  setDataset(ds_);
  this->numberOfHashTables = numberOfHashTables_;
  this->numberOfHyperplanesPerTable = numberOfHyperplanesPerTable_;

  start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < numberOfHashTables; i++)
    {      
      hashTables.emplace_back(ds,
			      numberOfHyperplanesPerTable,
			      &gen);
    }
  stop = std::chrono::steady_clock::now();
  duration_initializingHashTables = stop - start;
}

LSHDBSCAN::LSHDBSCAN(dataset* ds_,
		     std::vector<std::string> fileNames,
		     bool benchmark_) : benchmark(benchmark_)
{
  setDataset(ds_);
  this->numberOfHashTables = fileNames.size();

  start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < numberOfHashTables; i++)
    {
      hashTables.emplace_back(ds,
			      fileNames[i]);
    }
  stop = std::chrono::steady_clock::now();
  duration_initializingHashTables = stop - start;
}

void LSHDBSCAN::setDataset(dataset* d)
{
  this->ds = d;
  if (ds == NULL)
    {
      std::cerr << "Invalid Dataset." << std::endl;
      exit(EXIT_FAILURE);
    }
}

void LSHDBSCAN::populateHashTables()
{
  for (auto & hashTable : hashTables)
    hashTable.populateHashTable();
}

void LSHDBSCAN::identifyCoreBuckets()
{
  for (auto & hashTable : hashTables)
    {
      //hashTable.identifyCoreBuckets();
      hashTable.identifyCoreBuckets_densityStyle();
      //      hashTable.printCoreBuckets(std::cout, '\t');
    }
}

void LSHDBSCAN::identifyMergeTasks()
{
  for (auto & hashTable : hashTables)
    hashTable.identifyMergeTasks_V2();
}

void LSHDBSCAN::performMergeTasks()
{
  for (auto & hashTable : hashTables)
    {
      hashTable.performMergeTasks();
    }
}

void LSHDBSCAN::performRelabeling()
{
  ds->relabelData();
}


void LSHDBSCAN::performClustering()
{
  ds->resetData();
  if (benchmark == false)
    {
      populateHashTables();
      identifyCoreBuckets();
      identifyMergeTasks();
      performMergeTasks();
      performRelabeling();
    }
  else
    {
      start = std::chrono::steady_clock::now();
      populateHashTables();
      stop  = std::chrono::steady_clock::now();
      duration_populatingHashTables = stop - start;
      
      start = std::chrono::steady_clock::now();
      identifyCoreBuckets();
      stop  = std::chrono::steady_clock::now();
      duration_identifyingCoreBuckets = stop - start;

      start = std::chrono::steady_clock::now();
      identifyMergeTasks();
      stop  = std::chrono::steady_clock::now();
      duration_identifyingMergeTasks = stop - start;
      
      start = std::chrono::steady_clock::now();
      performMergeTasks();
      stop  = std::chrono::steady_clock::now();
      duration_performingMergeTasks = stop - start;

      start = std::chrono::steady_clock::now();
      performRelabeling();
      stop  = std::chrono::steady_clock::now();
      duration_relabelingData = stop - start;
    }
}

std::ostream& LSHDBSCAN::getBenchmarkResults(std::ostream& stream, char deli) const
{
  if (benchmark == false)
    stream << "Benchmarking was not activated!" << std::endl;

  else 
    stream << duration_initializingHashTables.count() << deli
	   << duration_populatingHashTables.count()   << deli
	   << duration_identifyingCoreBuckets.count() << deli
	   << duration_identifyingMergeTasks.count()  << deli
	   << duration_performingMergeTasks.count()   << deli
	   << duration_relabelingData.count()         << deli;

  return stream;
}

void LSHDBSCAN::performGlobalMergeTasks()
{
  std::cout << "Performing Global Merge Tasks\n";
  for (auto & core_i : corePoints)
    {
      for (auto & core_j : corePoints)
	{
	  if (core_i->id != core_j->id)
	    {
	      if (distanceFunc(*core_i, *core_j) <= epsilon)
		{
		  core_i->link(core_j);
		}
	    }
	}
    }
}
