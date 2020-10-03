// File: UnitTests.ino   
// Author: Daniel Kleine-Albers (Since 2012) 
// Modified: Thorsten Gehrig (Since 2014)
// Modified: Mag Gyver (Since 2016)

// Test constellation = Not tested

#include <KnxTpUart.h>
#include <ArduinoUnit.h>

TestSuite suite;
KnxTpUart knx(&Serial1, "15.15.20");
KnxTelegram* knxTelegram = new KnxTelegram();

void setup() {
}

test(knxTelegramClearAfterCreation) {
  knxTelegram = new KnxTelegram();

  for (int i = 0; i < MAX_KNX_TELEGRAM_SIZE; i++) {
    if (i != 0 && i != 5) {
      assertEquals(0, knxTelegram->getBufferByte(i));
    } 
  }  

  assertEquals(B10111100, knxTelegram->getBufferByte(0));
  assertEquals(B11100001, knxTelegram->getBufferByte(5));
  assertEquals(9, knxTelegram->getTotalLength());  
}

test(knxTelegramClear) {
  knxTelegram->setBufferByte(1, 12345);
  knxTelegram->clear();

  for (int i = 0; i < MAX_KNX_TELEGRAM_SIZE; i++) {
    if (i != 0 && i != 5) {
      assertEquals(0, knxTelegram->getBufferByte(i));
    } 
  }  

  assertEquals(B10111100, knxTelegram->getBufferByte(0));
  assertEquals(B11100001, knxTelegram->getBufferByte(5)); 
}

test(repeatProperty) {
  knxTelegram->setRepeated(true);
  assertTrue(knxTelegram->isRepeated());

  knxTelegram->setRepeated(false);
  assertTrue(! knxTelegram->isRepeated());
}

test(priorityProperty) {
  knxTelegram->setPriority(KNX_PRIORITY_NORMAL);
  assertEquals(KNX_PRIORITY_NORMAL, knxTelegram->getPriority()); 

  knxTelegram->setPriority(KNX_PRIORITY_HIGH);
  assertEquals(KNX_PRIORITY_HIGH, knxTelegram->getPriority()); 

  knxTelegram->setPriority(KNX_PRIORITY_ALARM);
  assertEquals(KNX_PRIORITY_ALARM, knxTelegram->getPriority()); 

  knxTelegram->setPriority(KNX_PRIORITY_SYSTEM);
  assertEquals(KNX_PRIORITY_SYSTEM, knxTelegram->getPriority()); 
}

test(sourceAddressProperties) {
  knxTelegram->setSourceAddress(15, 12, 20);
  assertEquals(15, knxTelegram->getSourceArea());
  assertEquals(12, knxTelegram->getSourceLine());
  assertEquals(20, knxTelegram->getSourceMember()); 
}

test(targetAddressProperties) {
  knxTelegram->setTargetGroupAddress(0, 3, 15);
  assertTrue(knxTelegram->isTargetGroup());
  assertEquals(0, knxTelegram->getTargetMainGroup());
  assertEquals(3, knxTelegram->getTargetMiddleGroup());
  assertEquals(15, knxTelegram->getTargetSubGroup()); 
}

test(routingCounterProperty) {
  knxTelegram->setRoutingCounter(5);
  assertEquals(5, knxTelegram->getRoutingCounter()); 
}

test(payloadLengthProperty) {
  knxTelegram->setPayloadLength(5);
  assertEquals(5, knxTelegram->getPayloadLength()); 
}

test(commandProperty) {
  knxTelegram->setCommand(KNX_COMMAND_READ);
  assertEquals(KNX_COMMAND_READ, knxTelegram->getCommand()); 

  knxTelegram->setCommand(KNX_COMMAND_WRITE);
  assertEquals(KNX_COMMAND_WRITE, knxTelegram->getCommand()); 

  knxTelegram->setCommand(KNX_COMMAND_ANSWER);
  assertEquals(KNX_COMMAND_ANSWER, knxTelegram->getCommand()); 
}

test(firstDataByteProperty) {
  knxTelegram->setFirstDataByte(B1100);
  assertEquals(B1100, knxTelegram->getFirstDataByte()); 
}

test(checksumProperty) {
  knxTelegram->createChecksum();
  assertTrue(knxTelegram->verifyChecksum()); 
}

test(receivingGroupAddresses) {
  knx.addListenGroupAddress(15, 15, 100);
  assertTrue(knx.isListeningToGroupAddress(15, 15, 100));
  assertTrue(! knx.isListeningToGroupAddress(15, 3, 28)); 
}

test(floatValues) {
  knxTelegram->set2ByteFloatValue(25.28);
  assertEquals(4, knxTelegram->getPayloadLength());
  assertEquals(25.28 * 100.0, knxTelegram->get2ByteFloatValue() * 100); 
}


void loop() {
  suite.run();
}