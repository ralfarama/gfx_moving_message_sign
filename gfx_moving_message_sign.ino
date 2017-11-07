

int PIN_CSG2B = A0;
int PIN_CSSA  = A1;
int PIN_CSSB  = A2;
int PIN_CSSC  = A3;
int PIN_CSG2A = A4;

int PIN_DBUS0 = 2;
int PIN_DBUS1 = 3;
int PIN_DBUS2 = 4;
int PIN_DBUS3 = 5;
int PIN_DBUS4 = 6;
int PIN_DBUS5 = 7;
int PIN_DBUS6 = 8;
int PIN_DBUS7 = 9;

byte ledbuffer[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

void fullreset(void) {
  // zero out the data bus and pull the reset line low to clear registers
  digitalWrite(PIN_DBUS0,LOW);
  digitalWrite(PIN_DBUS1,LOW);
  digitalWrite(PIN_DBUS2,LOW);
  digitalWrite(PIN_DBUS3,LOW);
  digitalWrite(PIN_DBUS4,LOW);
  digitalWrite(PIN_DBUS5,LOW);
  digitalWrite(PIN_DBUS6,LOW);
  digitalWrite(PIN_DBUS7,LOW);  // reset line low
  delay(10) ;           // keep it low for .01 sec
  digitalWrite(PIN_DBUS7,HIGH); // pull up reset line
}

void set_matrix_col_latch(int latch_num, int latch_bit, int latch_set) {
  int latch_i, s0val, s1val, s2val;
  // start by disabling LS138 outputs
  digitalWrite(PIN_CSG2A,HIGH);
  digitalWrite(PIN_CSG2B,HIGH);
  // set higher bits of LS138 output select
  digitalWrite(PIN_CSSC,HIGH);
  digitalWrite(PIN_CSSB,HIGH);
  // set latch_i to low bit of LS138 output select
  if ( latch_num & 1 ) {
    latch_i = HIGH;
    digitalWrite(PIN_CSSA,HIGH);
  } else {
    latch_i = LOW;
    digitalWrite(PIN_CSSA,LOW);
  }
  // set s{0,1,2}val to bus line values to select bit of addressed LS259
  if ( latch_bit & 1 ) { s0val = HIGH; } else { s0val = LOW; }
  if ( latch_bit & 2 ) { s1val = HIGH; } else { s1val = LOW; }
  if ( latch_bit & 4 ) { s2val = HIGH; } else { s2val = LOW; }
  digitalWrite(PIN_CSSA,latch_i);
  // disable reset of addressed LS259
  digitalWrite(PIN_DBUS0,HIGH);
  // set data bus lines to select latch bit in addressed LS259
  digitalWrite(PIN_DBUS4,s2val);
  digitalWrite(PIN_DBUS5,s1val);
  digitalWrite(PIN_DBUS6,s0val);
  // set value for addressed latch bit
  digitalWrite(PIN_DBUS7,latch_set);
  // pull down LS138 G2 lines to enable outputs and perform load
  digitalWrite(PIN_CSG2A,LOW);
  digitalWrite(PIN_CSG2B,LOW);
  // pull up LS138 G2 lines to disable output
  digitalWrite(PIN_CSG2A,HIGH);
  digitalWrite(PIN_CSG2B,HIGH);
}

void set_matrix_bank_register(int bank,int reg_value) {
  int bankbit0, bankbit1,bankbit2;

  if ( (bank+2) & 1 ) { bankbit0 = HIGH; } else { bankbit0 = LOW; }
  if ( (bank+2) & 2 ) { bankbit1 = HIGH; } else { bankbit1 = LOW; }
  if ( (bank+2) & 4 ) { bankbit2 = HIGH; } else { bankbit2 = LOW; }  
  // start by disabling LS138 outputs
  digitalWrite(PIN_CSG2A,HIGH);
  digitalWrite(PIN_CSG2B,HIGH);
  // set LS138 select bits to choose desired LS273 register
  digitalWrite(PIN_CSSA,bankbit0);
  digitalWrite(PIN_CSSB,bankbit1);
  digitalWrite(PIN_CSSC,bankbit2);
  // disable the reset on the register
  digitalWrite(PIN_DBUS0,HIGH);
  // set data bus values
  for ( int i = 0; i < 7; i++ ) {
    int portnum = i + 3;
    int portbit;
    
    if ( reg_value & ( 1 << i ) ) {
      portbit = HIGH;
    } else {
      portbit = LOW;
    }
    digitalWrite(portnum,portbit);
  }
  // strobe the LS138 output enable lines to perform the load
  digitalWrite(PIN_CSG2A,LOW);
  digitalWrite(PIN_CSG2B,LOW);
  digitalWrite(PIN_CSG2A,HIGH);
  digitalWrite(PIN_CSG2B,HIGH);  
}

void movingsign_bootstrap(void) {
  // 74LS138 demultiplexer chip select control signal outputs
  pinMode(PIN_CSG2B, OUTPUT); // 74LS138 Pin 5 - /G2B
  pinMode(PIN_CSSA, OUTPUT); // 74LS138 Pin 1 - A
  pinMode(PIN_CSSB, OUTPUT); // 74LS138 Pin 2 - B
  pinMode(PIN_CSSC, OUTPUT); // 74LS138 Pin 3 - C
  pinMode(PIN_CSG2A, OUTPUT); // 74LS138 Pin 4 - /G2A
  // 74LS138 outputs (Y=1*A+2*B+4*C):
  // Y2 - 74LS259 IC11 /G input
  // Y3 - 74LS259 IC13 /G input
  // Y4 - 74LS373 IC4  CP input
  // Y5 - 74LS373 IC5  CP input
  // Y6 - 74LS373 IC10 CP input
  // Y7 - 74LS373 IC12 CP input
  
  // data bus outputs
  // each line goes to two 74LS259 latches and four 74LS273 registers
  pinMode(PIN_DBUS0, OUTPUT); // LS273 D0  (Pin  3) ; LS259 D   (Pin 13)
  pinMode(PIN_DBUS1, OUTPUT); // LS273 D1  (Pin  4) ; LS259 S0  (Pin  1)
  pinMode(PIN_DBUS2, OUTPUT); // LS273 D2  (Pin  7) ; LS259 S1  (Pin  2)
  pinMode(PIN_DBUS3, OUTPUT); // LS273 D3  (Pin  8) ; LS259 S2  (Pin  3)
  pinMode(PIN_DBUS4, OUTPUT); // LS273 D4  (Pin 13)
  pinMode(PIN_DBUS5, OUTPUT); // LS273 D5  (Pin 14)
  pinMode(PIN_DBUS6, OUTPUT); // LS273 D6  (Pin 17)
  // this line acts as a reset line to the latches and registers
  pinMode(PIN_DBUS7, OUTPUT); // LS273 /MR (Pin  1) ; LS259 /CLR (Pin 15)

  // set initial state to LS138 disabled, all other lines low
  digitalWrite(PIN_CSG2B,HIGH);
  digitalWrite(PIN_CSSA,LOW);
  digitalWrite(PIN_CSSB,LOW);
  digitalWrite(PIN_CSSC,LOW);
  digitalWrite(PIN_CSG2A,HIGH);
  fullreset();
  for ( int l_i = 0; l_i < 2; l_i ++ ) {
    for ( int l_b = 0; l_b < 8; l_b ++ ) {
      set_matrix_col_latch(l_i,l_b,HIGH);
    }
  }
}

void enable_col(int colnum) {
  int bank, bcol;
  int bankbit0, bankbit1,bankbit2;
  int latch_i, bit_i;
  int s0val, s1val, s2val;

  bank = colnum / 15;
  bcol = colnum % 15;
  latch_i = bcol >> 3;
  bit_i = bcol & 0x07;
  if ( (bank+2) & 1 ) { bankbit0 = HIGH; } else { bankbit0 = LOW; }
  if ( (bank+2) & 2 ) { bankbit1 = HIGH; } else { bankbit1 = LOW; }
  if ( (bank+2) & 4 ) { bankbit2 = HIGH; } else { bankbit2 = LOW; }
  
  if ( bit_i & 1 ) { s0val = HIGH; } else { s0val = LOW; }
  if ( bit_i & 2 ) { s1val = HIGH; } else { s1val = LOW; }
  if ( bit_i & 4 ) { s2val = HIGH; } else { s2val = LOW; }

  // disable LS138 outputs prior to setting signals
  digitalWrite(PIN_CSG2B,HIGH); // disable all LS138 outputs
  digitalWrite(PIN_CSG2A,HIGH); // disable all LS138 outputs
  // set LS138 signals to perform latch select to enable desired column
  digitalWrite(PIN_CSSC,HIGH); // set high select bit of LS138 
  digitalWrite(PIN_CSSB,HIGH); // set middle select bit of LS138
  digitalWrite(PIN_CSSA,latch_i); // low select bit of LS138, selecting latch
  digitalWrite(PIN_DBUS0,HIGH); // disable latch clear signal
  digitalWrite(PIN_DBUS6,s0val);
  digitalWrite(PIN_DBUS5,s1val);
  digitalWrite(PIN_DBUS4,s2val);
  // reset latches (LS273's, LS259's)
  digitalWrite(PIN_DBUS7,LOW);
  //
  digitalWrite(PIN_CSG2B,LOW);
  digitalWrite(PIN_CSG2A,LOW);

  // disable LS138 outputs before moving on
  digitalWrite(PIN_CSG2B,HIGH);
  digitalWrite(PIN_CSG2A,HIGH);

  // set LS138 select bits to choose desired LS273 register
  digitalWrite(PIN_CSSA,bankbit0);
  digitalWrite(PIN_CSSB,bankbit1);
  digitalWrite(PIN_CSSC,bankbit2);
}

void setup() {
  // put your setup code here, to run once:
  movingsign_bootstrap();
}

int c;
int iters = 0;

void loop() {
  // put your main code here, to run repeatedly:
  int c, i, qq;

  if ( iters % 2 == 0 ) {
    qq = (millis() >> 9) & 0x7f;
    for ( c = 0; c < 60; c++ ) {
      ledbuffer[c] = qq+c;
    }
  }
  iters = iters + 1;
  
  for (c = 0; c < 15; c++ ) {
    set_matrix_bank_register(0,ledbuffer[c]);
    set_matrix_bank_register(1,ledbuffer[c+15]);
    set_matrix_bank_register(2,ledbuffer[c+30]);
    set_matrix_bank_register(3,ledbuffer[c+45]);
    // int c_banknum = c / 15;
    // int c_bankcol = c % 15;
    int c_latchbit = c & 7;
    int c_latchnum = ( c & 8 ) >> 3;

    set_matrix_col_latch(c_latchnum,c_latchbit,LOW);
    delay(1);
    set_matrix_col_latch(c_latchnum,c_latchbit,HIGH);
  }
//  delay(50);
}
