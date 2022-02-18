
#include <VanillaDBSCAN.h>
#include <globals.h>

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

  VanillaDBSCAN(&d);

  std::cout << "Saving clustering labels..." << std::endl;
  std::string outputFile = params.fileName
    + "_"
    + ".idx_vanilla";

  std::ofstream labelStr(outputFile);	  
  d.printData(labelStr, '\t', true);
  labelStr.close();

  
  /*
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
  */
  return 0;
}
