#ifndef __CONCURRENT_LSHDBSCAN_H__
#define __CONCURRENT_LSHDBSCAN_H__

#include <pthread.h>
#include <LSHDBSCAN.h>

class MergeTasksIdentificationTask
{
 public:
  HashTable & table;
  bool booked = false;
 MergeTasksIdentificationTask(HashTable & table_) : table(table_) {}
};

class CoreBucketIdentificationTask
{
 public:
  HashTable & table;
  bool booked = false;
 CoreBucketIdentificationTask(HashTable & table_) : table(table_) {}
};

class PopulationTask
{
 public:
  std::vector<point>::iterator begin, end;
  HashTable & table;
  bool booked = false;
  
 PopulationTask(std::vector<point>::iterator begin_,
		std::vector<point>::iterator end_,
		HashTable & table_) : begin(begin_),
    end(end_),
    table(table_)
    {}
};

class RelabelingTask
{
 public:
  std::vector<point>::iterator begin, end;
  bool booked = false;
  
 RelabelingTask(std::vector<point>::iterator begin_,
		std::vector<point>::iterator end_): begin(begin_), end(end_)
  {}
};

class ConcurrentLSHDBSCAN : public LSHDBSCAN
{
 private:
  pthread_t *pid;
  
  const size_t numberOfThreads;
  const size_t numberOfBatches;
  
  virtual void populateHashTables() override;
  virtual void identifyCoreBuckets() override;
  virtual void identifyMergeTasks() override;
  virtual void performMergeTasks() override;
  virtual void performRelabeling() override;

 public:
  void introduceMe() const;
  ConcurrentLSHDBSCAN(dataset*,
		      size_t numberOfHashTables_,
		      size_t numberOfHyperplanesPerTable_,
		      size_t numberOfThreads_,
		      size_t numberOfBatches_ = 2,
		      bool benchamrk_ = false);

  std::vector<PopulationTask> populationTasks;
  std::vector<RelabelingTask> relabelingTasks;
  std::vector<CoreBucketIdentificationTask> coreBucketIdentificationTasks;
  std::vector<MergeTasksIdentificationTask> mergeTasksIdentificationTasks;
  tbb::concurrent_vector<std::pair<std::pair<point*, point*>, bool>> mergeTasks;
  
  static void* populateHashTables_thread(void*);
  static void* identifyCoreBuckets_thread(void*);
  static void* identifyMergeTasks_thread(void*);
  static void* identifyAndPerformMergeTasks_thread(void*);
  static void* performMergeTasks_thread(void*);
  static void* performRelabelingTasks_thread(void*);
  
};

#endif
