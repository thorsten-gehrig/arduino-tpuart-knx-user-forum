// File: KnxTpUart.cpp
// Author: Daniel Kleine-Albers (Since 2012)
// Modified: Thorsten Gehrig (Since 2014)
// Modified: Michael Werski (Since 2014)
// Modified: Katja Blankenheim (Since 2014)
// Modified: Mag Gyver (Since 2016)

// Last modified: 06.06.2017

#include "KnxTpUart.h"

KnxTpUart::KnxTpUart(TPUART_SERIAL_CLASS* sport, String address) {
  _serialport = sport;
  _source_area = address.substring(0, address.indexOf('.')).toInt();
  _source_line = address.substring(address.indexOf('.') + 1, address.length()).substring(0, address.substring(address.indexOf('.') + 1, address.length()).indexOf('.')).toInt();
  _source_member = address.substring(address.lastIndexOf('.') + 1, address.length()).toInt();
  _listen_group_address_count = 0;
  _tg = new KnxTelegram();
  _tg_ptp = new KnxTelegram();
  _listen_to_broadcasts = false;
}

void KnxTpUart::setListenToBroadcasts(bool listen) {
  _listen_to_broadcasts = listen;
}

void KnxTpUart::uartReset() {
  byte sendByte = 0x01;
  _serialport->write(sendByte);
}

void KnxTpUart::uartStateRequest() {
  byte sendByte = 0x02;
  _serialport->write(sendByte);
}

void KnxTpUart::setIndividualAddress(int area, int line, int member) {
  _source_area = area;
  _source_line = line;
  _source_member = member;
}

KnxTpUartSerialEventType KnxTpUart::serialEvent() {
  while (_serialport->available() > 0) {
    checkErrors();

    int incomingByte = _serialport->peek();
    printByte(incomingByte);

    if (isKNXControlByte(incomingByte)) {
      bool interested = readKNXTelegram();
      if (interested) {
#if defined(TPUART_DEBUG)
        TPUART_DEBUG_PORT.println("Event KNX_TELEGRAM");
#endif
        return KNX_TELEGRAM;
      }
      else {
#if defined(TPUART_DEBUG)
        TPUART_DEBUG_PORT.println("Event IRRELEVANT_KNX_TELEGRAM");
#endif
        return IRRELEVANT_KNX_TELEGRAM;
      }
    }
    else if (incomingByte == TPUART_RESET_INDICATION_BYTE) {
      serialRead();
#if defined(TPUART_DEBUG)
      TPUART_DEBUG_PORT.println("Event TPUART_RESET_INDICATION");
#endif
      return TPUART_RESET_INDICATION;
    }
    else {
      serialRead();
#if defined(TPUART_DEBUG)
      TPUART_DEBUG_PORT.println("Event TPUART_UNKNOWN_EVENT");
#endif
      return TPUART_UNKNOWN_EVENT;
    }
  }
#if defined(TPUART_DEBUG)
  TPUART_DEBUG_PORT.println("Event TPUART_UNKNOWN_EVENT");
#endif
  return TPUART_UNKNOWN_EVENT;
}


bool KnxTpUart::isKNXControlByte(int b) {
  return ( (b | 0b00101100) == 0b10111100 ); // Ignore repeat flag and priority flag
}

void KnxTpUart::checkErrors() {
#if defined(TPUART_DEBUG)
#if defined(_SAM3XA_)  // For DUE
  if (USART1->US_CSR & US_CSR_OVRE) {
    TPUART_DEBUG_PORT.println("Overrun");
  }

  if (USART1->US_CSR & US_CSR_FRAME) {
    TPUART_DEBUG_PORT.println("Frame Error");
  }

  if (USART1->US_CSR & US_CSR_PARE) {
    TPUART_DEBUG_PORT.println("Parity Error");
  }
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) // For UNO
  if (UCSR0A & 0b00010000) {
    TPUART_DEBUG_PORT.println("Frame Error");
  }

  if (UCSR0A & 0b00000100) {
    TPUART_DEBUG_PORT.println("Parity Error");
  }
#else
  if (UCSR1A & 0b00010000) {
    TPUART_DEBUG_PORT.println("Frame Error");
  }

  if (UCSR1A & 0b00000100) {
    TPUART_DEBUG_PORT.println("Parity Error");
  }
#endif
#endif
}

void KnxTpUart::printByte(int incomingByte) {
#if defined(TPUART_DEBUG)
  TPUART_DEBUG_PORT.print("Incoming Byte: ");
  TPUART_DEBUG_PORT.print(incomingByte, DEC);
  TPUART_DEBUG_PORT.print(" - ");
  TPUART_DEBUG_PORT.print(incomingByte, HEX);
  TPUART_DEBUG_PORT.print(" - ");
  TPUART_DEBUG_PORT.print(incomingByte, BIN);
  TPUART_DEBUG_PORT.println();
#endif
}

