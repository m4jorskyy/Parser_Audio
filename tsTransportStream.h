//tsTransportStream.h

#pragma once
#include "tsCommon.h"
#include <string>
#include <vector>
using namespace std;

/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/


//=============================================================================================================================================================================

class xTS {
public:
    static constexpr uint32_t TS_PacketLength = 188;
    static constexpr uint32_t TS_HeaderLength = 4;

    static constexpr uint32_t PES_HeaderLength = 6;


    static constexpr uint32_t BaseClockFrequency_Hz = 90000; //Hz
    static constexpr uint32_t ExtendedClockFrequency_Hz = 27000000; //Hz
    static constexpr uint32_t BaseClockFrequency_kHz = 90; //kHz
    static constexpr uint32_t ExtendedClockFrequency_kHz = 27000; //kHz
    static constexpr uint32_t BaseToExtendedClockMultiplier = 300;
};

//=============================================================================================================================================================================

class xTS_PacketHeader {
public:
    enum class ePID : uint16_t {
        PAT = 0x0000,
        CAT = 0x0001,
        TSDT = 0x0002,
        IPMT = 0x0003,
        NIT = 0x0010, //DVB specific PID
        SDT = 0x0011, //DVB specific PID
        NuLL = 0x1FFF,
    };

protected:
    uint32_t m_header;
    uint8_t m_SB;
    uint8_t m_E;
    uint8_t m_S;
    uint8_t m_T;
    uint16_t m_PID;
    uint8_t m_TSC;
    uint8_t m_AFC;
    uint8_t m_CC;

public:
    void Reset();

    int32_t Parse(const uint8_t *Input);

    void Print() const;

    uint8_t getSyncByte() const { return m_SB; }
    uint8_t getEByte() const { return m_E; }
    uint8_t getSByte() const { return m_S; }
    uint8_t getTByte() const { return m_T; }
    uint16_t getPID() const { return m_PID; }
    uint8_t getTSC() const { return m_TSC; }
    uint8_t getAFC() const { return m_AFC; }
    uint8_t getCC() const { return m_CC; }

    bool hasAdaptationField() const {
        if (m_AFC == 0x02 || m_AFC == 0x03) {
            return true;
        }
        return false;
    }

    bool hasPayload() const {
        return (m_AFC == 0x01 || m_AFC == 0x03);
    }

    bool getPayloadUnitStartIndicator() const { return m_S; }
    bool hasContinuityCounter() const { return m_TSC != 0x03; } // CC is undefined when TSC is '11'
};

//=============================================================================================================================================================================

class xTS_AdaptationField {
protected:
    //AF length
    uint8_t m_AF;
    uint8_t m_Stuffing;
    //mandatory fields
    uint8_t m_AFC;
    uint8_t m_DC;
    uint8_t m_RA;
    uint8_t m_SP;
    uint8_t m_PCR;
    uint8_t m_OR;
    uint8_t m_SF;
    uint8_t m_TP;
    uint8_t m_EX;

    uint64_t m_PCR_base;
    uint8_t m_PCR_reserved;
    uint16_t m_PCR_extension;

    uint64_t m_OPCR_base;
    uint8_t m_OPCR_reserved;
    uint16_t m_OPCR_extension;

public:
    void Reset();

    int32_t Parse(const uint8_t *PacketBuffer, uint8_t AdaptationFieldControl);

    void Print() const;

    uint32_t getNumBytes() const {
        return (!m_AF)? 0 : m_AF + 1;
    }
};


//=============================================================================================================================================================================

class xPES_PacketHeader {
public:
    enum class eStreamId : uint8_t {
        eStreamId_program_stream_map = 0xBC,
        eStreamId_padding_stream = 0xBE,
        eStreamId_private_stream_2 = 0xBF,
        eStreamId_ECM = 0xF0,
        eStreamId_EMM = 0xF1,
        eStreamId_program_stream_directory = 0xFF,
        eStreamId_DSMCC_stream = 0xF2,
        eStreamId_ITUT_H222_1_type_E = 0xF8
    };

protected:
    uint32_t m_PacketStartCodePrefix;
    uint8_t m_StreamId;
    uint16_t m_PacketLength;
    uint8_t m_HeaderLength;

public:
    void Reset();
    int32_t Parse(const uint8_t *PacketBuffer);
    void Print() const;

    uint32_t getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }
    uint8_t getStreamId() const { return m_StreamId; }
    uint16_t getPacketLength() const { return m_PacketLength; }
    uint8_t getHeaderLength() const { return m_HeaderLength; }
};

//=============================================================================================================================================================================

class xPES_Assembler {
public:
    enum class eResult : int32_t {
        UnexpectedPID = 1,
        StreamPacketLost,
        AssemblingStarted,
        AssemblingContinue,
        AssemblingFinished,
        NoPayload,
        BufferOverflow
    };

protected:
    int32_t m_PID;
    uint8_t *m_Buffer;
    uint32_t m_BufferSize;
    int8_t m_LastContinuityCounter;
    bool m_Started;
    xPES_PacketHeader m_PESH;

public:
    xPES_Assembler();

    ~xPES_Assembler();

    void Init(int32_t PID);

    eResult AbsorbPacket(const uint8_t *TransportStreamPacket, const xTS_PacketHeader *PacketHeader,
                         const xTS_AdaptationField *AdaptationField);

    void PrintPESH() const {m_PESH.Print();};
    uint8_t *getPacket() { return m_Buffer; }
    int32_t getNumPacketBytes() const { return m_BufferSize; }
    uint8_t getHeaderLength() const { return m_PESH.getHeaderLength(); }

    void assemblerPes(const uint8_t *TS_PacketBuffer, const xTS_PacketHeader *TS_PacketHeader,
                      const xTS_AdaptationField *TS_AdaptationField, FILE *AudioMP2);

    void saveBufferToFile(FILE *AudioMP2);

protected:
    void xBufferReset();

    void xBufferAppend(const uint8_t *Data, uint32_t Size);
};