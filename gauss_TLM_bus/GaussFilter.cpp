#include <cmath>
#include <iomanip>

#include "GaussFilter.h"

GaussFilter::GaussFilter(sc_module_name n)
    : sc_module(n), t_skt("t_skt"), base_offset(0) {
  SC_THREAD(do_filter);

  t_skt.register_b_transport(this, &GaussFilter::blocking_transport);
}

GaussFilter::~GaussFilter() = default;

void GaussFilter::do_filter() {
  { wait(CLOCK_PERIOD, SC_NS); }
// std::cout << "do filter" << std::endl;
  while (true) {
// std::cout << "in loop" << std::endl;
    unsigned int cnt = 0;
    r_val = 0; g_val = 0; b_val = 0;
    for (unsigned int v = 0; v < MASK_Y; ++v) {
      for (unsigned int u = 0; u < MASK_X; ++u) {
// std::cout << "read i_r, i_g, i_b" << std::endl;
        unsigned char r_value = i_r.read();
        unsigned char g_value = i_g.read();
        unsigned char b_value = i_b.read();
        wait(CLOCK_PERIOD, SC_NS);
        if (r_value != 0 && g_value != 0 && b_value !=0 ) cnt+=mask[u][v];
        r_val += r_value * mask[u][v];
        g_val += g_value * mask[u][v];
        b_val += b_value * mask[u][v];        
        wait(CLOCK_PERIOD, SC_NS);
      }
    }
// std::cout << "write" << std::endl;
    o_result_r.write(r_val / cnt);    
// std::cout << "write" << std::endl;
    o_result_g.write(g_val / cnt);
// std::cout << "write" << std::endl;
    o_result_b.write(b_val / cnt);
  }
}

void GaussFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  wait(delay);
  sc_dt::uint64 addr = payload.get_address();
  addr = addr - base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case GAUSS_FILTER_RESULT_ADDR:
// std::cout << "read R, G, B" << std::endl;
      buffer.uc[0] = (char)(o_result_r.read());
      buffer.uc[1] = (char)(o_result_g.read());
      buffer.uc[2] = (char)(o_result_b.read());
      buffer.uc[3] = 0;
      break;
    default:
      buffer.uc[0] = 0;
      buffer.uc[1] = 0;
      buffer.uc[2] = 0;
      buffer.uc[3] = 0;     
      std::cerr << "Error! GaussFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
// std::cout << "save in data_ptr" << std::endl;
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];
    break;

  case tlm::TLM_WRITE_COMMAND:
    switch (addr) {
    case GAUSS_FILTER_R_ADDR:
// std::cout << "sent in fifo" << std::endl;
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);
      }
      if (mask_ptr[1] == 0xff) {
        i_g.write(data_ptr[1]);
      }
      if (mask_ptr[2] == 0xff) {
        i_b.write(data_ptr[2]);
      }
      break;
    default:
      std::cerr << "Error! GaussFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    break;

  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