bool KnxTpUart::readKNXTelegram() {
  // Receive header
  for (int i = 0; i < 6; i++) {
    _tg->setBufferByte(i, serialRead());
  }

#if defined(TPUART_DEBUG)
  TPUART_DEBUG_PORT.print("Payload Length: ");
  TPUART_DEBUG_PORT.println(_tg->getPayloadLength());
#endif
  int bufpos = 6;
  for (int i = 0; i < _tg->getPayloadLength(); i++) {
    _tg->setBufferByte(bufpos, serialRead());
    bufpos++;
  }

  // Checksum
  _tg->setBufferByte(bufpos, serialRead());

#if defined(TPUART_DEBUG)
  // Print the received telegram
  _tg->print(&TPUART_DEBUG_PORT);
#endif

  // Verify if we are interested in this message - GroupAddress
  bool interested = _tg->isTargetGroup() && isListeningToGroupAddress(_tg->getTargetMainGroup(), _tg->getTargetMiddleGroup(), _tg->getTargetSubGroup());

  // Physical address
  interested = interested || ((!_tg->isTargetGroup()) && _tg->getTargetArea() == _source_area && _tg->getTargetLine() == _source_line && _tg->getTargetMember() == _source_member);

  // Broadcast (Programming Mode)
  interested = interested || (_listen_to_broadcasts && _tg->isTargetGroup() && _tg->getTargetMainGroup() == 0 && _tg->getTargetMiddleGroup() == 0 && _tg->getTargetSubGroup() == 0);

  if (interested) {
    sendAck();
  }
  else {
    sendNotAddressed();
  }

  if (_tg->getCommunicationType() == KNX_COMM_UCD) {
#if defined(TPUART_DEBUG)
    TPUART_DEBUG_PORT.println("UCD Telegram received");
#endif
  }
  else if (_tg->getCommunicationType() == KNX_COMM_NCD) {
#if defined(TPUART_DEBUG)
    TPUART_DEBUG_PORT.print("NCD Telegram ");
    TPUART_DEBUG_PORT.print(_tg->getSequenceNumber());
    TPUART_DEBUG_PORT.println(" received");
#endif
    if (interested) {
      sendNCDPosConfirm(_tg->getSequenceNumber(), _tg->getSourceArea(), _tg->getSourceLine(), _tg->getSourceMember()); // Thanks to Katja Blankenheim for the help
    }
  }

  // Returns if we are interested in this diagram
  return interested;
}

KnxTelegram* KnxTpUart::getReceivedTelegram() {
  return _tg;
}

// Command Write

bool KnxTpUart::groupWriteBool(String Address, bool value) {
  int valueAsInt = 0;
  if (value) {
    valueAsInt = 0b00000001;
  }

  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, valueAsInt);
  return sendMessage();
}

bool KnxTpUart::groupWrite4BitInt(String Address, int value) {
  int out_value = 0;
  if (value) {
    out_value = value & 0b00001111;
  }

  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, out_value);
  return sendMessage();
}

bool KnxTpUart::groupWrite4BitDim(String Address, bool direction, byte steps) {
  int value = 0;
  if (direction || steps) {
    value = (direction << 3) + (steps & 0b00000111);
  }

  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, value);
  return sendMessage();
}

bool KnxTpUart::groupWrite1ByteInt(String Address, int value) {
  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, 0);
  _tg->set1ByteIntValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupWrite2ByteInt(String Address, int value) {
  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, 0);
  _tg->set2ByteIntValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupWrite2ByteFloat(String Address, float value) {
  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, 0);
  _tg->set2ByteFloatValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupWrite3ByteTime(String Address, int weekday, int hour, int minute, int second) {
  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, 0);
  _tg->set3ByteTime(weekday, hour, minute, second);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupWrite3ByteDate(String Address, int day, int month, int year) {
  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, 0);
  _tg->set3ByteDate(day, month, year);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupWrite4ByteFloat(String Address, float value) {
  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, 0);
  _tg->set4ByteFloatValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupWrite14ByteText(String Address, String value) {
  createKNXMessageFrame(2, KNX_COMMAND_WRITE, Address, 0);
  _tg->set14ByteValue(value);
  _tg->createChecksum();
  return sendMessage();
}

// Command Answer

bool KnxTpUart::groupAnswerBool(String Address, bool value) {
  int valueAsInt = 0;
  if (value) {
    valueAsInt = 0b00000001;
  }

  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, valueAsInt);
  return sendMessage();
}

