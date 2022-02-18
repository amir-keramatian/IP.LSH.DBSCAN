#include <ConcurrentLSHDBSCAN.h>
#include <cassert>
#include <iostream>
#include <pthread.h>
#include <globals.h>
ConcurrentLSHDBSCAN::ConcurrentLSHDBSCAN(dataset* ds_,
					 size_t numberOfHashTables_,
					 size_t numberOfHyperplanesPerTable_,
					 size_t numberOfThreads_,
					 size_t numberOfBatches_,
					 bool benchamrk_) :
  numberOfThreads((assert(numberOfThreads_>=1), numberOfThreads_)),
  numberOfBatches((assert(numberOfBatches_>=1), numberOfBatches_)),
  LSHDBSCAN(ds_,
	    numberOfHashTables_,
	    numberOfHyperplanesPerTable_,
	    benchamrk_)
{
  pid = new pthread_t[numberOfThreads];

  populationTasks.reserve(numberOfBatches*numberOfHashTables);
  relabelingTasks.reserve(numberOfBatches);
  coreBucketIdentificationTasks.reserve(numberOfHashTables);
  mergeTasksIdentificationTasks.reserve(numberOfHashTables);
  
  std::vector<point>::iterator begin, end;
  size_t batchSize = round(this->ds->points.size()/numberOfBatches);
    
  begin = this->ds->points.begin();
  end = begin + batchSize;
  
  for (size_t batch = 0; batch < numberOfBatches; ++batch)

    {
      if ( batch == numberOfBatches - 1)
	end = this->ds->points.end();
      
      for ( size_t table = 0; table < numberOfHashTables; ++table)
	{
	  populationTasks.emplace_back(begin, end, hashTables[table]);
	}
      relabelingTasks.emplace_back(begin, end);
      begin += batchSize;
      end += batchSize;
    }

  for ( size_t table = 0; table < numberOfHashTables; ++table)
    {
      coreBucketIdentificationTasks.emplace_back(hashTables[table]);
      mergeTasksIdentificationTasks.emplace_back(hashTables[table]);
    }
  assert(populationTasks.size() == numberOfBatches*numberOfHashTables);
}

void ConcurrentLSHDBSCAN::populateHashTables()
{
  std::cout << "Populating the concurrent way!" << std::endl;

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_create(&pid[threadID], NULL, populateHashTables_thread, this);
    }

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_join(pid[threadID], NULL);
    }
}

void ConcurrentLSHDBSCAN::identifyCoreBuckets()
{
  std::cout << "Identifying core-buckets the concurrent way!" << std::endl;

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_create(&pid[threadID], NULL, identifyCoreBuckets_thread, this);
    }

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_join(pid[threadID], NULL);
    }
}

void ConcurrentLSHDBSCAN::identifyMergeTasks()
{
  std::cout << "Identifying and Performing the merge tasks the concurrent way!" << std::endl;

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_create(&pid[threadID], NULL, identifyAndPerformMergeTasks_thread, this);
      //pthread_create(&pid[threadID], NULL, identifyMergeTasks_thread, this);
    }

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_join(pid[threadID], NULL);
    }

  //std::cout << "mergeTasks.size(): " << mergeTasks.size() << std::endl;
}

void ConcurrentLSHDBSCAN::performMergeTasks()
{
  return ; //Remove return if mergeTasks are not performed in function identifyMergeTasks()


  std::cout << "Performing the mergeTasks the concurrent way!" << std::endl;

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_create(&pid[threadID], NULL, performMergeTasks_thread, this);
    }

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_join(pid[threadID], NULL);
    }
}

void ConcurrentLSHDBSCAN::performRelabeling()
{
  std::cout << "Performing concurrent relabeling!" << std::endl;

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_create(&pid[threadID], NULL, performRelabelingTasks_thread, this);
    }

  for (size_t threadID = 0; threadID < numberOfThreads; ++threadID)
    {
      pthread_join(pid[threadID], NULL);
    }
}

void* ConcurrentLSHDBSCAN::populateHashTables_thread(void *inputArg)
{
  auto input = (ConcurrentLSHDBSCAN*) inputArg;
  
  for ( auto & task : input->populationTasks)
    {
      if (task.booked == false)
	{
	  if (__sync_bool_compare_and_swap ( &(task.booked),
					     false,
					     true)
	      )
	    {
	      task.table.populateHashTable(task.begin, task.end);
	    }
	}
    }  
  return NULL;
}

void* ConcurrentLSHDBSCAN::identifyCoreBuckets_thread(void *inputArg)
{
  auto input = (ConcurrentLSHDBSCAN*) inputArg;
  
  for ( auto & task : input->coreBucketIdentificationTasks)
    {
      if (task.booked == false)
	{
	  if (__sync_bool_compare_and_swap ( &(task.booked),
					     false,
					     true)
	      )
	    {
	      task.table.identifyCoreBuckets_densityStyle();
	    }
	}
    }  
  return NULL;
}

void* ConcurrentLSHDBSCAN::identifyAndPerformMergeTasks_thread(void *inputArg)
{
  auto input = (ConcurrentLSHDBSCAN*) inputArg;
  
  for ( auto & task : input->mergeTasksIdentificationTasks)
    {
      if (task.booked == false)
	{
	  if (__sync_bool_compare_and_swap ( &(task.booked),
					     false,
					     true)
	      )
	    {
	      //task.table.identifyMergeTasks_V2(input->mergeTasks);
	      task.table.identifyAndPerformMergeTasks();
	    }
	}
    }  
  return NULL;
}

void* ConcurrentLSHDBSCAN::performMergeTasks_thread(void *inputArg)
{
  ConcurrentLSHDBSCAN* input = (ConcurrentLSHDBSCAN*) inputArg;

  for (auto & task : input->mergeTasks)
    {
      if(task.second == false)
	{
	  if (__sync_bool_compare_and_swap ( &(task.second),
					     false,
					     true)
	      )
	    {
	      task.first.first->link(task.first.second);
	    }
	}
    }

  return NULL;
}

void* ConcurrentLSHDBSCAN::performRelabelingTasks_thread(void *inputArg)
{
  auto input = (ConcurrentLSHDBSCAN*) inputArg;
  
  for ( auto & task : input->relabelingTasks)
    {
      if (task.booked == false)
	{
	  if (__sync_bool_compare_and_swap ( &(task.booked),
					     false,
					     true)
	      )
	    {
	      for (auto iter = task.begin; iter < task.end; iter++)
		{
		  iter->reLabel();
		}
	    }
	}
    }  
  return NULL;
}


void ConcurrentLSHDBSCAN::introduceMe() const
{
  std::cout << "Concurrent LSHDBSCAN:\n"
	    << "\t#points: " << this->ds->points.size()
	    << "\t#dims: " << this->ds->numberOfDimensions
    	    << "\tmetric: " << ( (metric == angular) ? "angular" : "euclidean")
	    << std::endl
	    << "\t#HashTables: " << numberOfHashTables
	    << "\t#HyperPlanesPerHashTable: " << numberOfHyperplanesPerTable
	    << "\t#threads: " << numberOfThreads
	    << "\t#batches: " << numberOfBatches
	    << std::endl
	    << "\tEps: " << epsilon_original
	    << "\tminPts: " << minPts
	    << std::endl;
}
