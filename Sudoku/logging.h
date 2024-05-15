#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <chrono>
#include "variables.h"


void log_headers(string fName) {

  std::ofstream outputfile(fName, std::ios_base::app);

  outputfile  << std::setw(10) << "VALID SOLUTION" << "\t"
              << std::setw(10) << "DIFFICULTY" << "\t"
              << std::setw(10) << "DIMENSION" << "\t"
              << std::setw(10) << "TOTAL CELLS" << "\t"
              << std::setw(10) << "THREAD COUNT" << "\t"
              << std::setw(25) << "MILLISECONDS" << "\t"
              << std::endl;
}

void log_results(string fName, double difficulty, 
                std::chrono::high_resolution_clock::time_point start, 
                std::chrono::high_resolution_clock::time_point end, bool result) {

  // Logging
  std::ofstream outputfile(fName, std::ios_base::app);

  // Calculate times
  std::chrono::duration<double, std::ratio<3600>> hours = (end - start);
  std::chrono::duration<double, std::ratio<60>> minutes = (end - start);
  std::chrono::duration<double> seconds = (end - start);
  std::chrono::duration<double, std::milli> milli = (end - start);
  

  outputfile  << std::setw(10) << result << "\t"
              << std::setw(10) << difficulty << "\t"
              << std::setw(10) << DIMENSION << "\t"
              << std::setw(10) << DIMENSION * DIMENSION << "\t"
              << std::setw(10) << THREAD_COUNT << "\t"
              << std::setw(25) << milli.count() << "\t"
              << std::setw(15) << static_cast<int>(hours.count()) << ":" << std::setfill('0')
              << std::setw(2) << static_cast<int>(minutes.count()) % 60 << ":"
              << std::setw(2) << static_cast<int>(seconds.count()) % 60 << "."
              << std::setw(3) << static_cast<int>(milli.count()) % 1000 << std::setfill(' ')
              << std::endl;

}