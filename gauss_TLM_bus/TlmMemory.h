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

/* derived from TLM examples memory.cpp by Imperas Software Ltd */

#ifndef __OVP_RAM_H__
#define __OVP_RAM_H__

#include <fstream>
#include <tlm> // TLM headers
#include <tlm_utils/simple_target_socket.h>

class TlmMemory {
public:
  TlmMemory(sc_dt::uint64 high_address // memory size (bytes-1)
            ,
            unsigned int memory_width // memory width (bytes)
  );

  unsigned int operation(tlm::tlm_generic_payload &gp // TLM2 GP reference
                         ,
                         sc_core::sc_time &delay_time // transaction delay
  );

  void get_delay(tlm::tlm_generic_payload &gp // TLM2 GP reference
                 ,
                 sc_core::sc_time &delay_time // time to be updated
  );

  unsigned char *get_mem_ptr(void);

  // tshsu: begin
  sc_dt::uint64 get_size() { return m_high_address - 1; }
  void memory_load(const unsigned int begin, unsigned int size,
                   const char *const file_name);
  void memory_dump(const unsigned int begin, unsigned int size,
                   const char *const file_name);
  // tshsu: end

private:
  tlm::tlm_response_status check_address(tlm::tlm_generic_payload &gp);
  sc_dt::uint64 m_high_address; // memory size (bytes-1)
  unsigned int m_memory_width;  // memory width (bytes)
  unsigned char *m_memory;      // memory
};

class ram : public sc_core::sc_module // inherit from SC module base clase

{
public:
  ram(sc_core::sc_module_name module_name // SC module name
      ,
      const char *memory_socket // socket name
      ,
      sc_dt::uint64 high_address // memory size (bytes-1)
      ,
      unsigned int memory_width = 4 // memory width (bytes)
  );

private:
  void custom_b_transport(tlm::tlm_generic_payload &payload,
                          sc_core::sc_time &delay_time);
  unsigned int debug_transport(tlm::tlm_generic_payload &payload);
  bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans,
                          tlm::tlm_dmi &dmi_data);

public:
  tlm_utils::simple_target_socket<ram> t_skt;
  TlmMemory *getMemory() { return &m_target_memory; }
  // tshsu
  void memory_load(const unsigned int begin, unsigned int size,
                   const char *const file_name);
  void memory_dump(const unsigned int begin, unsigned int size,
                   const char *const file_name);

private:
  sc_dt::uint64 m_high_address;
  unsigned int m_memory_width;
  TlmMemory m_target_memory;
};

#endif
