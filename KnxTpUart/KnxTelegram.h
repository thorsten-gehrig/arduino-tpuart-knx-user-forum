// File: KnxTelegram.h
// Author: Daniel Kleine-Albers (Since 2012)
// Modified: Thorsten Gehrig (Since 2014)
// Modified: Michael Werski (Since 2014)
// Modified: Katja Blankenheim (Since 2014)
// Modified: Mag Gyver (Since 2016)

// Last modified: 06.06.2017

#ifndef KnxTelegram_h
#define KnxTelegram_h

#include "Arduino.h"

#define MAX_KNX_TELEGRAM_SIZE 23
#define KNX_TELEGRAM_HEADER_SIZE 6

#define TPUART_SERIAL_CLASS Stream

// KNX priorities
enum KnxPriorityType {
  KNX_PRIORITY_SYSTEM = 0b00,
  KNX_PRIORITY_ALARM = 0b10,
  KNX_PRIORITY_HIGH = 0b01,
  KNX_PRIORITY_NORMAL = 0b11
};

// KNX commands / APCI Coding
enum KnxCommandType {
  KNX_COMMAND_READ = 0b0000,
  KNX_COMMAND_WRITE = 0b0010,
  KNX_COMMAND_ANSWER = 0b0001,
  KNX_COMMAND_INDIVIDUAL_ADDR_WRITE = 0b0011,
  KNX_COMMAND_INDIVIDUAL_ADDR_REQUEST = 0b0100,
  KNX_COMMAND_INDIVIDUAL_ADDR_RESPONSE = 0b0101,
  KNX_COMMAND_MASK_VERSION_READ = 0b1100,
  KNX_COMMAND_MASK_VERSION_RESPONSE = 0b1101,
  KNX_COMMAND_RESTART = 0b1110,
  KNX_COMMAND_ESCAPE = 0b1111
};

// Extended (escaped) KNX commands
enum KnxExtendedCommandType {
  KNX_EXT_COMMAND_AUTH_REQUEST = 0b010001,
  KNX_EXT_COMMAND_AUTH_RESPONSE = 0b010010
};

// KNX Transport Layer Communication Type
enum KnxCommunicationType {
  KNX_COMM_UDP = 0b00, // Unnumbered Data Packet
  KNX_COMM_NDP = 0b01, // Numbered Data Packet
  KNX_COMM_UCD = 0b10, // Unnumbered Control Data
  KNX_COMM_NCD = 0b11  // Numbered Control Data
};

// KNX Control Data (for UCD / NCD packets)
enum KnxControlDataType {
  KNX_CONTROLDATA_CONNECT = 0b00,      // UCD
  KNX_CONTROLDATA_DISCONNECT = 0b01,   // UCD
  KNX_CONTROLDATA_POS_CONFIRM = 0b10,  // NCD
  KNX_CONTROLDATA_NEG_CONFIRM = 0b11   // NCD
};

class KnxTelegram {
  public:
    KnxTelegram();

    void clear();
    void setBufferByte(int index, int content);
    int getBufferByte(int index);
    void setPayloadLength(int size);
    int getPayloadLength();
    void setRepeated(bool repeat);
    bool isRepeated();
    void setPriority(KnxPriorityType prio);
    KnxPriorityType getPriority();
    void setSourceAddress(int area, int line, int member);
    int getSourceArea();
    int getSourceLine();
    int getSourceMember();
    void setTargetGroupAddress(int main, int middle, int sub);
    void setTargetIndividualAddress(int area, int line, int member);
    bool isTargetGroup();
    int getTargetMainGroup();
    int getTargetMiddleGroup();
    int getTargetSubGroup();
    int getTargetArea();
    int getTargetLine();
    int getTargetMember();
    void setRoutingCounter(int counter);
    int getRoutingCounter();
    void setCommand(KnxCommandType command);
    KnxCommandType getCommand();

    void setFirstDataByte(int data);
    int getFirstDataByte();
    bool getBool();

    int get4BitIntValue();
    bool get4BitDirectionValue();
    byte get4BitStepsValue();

    void set1ByteIntValue(int value);
    int get1ByteIntValue();

    void set2ByteIntValue(int value);
    int get2ByteIntValue();
    void set2ByteFloatValue(float value);
    float get2ByteFloatValue();

    void set3ByteTime(int weekday, int hour, int minute, int second);
    int get3ByteWeekdayValue();
    int get3ByteHourValue();
    int get3ByteMinuteValue();
    int get3ByteSecondValue();
    void set3ByteDate(int day, int month, int year);
    int get3ByteDayValue();
    int get3ByteMonthValue();
    int get3ByteYearValue();

    void set4ByteFloatValue(float value);
    float get4ByteFloatValue();

    void set14ByteValue(String value);
    String get14ByteValue();

    void createChecksum();
    bool verifyChecksum();
    int getChecksum();
    void print(TPUART_SERIAL_CLASS*);
    int getTotalLength();
    KnxCommunicationType getCommunicationType();
    void setCommunicationType(KnxCommunicationType);
    int getSequenceNumber();
    void setSequenceNumber(int);
    KnxControlDataType getControlData();
    void setControlData(KnxControlDataType);
  private:
    int buffer[MAX_KNX_TELEGRAM_SIZE];
    int calculateChecksum();

};

#endif