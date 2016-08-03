#include "fpu_tests.h"
#include "shift_io.h"

//------------------------------------------------------------------------------

extern InputData*  input_data;
extern OutputData* output_data;

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
              bool     verbose)
{
  bool success = false;
  int  cycle   = 0;

  if (verbose) {
    Serial.print("\n-- BEGIN ");
    Serial.println(test_name);
    Serial.println();
  }

  // setting the state to LOW enables shifter output
  output_data->SetOutputState(LOW);

  // initialize the output state
  output_data->ClearData();
  output_data->SetBit(CS,   1);
  output_data->SetBit(DIV,  0);
  output_data->SetBit(OPT,  opt);
  output_data->SetBit(RSTn, 1);

  // set the debug mode
  output_data->SetRange(MODE, MODE_WIDTH, debug_mode, false);
  output_data->Write();

  if (verbose) {
    Serial.println("Initial Data to DUT:");
    output_data->PrintBits(0, 5, true);
  }

  if (verbose) {
    Serial.println("\nInitial Data from DUT:");
    input_data->Read();
    input_data->PrintBits(0, 5, true);
    print_dut_inputs();
    Serial.println();

    Serial.println("RESETTING DUT\n");
  }

  // toggle the reset line
  output_data->SetBit(RSTn, 0);
  output_data->Write();
  pulse_clk();
  print_dut_state(++cycle, true, true, verbose);
  output_data->SetBit(RSTn, 1);
  output_data->Write();
  pulse_clk();
  print_dut_state(++cycle, true, true, verbose);

  // TODO: triple-check that the operands are being set correctly
  output_data->SetRange(DIN1, 16, din1, DIN1_ORDER);
  output_data->SetRange(DIN2, 16, din2, DIN2_ORDER);
  output_data->SetBit(DIV, 1);
  output_data->Write();

  if (verbose) {
    Serial.println("APPLIED OPERANDS\n");

    input_data->Read();
    print_dut_state(cycle, true, true, verbose);
  }

  // pulse the DUT clock until DOV goes high, for up to 100 cycles
  while (cycle < max_cycles) {
    pulse_clk();
    input_data->Read();
    print_dut_state(++cycle, true, false, verbose);

    // clear DIV when ABUSY goes high
    if (input_data->GetBit(ABUSY)) {
      output_data->SetBit(DIV, 0);
      output_data->Write();
    }

    // break out of the loop when DOV is asserted
    if (input_data->GetBit(DOV)) {
      if (input_data->GetBit(DR) != 1) {
        Serial.println("\nWARNING: Data Ready was not asserted!\n");
      }

      pulse_clk();
      input_data->Read();
      print_dut_state(++cycle, true, true, verbose);

      break;
    }
  }

  // validate the result
  if (cycle < max_cycles) {
    if (verbose) {
      Serial.println("OPERATION IS COMPLETE!\n");
    }

    if (check_exc) {
      if (verbose) {
        Serial.println("CHECKING EXCEPTION CODE\n");
      }
      int exc = input_data->GetRange(EXC, EXC_WIDTH);
      if (exc ^ exc_code) {
        Serial.println("\nWARNING: EXCEPTION CODE DOES NOT MATCH!\n");
        Serial.println(exc);
      }
      else {
        if (verbose) {
          Serial.println("\EXCEPTION CODE MATCHES EXPECTED VALUE!\n");
        }
        success = true;
      }
    }
    else {
      int res = input_data->GetRange(0, 16);
      if (res ^ expected_val) {
        Serial.println("\nWARNING: RESULT DOES NOT MATCH EXPECTED VALUE!\n");
      }
      else {
        if (verbose) {
          Serial.println("\RESULT MATCHES EXPECTED VALUE!\n");
        }
        success = true;
      }
    }
  }
  else {
    Serial.println("WARNING: OPERATION WAS UNSUCCESSFUL\n");
  }

  if (verbose) {
    Serial.println("ACKNOWLEDGING RESULT\n");
  }

  // toggle the DOA line
  output_data->SetBit(DOA, 1);
  output_data->Write();
  pulse_clk();
  input_data->Read();
  print_dut_state(++cycle, true, true, verbose);

  output_data->SetBit(DOA, 0);
  output_data->Write();
  pulse_clk();
  input_data->Read();
  print_dut_state(++cycle, true, true, verbose);

  if (input_data->GetBit(ABUSY)) {
    Serial.println("\nWARNING: ABUSY should not be asserted!\n");
  }

  // disable the shift register output
  output_data->SetOutputState(HIGH);

  if (verbose) {
    Serial.print("-- END ");
    Serial.println(test_name);
    Serial.println();
  }
  else {
    Serial.print(" - ");
    Serial.print(test_name);
    if (success) {
      Serial.println(":\t PASS");
    }
    else {
      Serial.println(":\t FAIL");
    }
  }

  return success;
}

