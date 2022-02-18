#ifndef __LSHDBSCAN_H__
#define __LSHDBSCAN_H__

#include <point.h>
#include <randomGen.h>
#include <transformation.h>

#include <chrono>

class LSHDBSCAN
{
 protected:
  dataset* ds = NULL;
  size_t numberOfHashTables;
  size_t numberOfHyperplanesPerTable;
  std::vector<HashTable> hashTables;
  RandGenerator gen;
  const bool benchmark = false;

  void setDataset(dataset*);
  virtual void populateHashTables();
  virtual void identifyCoreBuckets();
  virtual void identifyMergeTasks();
  virtual void performMergeTasks();
  virtual void performRelabeling();
  
  void performGlobalMergeTasks();
  
  std::chrono::time_point<std::chrono::steady_clock> start, stop;
  std::chrono::duration<double> duration_initializingHashTables;
  std::chrono::duration<double> duration_populatingHashTables;
  std::chrono::duration<double> duration_identifyingCoreBuckets;
  std::chrono::duration<double> duration_identifyingMergeTasks;
  std::chrono::duration<double> duration_performingMergeTasks;
  std::chrono::duration<double> duration_relabelingData;
  
 public:
  LSHDBSCAN(dataset*,
	    size_t numberOfHashTables_,
	    size_t numberOfHyperplanesPerTable_,
	    bool benchamrk_ = false);
  LSHDBSCAN(dataset*,
	    std::vector<std::string> fileNames,
	    bool benchmark_ = false);

  void performClustering();
  std::ostream& getBenchmarkResults(std::ostream&, char deli) const;
};

#endif
