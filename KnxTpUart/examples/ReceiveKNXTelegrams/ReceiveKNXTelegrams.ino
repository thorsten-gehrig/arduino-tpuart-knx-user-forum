// File: ReceiveKNXTelegrams.ino
// Author: Daniel Kleine-Albers (Since 2012) 
// Modified: Thorsten Gehrig (Since 2014)
// Modified: Mag Gyver (Since 2016)

// Test constellation = ARDUINO MEGA <-> 5WG1 117-2AB12

#include <KnxTpUart.h>

// Initialize the KNX TP-UART library on the Serial1 port of ARDUINO MEGA
// and with KNX physical address 15.15.20
KnxTpUart knx(&Serial1, "15.15.20");

int LED = 13;

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);
  Serial.println("TP-UART Test");

  Serial1.begin(19200, SERIAL_8E1);

  Serial.print("UCSR1A: ");
  Serial.println(UCSR1A, BIN);

  Serial.print("UCSR1B: ");
  Serial.println(UCSR1B, BIN);

  Serial.print("UCSR1C: ");
  Serial.println(UCSR1C, BIN);

  knx.uartReset();

  knx.addListenGroupAddress("15/0/0");
  knx.addListenGroupAddress("15/0/1");
  knx.addListenGroupAddress("15/0/2");
  knx.addListenGroupAddress("15/0/3");
  knx.addListenGroupAddress("15/0/4");
  knx.addListenGroupAddress("15/0/5");
  knx.addListenGroupAddress("15/0/6");
  knx.addListenGroupAddress("15/0/7");
  knx.addListenGroupAddress("15/0/8");
  knx.addListenGroupAddress("15/0/9");
}

void loop() {
  // nothing in the loop. This example is only to receive telegrams
}

void serialEvent1() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == TPUART_RESET_INDICATION) {
    Serial.println("Event TPUART_RESET_INDICATION");
  } 
  else if (eType == TPUART_UNKNOWN_EVENT) {
    Serial.println("Event TPUART_UNKNOWN_EVENT");
  } 
  else if (eType == KNX_TELEGRAM) {
    Serial.println("Event KNX_TELEGRAM");
    KnxTelegram* telegram = knx.getReceivedTelegram();
    // Telegrammauswertung auf KNX (bei Empfang immer notwendig)
    String target =
      String(0 + telegram->getTargetMainGroup())   + "/" +
      String(0 + telegram->getTargetMiddleGroup()) + "/" +
      String(0 + telegram->getTargetSubGroup());

    // Here you have the telegram and can do whatever you want
    if (telegram->getCommand() == KNX_COMMAND_WRITE) {
      // Auswertung des empfangenen KNX-Telegrammes mit Schreibbefehl (Flag) -> Aktion
      if (target == "15/0/0") {
        int received_15_0_0 = telegram->getBool();
        Serial.print("Empfangener wert");
        Serial.println(received_15_0_0);
        if (received_15_0_0) {
          digitalWrite(LED, HIGH);
        }
        else {
          digitalWrite(LED, LOW);
        }
      }
      if (target == "15/0/1") {
        int received_15_0_1 = telegram->get4BitIntValue();
        Serial.print("Empfangener Wert:");
        Serial.println(received_15_0_1);
      }
      if (target == "15/0/2") {
        int received_15_0_2_0 = telegram->get4BitDirectionValue();
        int received_15_0_2_1 = telegram->get4BitStepsValue();
        Serial.print("Empfangener Wert:");
        Serial.println("");
        switch (received_15_0_2_0) {
        case 0:
          Serial.print("Direction: down");
          break;
        case 1:
          Serial.print("Direction: up");
          break;
        }
        Serial.print("  ");
        switch (received_15_0_2_1) {
        case 0:
          Serial.print("Step: stop");
          break;
        case 1:
          Serial.print("Step: 100%");
          break;
        case 2:
          Serial.print("Step: 50%");
          break;
        case 3:
          Serial.print("Step: 25%");
          break;
        case 4:
          Serial.print("Step: 12%");
          break;
        case 5:
          Serial.print("Step: 6%");
          break;
        case 6:
          Serial.print("Step: 3%");
          break;
        case 7:
          Serial.print("Step: 1%");
          break;
        }
        Serial.println("");
      }
      if (target == "15/0/3") {
        int received_15_0_3 = telegram->get1ByteIntValue();
        Serial.print("Empfangener Wert:");
        Serial.println(received_15_0_3);
      }
      if (target == "15/0/4") {
        int received_15_0_4 = telegram->get2ByteIntValue();
        Serial.print("Empfangener Wert:");
        Serial.println(received_15_0_4);
      }
      if (target == "15/0/5") {
        float received_15_0_5 = telegram->get2ByteFloatValue();
        Serial.print("Empfangener Wert:");
        Serial.println(received_15_0_5);
      }
      if (target == "15/0/6") {
        int received_15_0_6_0 = telegram->get3ByteWeekdayValue();
        int received_15_0_6_1 = telegram->get3ByteHourValue();
        int received_15_0_6_2 = telegram->get3ByteMinuteValue();
        int received_15_0_6_3 = telegram->get3ByteSecondValue();
        Serial.print("Empfangener Wert:");
        Serial.println("");
        Serial.print(received_15_0_6_0);
        Serial.print("  ");
        Serial.print(received_15_0_6_1);
        Serial.print(":");
        Serial.print(received_15_0_6_2);
        Serial.print(":");
        Serial.print(received_15_0_6_3);
        Serial.println("");
      }
      if (target == "15/0/7") {
        int received_15_0_7_0 = telegram->get3ByteDayValue();
        int received_15_0_7_1 = telegram->get3ByteMonthValue();
        int received_15_0_7_2 = telegram->get3ByteYearValue();
        Serial.print("Empfangener Wert:");
        Serial.println("");
        Serial.print(received_15_0_7_0);
        Serial.print(".");
        Serial.print(received_15_0_7_1);
        Serial.print(".");
        Serial.print(received_15_0_7_2);
        Serial.println("");
      }
      if (target == "15/0/8") {
        float received_15_0_8 = telegram->get4ByteFloatValue();
        Serial.print("Empfangener Wert:");
        Serial.println(received_15_0_8);
      }
      if (target == "15/0/9") {
        String received_15_0_9 = telegram->get14ByteValue();
        Serial.print("Empfangener Wert:");
        Serial.println(received_15_0_9);
      }
    }
  }
}