/*
  bool KnxTpUart::groupAnswerBitInt(String Address, int value) {
  int out_value = 0;
  if (value) {
    out_value = value & B00001111;
  }

  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, out_value);
  return sendMessage();
  }

  bool KnxTpUart::groupAnswer4BitDim(String Address, bool direction, byte steps) {
  int value = 0;
  if (direction || steps) {
    value = (direction << 3) + (steps & B00000111);
  }

  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, value);
  return sendMessage();
  }
*/

bool KnxTpUart::groupAnswer1ByteInt(String Address, int value) {
  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, 0);
  _tg->set1ByteIntValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupAnswer2ByteInt(String Address, int value) {
  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, 0);
  _tg->set2ByteIntValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupAnswer2ByteFloat(String Address, float value) {
  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, 0);
  _tg->set2ByteFloatValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupAnswer3ByteTime(String Address, int weekday, int hour, int minute, int second) {
  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, 0);
  _tg->set3ByteTime(weekday, hour, minute, second);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupAnswer3ByteDate(String Address, int day, int month, int year) {
  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, 0);
  _tg->set3ByteDate(day, month, year);
  _tg->createChecksum();
  return sendMessage();
}
bool KnxTpUart::groupAnswer4ByteFloat(String Address, float value) {
  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, 0);
  _tg->set4ByteFloatValue(value);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::groupAnswer14ByteText(String Address, String value) {
  createKNXMessageFrame(2, KNX_COMMAND_ANSWER, Address, 0);
  _tg->set14ByteValue(value);
  _tg->createChecksum();
  return sendMessage();
}

// Command Read

