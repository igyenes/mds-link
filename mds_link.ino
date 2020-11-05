#define SLINK_PIN               3       // choose the input/output pin
#define DEVICE                  0xB0    // this is a MiniDisck the device identifier is 0xB0
#define SLINK_WORD_DELIMITER  21000     // padding microseconds after transmitting a command (45msecs should be the correct value...)
#define SLINK_LINE_READY       2500     // microseconds to wait for the line to become ready before transmitting a command
#define SLINK_LOOP_DELAY         80     // microseconds timer thick during a wait operation
#define SLINK_LOOP_TIMEOUT   500000     // microseconds total timeout while waiting           


String  slink_cmd = "";                 // slink command via serial interface
int     cmd_length = 0;                 // lenght of the command            
byte    track_or_disc = 0;              // 0x98 if disc writing for track 0x9A
byte    track_no = 0;                   // Tranck no. - for disc labeling it is '01'
byte    more = 0;                       // 0x99 if disc, 0x9B if track



void setup() {
  Serial.begin(9600);
  pinMode(SLINK_PIN, OUTPUT);
  digitalWrite(SLINK_PIN, HIGH); // default bus state: HIGH 5V
}

void send_init() {
  pinMode(SLINK_PIN, OUTPUT);
  digitalWrite(SLINK_PIN, LOW);
  delayMicroseconds(2400);
  digitalWrite(SLINK_PIN, HIGH);
  delayMicroseconds(600);
}

void send_bit1() {
  pinMode(SLINK_PIN, OUTPUT);
  digitalWrite(SLINK_PIN, LOW);
  delayMicroseconds(1200);
  digitalWrite(SLINK_PIN, HIGH);
  delayMicroseconds(600);
}

void send_bit0() {
  pinMode(SLINK_PIN, OUTPUT);
  digitalWrite(SLINK_PIN, LOW);
  delayMicroseconds(600);
  digitalWrite(SLINK_PIN, HIGH);
  delayMicroseconds(600);
}

void send_byte(byte octet) {
  for (int i = 7; i >= 0 ; i--) {
    if (octet & 1 << i) {
      send_bit1();
    } else {
      send_bit0();
    }
  }
}

void wait_line_ready () {
  pinMode (SLINK_PIN, INPUT);
  unsigned long Start = micros();
  unsigned long beginTimeout = Start;

  do {
    delayMicroseconds(SLINK_LOOP_DELAY);
    if (digitalRead(SLINK_PIN) == LOW)
      Start = micros(); // reset the loop each time traffic is detected
  }
  while ( (micros() - Start < SLINK_LINE_READY) && (micros() - beginTimeout < SLINK_LOOP_TIMEOUT) );

}

void loop() {
  unsigned long Start;
  if (Serial.available()) {
    slink_cmd = Serial.readString ();
    cmd_length = slink_cmd.length();

    // first to chars has to be a digit othervise drop the whole thing
    if ((isDigit(slink_cmd.charAt(0))) && (isDigit(slink_cmd.charAt(1))))  {

      // check the length of the message: if it is longer than 140 chars then drop it
      if (cmd_length < 140) {
        Serial.print("Title to be written:"); Serial.println (slink_cmd);

        // the command has to be a multiplication of 16 let's round it
        if (cmd_length % 16 != 0) {
          cmd_length = (((cmd_length / 16) + 1) * 16);
        }

        // define an array and fill with '0'
        byte slink_byte [cmd_length + 1];
        for (int z = 0; z < cmd_length + 1; z++) {
          slink_byte [z] = 0;
        }

        //convert the string into an array of bytes
        slink_cmd.getBytes (slink_byte, cmd_length + 1);

        // if the first two chars are '00' then this will be disk title
        if ( (slink_byte [0] == 48) && (slink_byte [1] == 48) ) {
              track_or_disc = 0x98;
              track_no = 0x01;
              more = 0x99;
        } else {
              track_or_disc = 0x9A;
              track_no = byte (int(slink_byte [0] - 48) * 10 + int(slink_byte [1] - 48));
              more = 0x9B;
        }

        //write the title to the DEVICE
        wait_line_ready ();
        send_init ();
        send_byte (DEVICE);
        send_byte (track_or_disc);
        send_byte (track_no);
        send_byte (00);
        send_byte (00);

        // First part of the title
        for (int i = 2; i < 16 ; i++) {
          send_byte (slink_byte [i]);
        }  delayMicroseconds(SLINK_WORD_DELIMITER);


        //If the title is longer than 14 chars use 'more' command
        for ( int more_blk = 1; more_blk < cmd_length / 16; more_blk++) {

          wait_line_ready ();
          pinMode (SLINK_PIN, OUTPUT);
          send_init();    
          send_byte(DEVICE);
          send_byte(more);
          send_byte (byte (more_blk + 1));

          for (int m = more_blk * 16; m < more_blk * 16 + 16; m++) {
            send_byte (slink_byte [m]);
          } delayMicroseconds(SLINK_WORD_DELIMITER);


        }
      } else {
        Serial.println ("Command is too long.");
      }
    } else {
      Serial.println ("First two chars has to be numbers. Use '00' for Disk Title.");
    }
  }
  delay (2);
}