//------------------------------------------------------------------------------

bool parallel_req_test(uint16_t din1,
                       uint16_t din2,
                       uint16_t max_cycles,
                       char*    test_name,
                       bool     verbose)
{
  bool success = false;
  int  cycle   = 0;

  if (verbose) {
    Serial.print("\n-- BEGIN ");
    Serial.println(test_name);
    Serial.println();
  }

  // setting the state to LOW enables shifter output
  output_data->SetOutputState(LOW);

  // initialize the output state
  output_data->ClearData();
  output_data->SetBit(CS,   1);
  output_data->SetBit(DIV,  0);
  output_data->SetBit(OPT,  OPT_ADD);
  output_data->SetBit(RSTn, 1);

  if (verbose) {
    Serial.println("Initial Data to DUT:");
    output_data->PrintBits(0, 5, true);
  }

  if (verbose) {
    Serial.println("\nInitial Data from DUT:");
    input_data->Read();
    input_data->PrintBits(0, 5, true);
    print_dut_inputs();
    Serial.println();

    Serial.println("RESETTING DUT\n");
  }

  // toggle the reset line
  output_data->SetBit(RSTn, 0);
  output_data->Write();
  pulse_clk();
  print_dut_state(++cycle, true, true, verbose);
  output_data->SetBit(RSTn, 1);
  output_data->Write();
  pulse_clk();
  print_dut_state(++cycle, true, true, verbose);

  // set the operands
  output_data->SetRange(DIN1, 16, din1, DIN1_ORDER);
  output_data->SetRange(DIN2, 16, din2, DIN2_ORDER);
  output_data->SetBit(DIV, 1);
  output_data->Write();

  if (verbose) {
    Serial.println("APPLIED OPERANDS\n");
    input_data->Read();
    print_dut_state(cycle, true, true, verbose);
  }

  bool abusy = false;
  bool mbusy = false;

  // pulse the DUT clock until DOV goes high, for up to 100 cycles
  while (cycle < max_cycles) {
    pulse_clk();
    input_data->Read();
    print_dut_state(++cycle, true, false, verbose);

    // note when ABUSY goes high
    if (input_data->GetBit(ABUSY)) {
      abusy = true;
      output_data->SetBit(OPT, OPT_MULT);
      output_data->Write();
    }

    // note when MBUSY goes high
    if (input_data->GetBit(MBUSY)) {
      abusy = true;
    }

    // break out of the loop when DOV is asserted
    if (input_data->GetBit(DOV)) {
      if (input_data->GetBit(DR) != 1) {
        Serial.println("\nWARNING: Data Ready was not asserted!\n");
      }

      if (input_data->GetBit(ABUSY) != 1) {
        Serial.println("\nWARNING: ABUSY is not asserted!\n");
      }
      if (input_data->GetBit(MBUSY) != 1) {
        Serial.println("\nWARNING: MBUSY is not asserted!\n");
      }

      pulse_clk();
      input_data->Read();
      print_dut_state(++cycle, true, true, verbose);

      break;
    }
  }

  if (cycle < max_cycles) {
    if (verbose) {
      Serial.println("ADD OPERATION IS COMPLETE!\n");
    }
  }
  else {
    Serial.println("WARNING: ADD OPERATION WAS UNSUCCESSFUL\n");
  }

  if (verbose) {
    Serial.println("ACKNOWLEDGING RESULT\n");
  }

  // toggle the DOA line
  output_data->SetBit(DOA, 1);
  output_data->Write();
  pulse_clk();
  input_data->Read();
  print_dut_state(++cycle, true, true, verbose);

  output_data->SetBit(DOA, 0);
  output_data->Write();
  pulse_clk();
  input_data->Read();
  print_dut_state(++cycle, true, true, verbose);

  if (input_data->GetBit(ABUSY)) {
    Serial.println("\nWARNING: ABUSY should not be asserted!\n");
  }
  else {
    success = true;
  }

  // disable the shift register output
  output_data->SetOutputState(HIGH);

  if (verbose) {
    Serial.print("-- END ");
    Serial.println(test_name);
    Serial.println();
  }
  else {
    Serial.print(" - ");
    Serial.print(test_name);
    if (success) {
      Serial.println(":\t PASS");
    }
    else {
      Serial.println(":\t FAIL");
    }
  }

  return success;
}

