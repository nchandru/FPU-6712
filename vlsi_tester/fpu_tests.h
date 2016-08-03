#ifndef _FPU_TESTS_H_
#define _FPU_TESTS_H_

//------------------------------------------------------------------------------

// IC CONTROL OUTPUT BITS (on the shifter -- NOT the Arduino)
#define CLK   (32)
#define RSTn  (33)
#define CS    (34)
#define DIV   (35)
#define DOA   (36)
#define OPT   (40)

// BUS START BITS
#define DIN1  (0)
#define DIN2  (16)
#define MODE  (37)
#define DEBUG (22)
#define EXC   (19)

// IC CONTROL INPUT BITS (on the shifter -- NOT the Arduino)
#define DACK  (16)
#define DR    (17)
#define DOV   (18)
#define ABUSY (27)
#define MBUSY (28)

// BUS START BITS
#define DOUT  (0)

//------------------------------------------------------------------------------

#define DEBUG_WIDTH (5)
#define MODE_WIDTH  (3)
#define EXC_WIDTH   (3)
#define NO_DEBUG    (0b000)
#define DEBUG_ADDER (0b001)
#define DEBUG_MULT  (0b101)
#define OPT_ADD     (0)
#define OPT_MULT    (1)

//------------------------------------------------------------------------------

// set to true to reverse bit oder
    
#define DIN1_ORDER (false)
#define DIN2_ORDER (false)
#define DOUT_ORDER (false)

//------------------------------------------------------------------------------

bool run_test(uint16_t din1,
              uint16_t din2,
              uint16_t expected_val,
              uint16_t opt,
              uint16_t debug_mode,
              bool     check_exc,
              uint16_t exc_code,
              uint16_t max_cycles,
              char*    test_name,
              bool     verbose);

bool parallel_req_test(uint16_t din1,
                       uint16_t din2,
                       uint16_t max_cycles,
                       char*    test_name,
                       bool     verbose=false);

void pulse_clk();
void print_dut_inputs();
void print_dut_outputs();
void print_dut_state(int cycle, bool print_in, bool print_out, bool verbose);

//------------------------------------------------------------------------------

#endif