bool KnxTpUart::groupRead(String Address) {
  createKNXMessageFrame(2, KNX_COMMAND_READ, Address, 0);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::individualAnswerAddress() {
  createKNXMessageFrame(2, KNX_COMMAND_INDIVIDUAL_ADDR_RESPONSE, "0/0/0", 0);
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::individualAnswerMaskVersion(int area, int line, int member) {
  createKNXMessageFrameIndividual(4, KNX_COMMAND_MASK_VERSION_RESPONSE, String(area) + "/" + String(line) + "/" + String(member), 0);
  _tg->setCommunicationType(KNX_COMM_NDP);
  _tg->setBufferByte(8, 0x07); // Mask version part 1 for BIM M 112
  _tg->setBufferByte(9, 0x01); // Mask version part 2 for BIM M 112
  _tg->createChecksum();
  return sendMessage();
}

bool KnxTpUart::individualAnswerAuth(int accessLevel, int sequenceNo, int area, int line, int member) {
  createKNXMessageFrameIndividual(3, KNX_COMMAND_ESCAPE, String(area) + "/" + String(line) + "/" + String(member), KNX_EXT_COMMAND_AUTH_RESPONSE);
  _tg->setCommunicationType(KNX_COMM_NDP);
  _tg->setSequenceNumber(sequenceNo);
  _tg->setBufferByte(8, accessLevel);
  _tg->createChecksum();
  return sendMessage();
}

void KnxTpUart::createKNXMessageFrame(int payloadlength, KnxCommandType command, String address, int firstDataByte) {
  int mainGroup = address.substring(0, address.indexOf('/')).toInt();
  int middleGroup = address.substring(address.indexOf('/') + 1, address.length()).substring(0, address.substring(address.indexOf('/') + 1, address.length()).indexOf('/')).toInt();
  int subGroup = address.substring(address.lastIndexOf('/') + 1, address.length()).toInt();
  _tg->clear();
  _tg->setSourceAddress(_source_area, _source_line, _source_member);
  _tg->setTargetGroupAddress(mainGroup, middleGroup, subGroup);
  _tg->setFirstDataByte(firstDataByte);
  _tg->setCommand(command);
  _tg->setPayloadLength(payloadlength);
  _tg->createChecksum();
}

void KnxTpUart::createKNXMessageFrameIndividual(int payloadlength, KnxCommandType command, String address, int firstDataByte) {
  int area = address.substring(0, address.indexOf('/')).toInt();
  int line = address.substring(address.indexOf('/') + 1, address.length()).substring(0, address.substring(address.indexOf('/') + 1, address.length()).indexOf('/')).toInt();
  int member = address.substring(address.lastIndexOf('/') + 1, address.length()).toInt();
  _tg->clear();
  _tg->setSourceAddress(_source_area, _source_line, _source_member);
  _tg->setTargetIndividualAddress(area, line, member);
  _tg->setFirstDataByte(firstDataByte);
  _tg->setCommand(command);
  _tg->setPayloadLength(payloadlength);
  _tg->createChecksum();
}

bool KnxTpUart::sendNCDPosConfirm(int sequenceNo, int area, int line, int member) {
  _tg_ptp->clear();
  _tg_ptp->setSourceAddress(_source_area, _source_line, _source_member);
  _tg_ptp->setTargetIndividualAddress(area, line, member);
  _tg_ptp->setSequenceNumber(sequenceNo);
  _tg_ptp->setCommunicationType(KNX_COMM_NCD);
  _tg_ptp->setControlData(KNX_CONTROLDATA_POS_CONFIRM);
  _tg_ptp->setPayloadLength(1);
  _tg_ptp->createChecksum();


  int messageSize = _tg_ptp->getTotalLength();

  uint8_t sendbuf[2];
  for (int i = 0; i < messageSize; i++) {
    if (i == (messageSize - 1)) {
      sendbuf[0] = TPUART_DATA_END;
    }
    else {
      sendbuf[0] = TPUART_DATA_START_CONTINUE;
    }

    sendbuf[0] |= i;
    sendbuf[1] = _tg_ptp->getBufferByte(i);

    _serialport->write(sendbuf, 2);
  }


  int confirmation;
  while (true) {
    confirmation = serialRead();
    if (confirmation == 0b10001011) {
      return true; // Sent successfully
    }
    else if (confirmation == 0b00001011) {
      return false;
    }
    else if (confirmation == -1) {
      // Read timeout
      return false;
    }
  }

  return false;
}

bool KnxTpUart::sendMessage() {
  int messageSize = _tg->getTotalLength();

  uint8_t sendbuf[2];
  for (int i = 0; i < messageSize; i++) {
    if (i == (messageSize - 1)) {
      sendbuf[0] = TPUART_DATA_END;
    }
    else {
      sendbuf[0] = TPUART_DATA_START_CONTINUE;
    }

    sendbuf[0] |= i;
    sendbuf[1] = _tg->getBufferByte(i);

    _serialport->write(sendbuf, 2);
  }


  int confirmation;
  while (true) {
    confirmation = serialRead();
    if (confirmation == 0b10001011) {
      delay (SERIAL_WRITE_DELAY_MS);
      return true; // Sent successfully
    }
    else if (confirmation == 0b00001011) {
      delay (SERIAL_WRITE_DELAY_MS);
      return false;
    }
    else if (confirmation == -1) {
      // Read timeout
      delay (SERIAL_WRITE_DELAY_MS);
      return false;
    }
  }

  return false;
}

void KnxTpUart::sendAck() {
  byte sendByte = 0b00010001;
  _serialport->write(sendByte);
  delay(SERIAL_WRITE_DELAY_MS);
}

void KnxTpUart::sendNotAddressed() {
  byte sendByte = 0b00010000;
  _serialport->write(sendByte);
  delay(SERIAL_WRITE_DELAY_MS);
}

int KnxTpUart::serialRead() {
  unsigned long startTime = millis();
#if defined(TPUART_DEBUG)
  TPUART_DEBUG_PORT.print("Available: ");
  TPUART_DEBUG_PORT.println(_serialport->available());
#endif

  while (! (_serialport->available() > 0)) {
    if ((millis() - startTime) > SERIAL_READ_TIMEOUT_MS) {
      // Timeout
#if defined(TPUART_DEBUG)
      TPUART_DEBUG_PORT.println("Timeout while receiving message");
#endif
      return -1;
    }
    delay(1);
  }

  int inByte = _serialport->read();
  checkErrors();
  printByte(inByte);

  return inByte;
}

void KnxTpUart::addListenGroupAddress(String address) {
  if (_listen_group_address_count >= MAX_LISTEN_GROUP_ADDRESSES) {
#if defined(TPUART_DEBUG)
    TPUART_DEBUG_PORT.println("Already listening to MAX_LISTEN_GROUP_ADDRESSES, cannot listen to another");
#endif
    return;
  }

  _listen_group_addresses[_listen_group_address_count][0] = address.substring(0, address.indexOf('/')).toInt();
  ;
  _listen_group_addresses[_listen_group_address_count][1] = address.substring(address.indexOf('/') + 1, address.length()).substring(0, address.substring(address.indexOf('/') + 1, address.length()).indexOf('/')).toInt();
  _listen_group_addresses[_listen_group_address_count][2] = address.substring(address.lastIndexOf('/') + 1, address.length()).toInt();

  _listen_group_address_count++;
}

bool KnxTpUart::isListeningToGroupAddress(int main, int middle, int sub) {
  for (int i = 0; i < _listen_group_address_count; i++) {
    if ( (_listen_group_addresses[i][0] == main)
         && (_listen_group_addresses[i][1] == middle)
         && (_listen_group_addresses[i][2] == sub)) {
      return true;
    }
  }

  return false;
}