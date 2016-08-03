#ifndef _SHIFT_IO_H_
#define _SHIFT_IO_H_

//------------------------------------------------------------------------------

#define NUM_ICS  (10)
#define IC_WIDTH (8)

//------------------------------------------------------------------------------

class SerialData {
 public:
  unsigned char data[NUM_ICS];

  // no-argument constructor
  SerialData()
  {
    ClearData();
  }

  // initialize data with data from (presumably) another SerialData object
  SerialData(const unsigned char* src_data)
  {
    memcpy(data, src_data, NUM_ICS);
  }

  // copy constructor
  SerialData(const SerialData& src)
  {
    memcpy(data, src.data, NUM_ICS);
  }

  //----------------------------------------------------------------------------
  // Member Functions
  //----------------------------------------------------------------------------

  void ClearData()
  {
    for (int i = 0; i < NUM_ICS; ++i) {
      data[i] = 0;
    }
  }

  // Returns the bit stored at <index>.
  int GetBit(int index)
  {
    assert(index < NUM_ICS * IC_WIDTH);

    int ic_num = index / IC_WIDTH;
    int offset = index % IC_WIDTH;

    return bitRead(data[ic_num], offset);
  }

  // Sets the bit at <index> with <value>.
  void SetBit(int index, int value)
  {
    assert(index < NUM_ICS * IC_WIDTH);

    int ic_num = index / IC_WIDTH;
    int offset = index % IC_WIDTH;

    bitWrite(data[ic_num], offset, value);
  }

  void SetVector(const unsigned char* input_vector)
  {
    for (int i = 0; i < NUM_ICS; ++i) {
      data[i] = input_vector[i];
    }
  }

  void SetRange(int start_bit, int num_bits, int values, bool reverse=false)
  {
    assert(start_bit + num_bits <= NUM_ICS * IC_WIDTH);

    if (reverse) {
      for (int bit_num = 0; bit_num < num_bits; ++bit_num) {
        SetBit(start_bit + bit_num, bitRead(values, num_bits - bit_num - 1));
      }
    }
    else {
      for (int bit_num = 0; bit_num < num_bits; ++bit_num) {
        SetBit(start_bit + bit_num, bitRead(values, bit_num));
      }
    }
  }

  int GetRange(int start_bit, int num_bits, bool reverse=false)
  {
    assert(start_bit + num_bits <= NUM_ICS * IC_WIDTH);
    assert(num_bits <= 16);

    int ret_val = 0;

    if (reverse) {
      for (int bit_num = num_bits; bit_num >= 0; --bit_num) {
        bitWrite(ret_val, bit_num, GetBit(start_bit + bit_num));
      }
    }
    else {
      for (int bit_num = 0; bit_num < num_bits; ++bit_num) {
        bitWrite(ret_val, bit_num, GetBit(start_bit + bit_num));
      }
    }

    return ret_val;
  }

  unsigned char GetByte(int ic_num)
  {
    assert(ic_num < NUM_ICS);
    return data[ic_num];
  }

  void SetByte(int ic_num, unsigned char byte_val)
  {
    assert(ic_num < NUM_ICS);
    data[ic_num] = byte_val;
  }

  // Returns a pointer to the stored data.
  unsigned char* GetData()
  {
    return data;
  }

  void PrintRange(int start_bit, int num_bits, bool reverse=false)
  {
    assert(start_bit + num_bits < IC_WIDTH * NUM_ICS);

    if (reverse) {
      for (int bit_index = 0; bit_index < num_bits; ++bit_index) {
        Serial.print(GetBit(start_bit + bit_index));
      }
    }
    else {
      for (int bit_index = num_bits - 1; bit_index >= 0; --bit_index) {
        Serial.print(GetBit(start_bit + bit_index));
      }
    }
  }

