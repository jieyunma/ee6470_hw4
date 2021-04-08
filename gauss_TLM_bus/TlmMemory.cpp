/**********************************************************************
  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2008 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 3.0 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.
 *********************************************************************/

/* derived from TLM examples by Imperas Software Ltd */

#include "TlmMemory.h"

using namespace std;
using namespace sc_core;

/// Constructor

TlmMemory::TlmMemory(sc_dt::uint64 high_address // memory size (bytes)
                     ,
                     unsigned int memory_width // memory width (bytes)
                     )
    : m_high_address(high_address), m_memory_width(memory_width) {
  m_memory = new unsigned char[size_t(m_high_address + 1)];
  memset(m_memory, 0, size_t(m_high_address + 1));
}

unsigned int
TlmMemory::operation(tlm::tlm_generic_payload &gp,
                     sc_core::sc_time &delay_time ///< transaction delay
) {
  sc_dt::uint64 address = gp.get_address();    // memory address
  tlm::tlm_command command = gp.get_command(); // memory command
  unsigned char *data = gp.get_data_ptr();     // data pointer
  unsigned int length = gp.get_data_length();  // data length

  tlm::tlm_response_status response_status = check_address(gp);

  if (gp.get_byte_enable_ptr()) {
    gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
  } else if (gp.get_streaming_width() != gp.get_data_length()) {
    gp.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
  }

  switch (command) {
  default: {
    gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    break;
  }

  case tlm::TLM_WRITE_COMMAND: {
    if (response_status == tlm::TLM_OK_RESPONSE) {
      for (unsigned int i = 0; i < length; i++) {
        m_memory[address++] = data[i]; // move the data to memory
      }
    }
    break;
  }

  case tlm::TLM_READ_COMMAND: {
    if (response_status == tlm::TLM_OK_RESPONSE) {
      for (unsigned int i = 0; i < length; i++) {
        data[i] = m_memory[address++]; // move the data to memory
      }
    }
    break;
  }
  } // end switch
  gp.set_dmi_allowed(true);
  gp.set_response_status(response_status);

  return length;
} // end memory_operation

unsigned char *TlmMemory::get_mem_ptr(void) { return m_memory; }

//==============================================================================
///  @fn memory::check_address
//
///  @brief Method to check if the gp is in the address range of this memory
//
///  @details
///    This routine used to check for errors in address space
//
//==============================================================================
tlm::tlm_response_status
TlmMemory::check_address(tlm::tlm_generic_payload &gp) {
  sc_dt::uint64 address = gp.get_address();   // memory address
  unsigned int length = gp.get_data_length(); // data length

  if ((address < 0) || (address > m_high_address)) {
    return tlm::TLM_ADDRESS_ERROR_RESPONSE; // operation response
  } else {
    if ((address + length - 1) > m_high_address) {
      return tlm::TLM_ADDRESS_ERROR_RESPONSE; // operation response
    }
    return tlm::TLM_OK_RESPONSE;
  }
} // end check address

// tshsu: begin
#include <cstring>
void TlmMemory::memory_load(const unsigned int begin, unsigned int size,
                            const char *const file_name) {
  // cout<<"External memory loading begins"<<endl;
  ifstream input;
  char *data_buffer;
  data_buffer = new char[size];
  memset(data_buffer, 0, sizeof(*data_buffer) * size); // fill 0 to data_buffer

  input.open(file_name, ios::in | ios::binary);
  if (!input) {
    cerr << "memory::load(): Could not open input file:" << file_name << endl;
    cerr << "Simulation aborted.";
    exit(1);
  } else if (begin + size > get_size()) {
    cerr << "TlmMemory::load(): " << begin + size << " is out of the bound"
         << endl;
    cerr << "Simulation aborted.";
    exit(1);
  } else {
    input.read(data_buffer, size);
    if (!input) {
      cerr << "TlmMemory::load(): only " << input.gcount() << " could be read"
           << endl;
    }
    input.close();
  }
  // Assume it's little endian. Big endian need to implement.
  memcpy(get_mem_ptr() + begin, data_buffer, size);
  delete data_buffer;
}
void TlmMemory::memory_dump(const unsigned int begin, unsigned int size,
                            const char *const file_name) {
  // cout<<"External memory dumping begins"<<endl;
  ofstream output;
  output.open(file_name, ios::out | ios::binary);
  if (!output) {
    cerr << "TlmMemory::dump(): Could not open output file:" << file_name
         << endl;
    cerr << "Simulation aborted.";
    exit(1);
  } else if (begin + size > get_size()) {
    cerr << "TlmMemory::dump(): " << begin + size << " is out of the bound"
         << endl;
    cerr << "Simulation aborted.";
    exit(1);
  } else {
    // Assume it's little endian. Big endian need to implement.
    output.write(reinterpret_cast<const char *>(get_mem_ptr() + begin), size);
    output.close();
  }
}
// tshsu: end

SC_HAS_PROCESS(ram);

ram::ram(sc_core::sc_module_name module_name // module name
         ,
         const char *memory_socket // socket name
         ,
         sc_dt::uint64 high_address // memory size (bytes)
         ,
         unsigned int memory_width // memory width (bytes)
         )
    : sc_module(module_name) // init module name
      ,
      t_skt(memory_socket) // init socket name
      ,
      m_high_address(high_address) // init memory size (bytes-1)
      ,
      m_memory_width(memory_width) // init memory width (bytes)
      ,
      m_target_memory(high_address, memory_width) {
  t_skt.register_b_transport(this, &ram::custom_b_transport);
  t_skt.register_transport_dbg(this, &ram::debug_transport);
  t_skt.register_get_direct_mem_ptr(this, &ram::get_direct_mem_ptr);
}

void ram::custom_b_transport(tlm::tlm_generic_payload &payload,
                             sc_core::sc_time &delay_time) {
  sc_core::wait(delay_time);
  sc_core::sc_time mem_op_time;
  m_target_memory.operation(payload, mem_op_time);
  return;
}

unsigned int ram::debug_transport(tlm::tlm_generic_payload &payload) {
  sc_core::sc_time mem_op_time;
  return m_target_memory.operation(payload, mem_op_time);
}

bool ram::get_direct_mem_ptr(tlm::tlm_generic_payload &trans,
                             tlm::tlm_dmi &dmi_data) {
  sc_dt::uint64 address = trans.get_address();
  if (address > m_high_address) {
    // should not happen
    cerr << "ram::get_direct_mem_ptr: address overflow";
    return false;
  }

  // if (m_invalidate) m_invalidate_dmi_event.notify(m_invalidate_dmi_time);
  dmi_data.allow_read_write();
  dmi_data.set_start_address(0x0);
  dmi_data.set_end_address(m_high_address);

  unsigned char *ptr = m_target_memory.get_mem_ptr();
  dmi_data.set_dmi_ptr(ptr);
  return true;
}

// tshsu
void ram::memory_load(const unsigned int begin, unsigned int size,
                      const char *const file_name) {
  getMemory()->memory_load(begin, size, file_name);
}
void ram::memory_dump(const unsigned int begin, unsigned int size,
                      const char *const file_name) {
  getMemory()->memory_dump(begin, size, file_name);
}
