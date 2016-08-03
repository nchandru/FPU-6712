#include "assert_handler.h"
#include "fpu_tests.h"
#include "shift_io.h"

//------------------------------------------------------------------------------

/////////////////////////////////////////////////////////
// DO NOT MAKE MORE THAN ONE INSTANCE OF THESE OBJECTS //
/////////////////////////////////////////////////////////

InputData*  input_data  = NULL;
OutputData* output_data = NULL;

//------------------------------------------------------------------------------

// SHIFT IN PINS
#define DSOUT (4)
#define PLBAR (5)
#define CP    (6)

// SHIFT OUT PINS
#define DSIN  (7)
#define OEBAR (8)
#define STCP  (9)
#define SHCP  (10)
#define MRBAR (11)

//------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println("========================================");
  Serial.println("Begin VLSI Tester");

  input_data  = new InputData(DSOUT, PLBAR, CP);
  output_data = new OutputData(DSIN, OEBAR, STCP, SHCP, MRBAR);

  bool verbose = false;

  run_test(0b0100000000110000, // DIN1
           0b0100000010110000, // DIN2
           0b0100000100000100, // expected value
           OPT_ADD,            // operation
           DEBUG_ADDER,        // debug mode
           false,              // test for exceptions
           0,                  // expected exception code
           100,                // max cycles
           "adder_01",         // test name
           verbose);           // verbose output

  run_test(0b1100000000110000,
           0b0100000010110000,
           0b0100000000110000,
           OPT_ADD,
           DEBUG_ADDER,
           false,
           0,
           100,
           "adder_02",
           verbose);

  run_test(0b0100000000110000,
           0b1100000010110000,
           0b1100000000110000,
           OPT_ADD,
           DEBUG_ADDER,
           false,
           0,
           100,
           "adder_03",
           verbose);

  run_test(0b0100000000100000,
           0b1100011101010110,
           0b1100011100101010,
           OPT_ADD,
           DEBUG_ADDER,
           false,
           0,
           100,
           "adder_04",
           verbose);

  run_test(0b0100000000110000,
           0b0100000010110000,
           0b0100000101110010,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_01",
           verbose);

  run_test(0b1100000000110000,
           0b0100000010110000,
           0b1100000101110010,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_02",
           verbose);

  run_test(0b1100000000110000,
           0b1100000010110000,
           0b0100000101110010,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_03",
           verbose);

  run_test(0b0000000001000000,
           0b0110000001000000,
           0b0010000101100000,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_04",
           verbose);

  run_test(0b0111111110000000,
           0b0000000000000000,
           0b0111111110000001,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_05",
           verbose);

  // both infinite
  run_test(0b0111111110000000,
           0b0111111110000000,
           0b0111111110000000,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_06",
           verbose);

  // overflow
  run_test(0b0111111101000000,
           0b0111111101000000,
           0b0111111110000000,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_07",
           verbose);

  // NaN
  run_test(0b0111111110000001,
           0b0111111101000000,
           0b0111111110000001,
           OPT_MULT,
           DEBUG_MULT,
           false,
           0,
           100,
           "mult_08",
           verbose);

    parallel_req_test(0b1100000000110000,
                    0b0100000010110000,
                    100,
                    "par_req",
                    verbose);

  // exceptions tests
  run_test(0b0111111110000000,
           0b0000000000000000,
           0b0000000000000000,
           OPT_ADD,
           NO_DEBUG,
           true,
           3,
           100,
           "add_exc",
           verbose);

  run_test(0b0111111110000000,
           0b0000000000000000,
           0b0000000000000000,
           OPT_MULT,
           NO_DEBUG,
           true,
           4,
           100,
           "mult_exc_01",
           verbose);

  run_test(0b0111111110000000,
           0b0111111110000000,
           0b0000000000000000,
           OPT_MULT,
           NO_DEBUG,
           true,
           3,
           100,
           "mult_exc_02",
           verbose);

  run_test(0b0111111101000000,
           0b0111111101000000,
           0b0000000000000000,
           OPT_MULT,
           NO_DEBUG,
           true,
           2,
           100,
           "mult_exc_03",
           verbose);

  Serial.println("End VLSI Tester");
  Serial.println("========================================\n");
}

void loop()
{
  delay(10);
}

//------------------------------------------------------------------------------