  // Prints the binary values of each stored bit, with one row per IC.
  void PrintBits(int start_ic=0, int end_ic=NUM_ICS-1, bool raw_vector=false)
  {
    // make sure that the USB serial port is available
    if (!Serial) {
      return;
    }

    if (!raw_vector) {
      Serial.println("   7 6 5 4 3 2 1 0");
      for (int ic_num = start_ic; ic_num <= end_ic; ++ic_num) {
        Serial.print(ic_num, HEX);
        Serial.print("  ");
        for (int bit_index = IC_WIDTH - 1; bit_index >= 0; --bit_index) {
          Serial.print(GetBit((ic_num * IC_WIDTH) + bit_index));
          Serial.print(' ');
        }
        Serial.print('\n');
      }
    }
    else {
      for (int bit_index = IC_WIDTH * NUM_ICS - 1; bit_index >= 0; --bit_index) {
        Serial.print(GetBit(bit_index));
        if (bit_index % 8 == 0) {
          Serial.print(' ');
        }
      }

      Serial.println();
    }
  }
};

//------------------------------------------------------------------------------

class InputData : public SerialData {
 public:
  int dsout;
  int plbar;
  int cp;

  // constructor
  InputData(int dsout, int plbar, int cp)
      : dsout (dsout),
        plbar (plbar),
        cp    (cp)
  {
    pinMode(dsout, INPUT);
    pinMode(plbar, OUTPUT);
    pinMode(cp,    OUTPUT);

    digitalWrite(cp, HIGH);
    digitalWrite(cp, LOW);
  }

  // Reads the entire shift chain and stores the values in data.
  void Read()
  {
    // trigger parallel load
    digitalWrite(plbar, LOW);
    digitalWrite(plbar, HIGH);

    // shift in the latched data MSB-first, and store it LSB-first
    for (int ic_num = NUM_ICS - 1; ic_num >= 0; --ic_num) {
      for (int index = 0; index < IC_WIDTH; ++index) {
        bitWrite(data[ic_num], IC_WIDTH - 1 - index, digitalRead(dsout));

        // pulse the shift clock
        digitalWrite(cp, HIGH);
        digitalWrite(cp, LOW);
      }
    }
  }
};

//------------------------------------------------------------------------------

class OutputData : public SerialData {
 public:
  int dsin;
  int oebar;
  int stcp;
  int shcp;
  int mrbar;

  // constructor
  OutputData(int dsin, int oebar, int stcp, int shcp, int mrbar)
      : dsin  (dsin),
        oebar (oebar),
        stcp  (stcp),
        shcp  (shcp),
        mrbar (mrbar)
  {
    pinMode(dsin,  OUTPUT);
    pinMode(oebar, OUTPUT);
    pinMode(stcp,  OUTPUT);
    pinMode(shcp,  OUTPUT);
    pinMode(mrbar, OUTPUT);

    digitalWrite(dsin,  LOW);
    digitalWrite(oebar, HIGH);
    digitalWrite(stcp,  LOW);
    digitalWrite(shcp,  LOW);
    digitalWrite(mrbar, HIGH);
  }

  // If <state> is LOW, output is enabled.
  // If <state> is HIGH, output is disabled.
  void SetOutputState(int state)
  {
    digitalWrite(oebar, state);
  }

  // Writes the stored data to the output shift registers.
  void Write()
  {
    // reset the shift register
    digitalWrite(mrbar, LOW);
    digitalWrite(mrbar, HIGH);

    // shift in the values
    for (int index = IC_WIDTH * NUM_ICS - 1; index >= 0 ; --index) {
      // set the serial output pin
      digitalWrite(dsin, GetBit(index));

      // pulse the shift clock
      digitalWrite(shcp, HIGH);
      digitalWrite(shcp, LOW);
    }

    // pulse the storage clock to present the values at the output
    digitalWrite(stcp, HIGH);
    digitalWrite(stcp, LOW);
  }
};

//------------------------------------------------------------------------------

#endif
