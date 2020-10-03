// File: GroupRead.ino
// Author: Mag Gyver (Since 2015) 
// Modified: Thorsten Gehrig (Since 2015)

// Test constellation = ARDUINO UNO <-> 5WG1 117-2AB12

/*
-> During programming, the BCU may have no connection to the ARDUINO UNO.
-> After programming for communication with the BCU. Connect the jumpers ICSP1 PIN 5 and ICSP1 PIN 6 together, tested only with ARDUINO UNO revision 3.
-> For programming the jumper ICSP1 PIN 5 and ICSP1 PIN 6 must be not connected together and the voltage must be taken away for a short time. Then, you can transfer the new "sketch".
*/

#include <KnxTpUart.h>

KnxTpUart knx(&Serial, "1.1.199");
int LED = 13;
int target_2_6_0;
int target_5_6_0;

void setup() {

  pinMode(LED, OUTPUT); // PIN 13 as output

  Serial.begin(19200, SERIAL_8E1);
  knx.uartReset();

  knx.addListenGroupAddress("2/6/0");
  knx.addListenGroupAddress("5/6/0");

  // Read request to groups address-> all data types

  // The function delay(1000) only to delay the necessary initialization query.
  // If you want no initialization query, you should comment on the next lines to end the function.

  delay(1000);

  // Read request to group addresses -> possible call to the read request void loop() function

  knx.groupRead("2/6/0");
  knx.groupRead("5/6/0");
}

void loop() {

}

void serialEvent() {

  //Aufruf knx.serialEvent()

  KnxTpUartSerialEventType eType = knx.serialEvent();

  //Evaluation of the received telegram -> only KNX telegrams are accepted

    if (eType == KNX_TELEGRAM) {
    KnxTelegram* telegram = knx.getReceivedTelegram();

    // Telegram evaluation on KNX (at reception always necessary)

    String target =
      String(0 + telegram->getTargetMainGroup())   + "/" +
      String(0 + telegram->getTargetMiddleGroup()) + "/" +
      String(0 + telegram->getTargetSubGroup());

    // Evaluation of group address of the received telegram and caching in variable "target"

    if (telegram->getCommand() == KNX_COMMAND_ANSWER) {

      // Evaluation of read request in serialEvent() with data types
      // Evaluation of the received KNX telegram with response to read request (flag) -> action

      if (target == "2/6/0") {
        target_2_6_0 = telegram->getBool();

        // Storage of the contents in variable "target_2_6_0" of the response to the read request of the group address "2/6/0"

        if (target_2_6_0) {
          digitalWrite(LED, HIGH);
        }
        else {
          digitalWrite(LED,LOW);
        }

        // Evaluation of the content and output of the content of the group address "2/6/0" to PIN 13 of the ARDUINO UNO
      }
      else if (target == "5/6/0") {
        target_5_6_0 = telegram->get1ByteIntValue();

        // Storage of the contents in a variable "target_5_6_0" of the response to the read request of the group address "5/6/0" for further processing

      }
    }
  }
}