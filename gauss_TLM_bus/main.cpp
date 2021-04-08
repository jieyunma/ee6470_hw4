#include <iostream>
#include <string>
using namespace std;

// Wall Clock Time Measurement
#include <sys/time.h>

#include "SimpleBus.h"
#include "GaussFilter.h"
#include "Testbench.h"
#include "TlmMemory.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

// int main(int argc, char *argv[])
int sc_main(int argc, char **argv) {
  if ((argc < 3) || (argc > 4)) {
    cout << "No arguments for the executable : " << argv[0] << endl;
    cout << "Usage : >" << argv[0] << " in_image_file_name out_image_file_name"
         << endl;
    return 0;
  }
  Testbench tb("tb");
  SimpleBus<1, 2> bus("bus");
  ram ram("ram", "t_sck", RAM_MM_SIZE - 1);
  bus.set_clock_period(sc_time(CLOCK_PERIOD, SC_NS));
  GaussFilter gauss_filter("gauss_filter");
  tb.initiator.i_skt(bus.t_skt[0]);
  bus.setDecode(0, GAUSS_MM_BASE, GAUSS_MM_BASE + GAUSS_MM_SIZE - 1);
  bus.i_skt[0](gauss_filter.t_skt);
  bus.setDecode(1, RAM_MM_BASE, RAM_MM_BASE + RAM_MM_SIZE - 1);
  bus.i_skt[1](ram.t_skt);

  tb.read_bmp(argv[1]);
  sc_start();
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  ram.memory_dump(0, tb.get_file_size(), argv[2]);

  return 0;
}
