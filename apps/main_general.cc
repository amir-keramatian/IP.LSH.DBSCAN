#include <ConcurrentLSHDBSCAN.h>
#include <point.h>
#include <globals.h>

#define BENCHMARK true
size_t numberOfBatches = 100;

int main(int argc, char* argv[])
{
  Parameters params;
  parseTheArguments(argc, argv, params);

  dataset d;
  auto start = std::chrono::steady_clock::now();
  d.readData(params.fileName);
  auto stop = std::chrono::steady_clock::now();  
  std::chrono::duration<double> duration = (stop - start);
  std::cout << "Reading data took: " << duration.count() << std::endl;

  if (metric == angular)
    {
      d.normalizeData();      //d.meanRemoveData();
    }

  ConcurrentLSHDBSCAN lshdbscan(&d,
				params.numberOfHashTables,
				params.numberOfHyperplanesPerTable,
				params.numberOfThreads,
				numberOfBatches,
				BENCHMARK);
  lshdbscan.introduceMe();
  std::cout << "Performing concurrent clustering...\n";
  lshdbscan.performClustering();


  std::ofstream benchmarkStr("benchmark.txt", std::ofstream::out | std::ofstream::app);
  lshdbscan.getBenchmarkResults(benchmarkStr, '\t') << std::endl;
  benchmarkStr.close();

  std::cout << "Saving clustering labels..." << std::endl;
  std::string outputFile = params.fileName
    + "_"
    + std::to_string(params.numberOfHashTables)
    + "_"
    + std::to_string(params.numberOfHyperplanesPerTable)
    + "_"
    + std::to_string(params.numberOfThreads)
    + ".idx_concurrentlshdbscan";

  std::ofstream labelStr(outputFile);	  
  d.printData(labelStr, '\t', true);
  labelStr.close();

  if (!params.baselineFileName.empty())
    {
      std::cout << "Computing clustering accuracy against baseline: "
		<< params.baselineFileName
		<< " ..."
		<< std::endl;
      std::string accuracyCommand = std::string("./computeAccuracy_fast ")
	+ outputFile + " "
	+ params.baselineFileName + " "
	+ "accuracy.txt";

      system(accuracyCommand.c_str());
    }

  return 0;
}