//------------------------------------------------------------------------------

void pulse_clk()
{
  delay(1);
  output_data->SetBit(CLK, 1);
  output_data->Write();
  delay(1);
  output_data->SetBit(CLK, 0);
  output_data->Write();
}

//------------------------------------------------------------------------------

void print_dut_inputs()
{
  // TODO: figure out why DACK never goes high
  Serial.print("DACK: ");
  Serial.print(input_data->GetBit(DACK));
  Serial.print(", DR: ");
  Serial.print(input_data->GetBit(DR));
  Serial.print(", DOV: ");
  Serial.print(input_data->GetBit(DOV));
  Serial.print(", ABUSY: ");
  Serial.print(input_data->GetBit(ABUSY));
  Serial.print(", MBUSY: ");
  Serial.print(input_data->GetBit(MBUSY));
  Serial.print(", DEBUG: ");
  input_data->PrintRange(DEBUG, DEBUG_WIDTH, false);
  Serial.println();

  Serial.print("DOUT: ");
  input_data->PrintRange(DOUT, 16, DOUT_ORDER);
  Serial.println();
}

//------------------------------------------------------------------------------

void print_dut_outputs()
{
  Serial.print("RSTn: ");
  Serial.print(output_data->GetBit(RSTn));
  Serial.print(", CS: ");
  Serial.print(output_data->GetBit(CS));
  Serial.print(", DIV: ");
  Serial.print(output_data->GetBit(DIV));
  Serial.print(", DOA: ");
  Serial.print(output_data->GetBit(DOA));
  Serial.print(", OPT: ");
  Serial.print(output_data->GetBit(OPT));
  Serial.print(", MODE: ");
  output_data->PrintRange(MODE, MODE_WIDTH, false);
  Serial.println();

  // TODO: might need to reverse the bit order of the result
  Serial.print("DIN1: ");
  output_data->PrintRange(DIN1, 16, DIN1_ORDER);
  Serial.print(", DIN2: ");
  output_data->PrintRange(DIN2, 16, DIN2_ORDER);
  Serial.println();
}

//------------------------------------------------------------------------------

void print_dut_state(int cycle, bool print_in, bool print_out, bool verbose)
{
  if (!verbose) {
    return;
  }

  Serial.println("--------------------------------------------------------------------------------");
  Serial.print("Cycle ");
  Serial.print(cycle, DEC);

  if (print_out) {
    Serial.println();
    Serial.println("Data to DUT:");
    output_data->PrintBits(0, 5, true);
    print_dut_outputs();
  }

  if (print_in) {
    Serial.println();
    Serial.println("Data from DUT:");
    input_data->PrintBits(0, 5, true);
    print_dut_inputs();
    Serial.println();
  }
}

//------------------------------------------------------------------------------
