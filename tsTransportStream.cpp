#include "tsTransportStream.h"
#include <cstring>


//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_PacketHeader::Reset()
{
    m_header = 0;
    m_SB = 0;
    m_E = 0;
    m_S = 0;
    m_T = 0;
    m_PID = 0;
    m_TSC = 0;
    m_AFC = 0;
    m_CC = 0;
}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input)
{
    m_SB = Input[0];
    m_E = (Input[1] >> 7) & 0x01;
    m_S = (Input[1] >> 6) & 0x01;
    m_T = (Input[1] >> 5) & 0x01;
    m_PID = ((uint16_t)(Input[1] & 0x1F) << 8) | Input[2];
    m_TSC = (Input[3] >> 6) & 0x03;
    m_AFC = (Input[3] >> 4) & 0x03;
    m_CC = Input[3] & 0x0F;

    m_header = ((uint32_t)Input[0] << 24) |
        ((uint32_t)Input[1] << 16) |
        ((uint32_t)Input[2] << 8) |
        ((uint32_t)Input[3]);
    return m_header;
}

void xTS_PacketHeader::Print() const
{
    printf("TS: SB=%d E=%d S=%d T=%d PID=%d TSC=%d AFC=%d CC=%d\n", m_SB, m_E, m_S, m_T, m_PID, m_TSC, m_AFC, m_CC);
}

//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================
void xTS_AdaptationField::Reset()
{
    m_AF = 0;
    m_Stuffing = 0;
    m_AFC = 0;
    m_DC = 0;
    m_RA = 0;
    m_SP = 0;
    m_PCR = 0;
    m_OR = 0;
    m_SF = 0;
    m_TP = 0;
    m_EX = 0;
    m_PCR_base = 0;
    m_PCR_reserved = 0;
    m_PCR_extension = 0;
    m_OPCR_base = 0;
    m_OPCR_reserved = 0;
    m_OPCR_extension = 0;
}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl)
{
    m_AFC = AdaptationFieldControl;
    if (m_AFC == 0x02 || m_AFC == 0x03)
    {
        m_AF = Input[4];
        if (m_AF > 0)
        {
            m_Stuffing = m_AF - 1;

            m_DC = (Input[5] >> 7) & 0x01;
            m_RA = (Input[5] >> 6) & 0x01;
            m_SP = (Input[5] >> 5) & 0x01;
            m_PCR = (Input[5] >> 4) & 0x01;
            m_OR = (Input[5] >> 3) & 0x01;
            m_SF = (Input[5] >> 2) & 0x01;
            m_TP = (Input[5] >> 1) & 0x01;
            m_EX = Input[5] & 0x01;

            int pointer = 6;
            if (m_PCR)
            {
                m_PCR_base = ((uint64_t)Input[pointer] << 25) |
                    ((uint64_t)Input[pointer + 1] << 17) |
                    ((uint64_t)Input[pointer + 2] << 9) |
                    ((uint64_t)Input[pointer + 3] << 1) |
                    ((uint64_t)(Input[pointer + 4] >> 7) & 0x01);
                m_PCR_reserved = (Input[pointer + 4] >> 1) & 0x3F;
                m_PCR_extension = ((uint16_t)(Input[pointer + 4] & 0x01) << 8) | Input[pointer + 5];
                pointer += 6;
                m_Stuffing -= 6;
            }
            if (m_OR)
            {
                m_OPCR_base = ((uint64_t)Input[pointer] << 25) |
                    ((uint64_t)Input[pointer + 1] << 17) |
                    ((uint64_t)Input[pointer + 2] << 9) |
                    ((uint64_t)Input[pointer + 3] << 1) |
                    ((uint64_t)(Input[pointer + 4] >> 7) & 0x01);
                m_OPCR_reserved = (Input[pointer + 4] >> 1) & 0x3F;
                m_OPCR_extension = ((uint16_t)(Input[pointer + 4] & 0x01) << 8) | Input[pointer + 5];
                pointer += 6;
                m_Stuffing -= 6;
            }
        }
        else
        {
            m_DC = m_RA = m_SP = m_PCR = m_OR = m_SF = m_TP = m_EX = 0;
            m_Stuffing = 0;
        }
    }
    else
    {
        m_AF = 0;
        m_Stuffing = 0;
        m_DC = m_RA = m_SP = m_PCR = m_OR = m_SF = m_TP = m_EX = 0;
    }
    return m_AF;
}

void xTS_AdaptationField::Print() const
{
    printf("AF: AF=%d DC=%d RA=%d SP=%d PR=%d OR=%d SF=%d TP=%d EX=%d\n",
           m_AF, m_DC, m_RA, m_SP,
           m_PCR, m_OR, m_SF, m_TP, m_EX);

    if (m_PCR)
    {
        printf("PCR=%llu ", (m_PCR_base * xTS::BaseToExtendedClockMultiplier + m_PCR_extension));
    }
    if (m_OR)
    {
        printf("OPCR=%llu ", (m_OPCR_base * xTS::BaseToExtendedClockMultiplier + m_OPCR_extension));
    }
    printf("\n");
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================
void xPES_PacketHeader::Reset()
{
    m_PacketStartCodePrefix = 0;
    m_StreamId = 0;
    m_PacketLength = 0;
    m_HeaderLength = 0;
}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input)
{
    m_PacketStartCodePrefix = ((uint32_t)Input[0] << 16) | ((uint32_t)Input[1] << 8) | Input[2];
    m_StreamId = Input[3];
    m_PacketLength = ((uint16_t)Input[4] << 8) | Input[5];

    m_HeaderLength = xTS::PES_HeaderLength; // Base PES header length

    uint8_t currentStreamId = m_StreamId;

    if (currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_program_stream_map &&
        currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_padding_stream &&
        currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_private_stream_2 &&
        currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_ECM &&
        currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_EMM &&
        currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_program_stream_directory &&
        currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_DSMCC_stream &&
        currentStreamId != (uint8_t)xPES_PacketHeader::eStreamId::eStreamId_ITUT_H222_1_type_E)
    {
        uint8_t pes_header_data_length = Input[6];
        m_HeaderLength += 1 + pes_header_data_length;

        if (pes_header_data_length > 0)
        {
            m_HeaderLength = 9;

            uint8_t flags_byte_2 = Input[7];
            uint8_t pes_header_data_len_byte = Input[8];

            m_HeaderLength = 9 + pes_header_data_len_byte;
        }
    }
    else
    {
        m_HeaderLength = 6;
    }

    return m_PacketStartCodePrefix;
}

void xPES_PacketHeader::Print() const
{
    printf("PES: ");
    printf("PSCP=0x%06X ", m_PacketStartCodePrefix);
    printf("SID=0x%02X ", m_StreamId);
    printf("L=%u ", m_PacketLength);
    printf("HL=%u\n", m_HeaderLength);
}

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================
xPES_Assembler::xPES_Assembler() : m_PID(0), m_Buffer(nullptr), m_BufferSize(0), m_LastContinuityCounter(-1),
                                   m_Started(false)
{
}

xPES_Assembler::~xPES_Assembler()
{
    delete[] m_Buffer;
    m_Buffer = nullptr;
}

void xPES_Assembler::Init(int32_t PID)
{
    m_PID = PID;
    if (m_Buffer)
    {
        delete[] m_Buffer;
    }
    m_Buffer = new uint8_t[200000];
    m_BufferSize = 0;
    m_LastContinuityCounter = -1;
    m_Started = false;
    m_PESH.Reset();
}

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket,
                                                     const xTS_PacketHeader* PacketHeader,
                                                     const xTS_AdaptationField* AdaptationField)
{
    if (PacketHeader->getPID() != m_PID)
    {
        return eResult::UnexpectedPID;
    }

    uint8_t currentCC = PacketHeader->getCC();

    if (m_Started && PacketHeader->hasContinuityCounter() && m_LastContinuityCounter != -1)
    {
        if (((m_LastContinuityCounter + 1) & 0x0F) != currentCC)
        {
            m_Started = false;
            m_BufferSize = 0;
            m_PESH.Reset();
            m_LastContinuityCounter = -1;
            return eResult::StreamPacketLost;
        }
    }
    m_LastContinuityCounter = currentCC;

    uint32_t tsAdaptationFieldLength = 0;
    if (PacketHeader->hasAdaptationField())
    {
        tsAdaptationFieldLength = AdaptationField->getNumBytes();
    }

    uint32_t payloadOffset = xTS::TS_HeaderLength + tsAdaptationFieldLength;
    uint32_t tsPayloadLength = xTS::TS_PacketLength - payloadOffset;

    if (!PacketHeader->hasPayload())
    {
        return eResult::NoPayload;
    }

    if (PacketHeader->getPayloadUnitStartIndicator())
    {
        if (m_Started)
        {
            m_BufferSize = 0;
            m_PESH.Reset();
        }

        if (tsPayloadLength < xTS::PES_HeaderLength)
        {
            m_Started = false;
            return eResult::BufferOverflow;
        }
        m_PESH.Parse(&TransportStreamPacket[payloadOffset]);

        uint32_t pesHeaderLength = m_PESH.getHeaderLength();
        if (tsPayloadLength < pesHeaderLength)
        {
            m_Started = false;
            return eResult::BufferOverflow;
        }

        uint32_t dataToCopyLength = tsPayloadLength - pesHeaderLength;
        if (m_BufferSize + dataToCopyLength > 200000)
        {
            m_Started = false;
            return eResult::BufferOverflow;
        }
        xBufferAppend(&TransportStreamPacket[payloadOffset + pesHeaderLength], dataToCopyLength);
        m_Started = true;
        return eResult::AssemblingStarted;
    }
    else
    {
        if (!m_Started)
        {
            return eResult::UnexpectedPID;
        }

        uint32_t dataToCopyLength = tsPayloadLength;
        if (m_BufferSize + dataToCopyLength > 200000)
        {
            m_Started = false;
            return eResult::BufferOverflow;
        }
        xBufferAppend(&TransportStreamPacket[payloadOffset], dataToCopyLength);

        if (m_PESH.getPacketLength() > 0 && (m_BufferSize + m_PESH.getHeaderLength()) >= (m_PESH.getPacketLength() + 6))
        {
            m_Started = false;
            return eResult::AssemblingFinished;
        }
        else
        {
            return eResult::AssemblingContinue;
        }
    }
}

void xPES_Assembler::xBufferReset()
{
    m_PESH.Reset();
    if (m_Buffer)
    {
        memset(m_Buffer, 0, 200000);
    }
    m_BufferSize = 0;
    m_Started = false;
    m_LastContinuityCounter = -1;
}

void xPES_Assembler::xBufferAppend(const uint8_t* data, uint32_t size)
{
    if (m_Buffer && (m_BufferSize + size) <= 200000)
    {
        memcpy(m_Buffer + m_BufferSize, data, size);
        m_BufferSize += size;
    }
}

void xPES_Assembler::assemblerPes(const uint8_t* TS_PacketBuffer, const xTS_PacketHeader* TS_PacketHeader,
                                  const xTS_AdaptationField* TS_AdaptationField, FILE* File)
{
    xPES_Assembler::eResult result = AbsorbPacket(TS_PacketBuffer, TS_PacketHeader, TS_AdaptationField);
    switch (result)
    {
    case xPES_Assembler::eResult::AssemblingStarted:
        {
            printf("Assembling Started: \n");
            PrintPESH();
            break;
        }
    case xPES_Assembler::eResult::AssemblingContinue:
        {
            break;
        }
    case xPES_Assembler::eResult::AssemblingFinished:
        {
            printf("Assembling Finished: \n");
            printf("PES: PacketLen=%d HeadLen=%d DataLen=%d\n", m_BufferSize + m_PESH.getHeaderLength(),
                   m_PESH.getHeaderLength(), m_BufferSize);
            saveBufferToFile(File);
            xBufferReset();
            break;
        }
    case xPES_Assembler::eResult::StreamPacketLost:
        {
            printf("Stream Packet Lost for PID %d! Resetting assembler.\n", m_PID);
            xBufferReset();
            break;
        }
    case xPES_Assembler::eResult::UnexpectedPID:
        {
            break;
        }
    case xPES_Assembler::eResult::NoPayload:
        {
            break;
        }
    case xPES_Assembler::eResult::BufferOverflow:
        {
            printf("PES Assembler Buffer Overflow for PID %d! Resetting.\n", m_PID);
            xBufferReset();
            break;
        }
    default: break;
    }
}

void xPES_Assembler::saveBufferToFile(FILE* AudioMP2)
{
    if (AudioMP2 && m_Buffer && m_BufferSize > 0)
    {
        fwrite(getPacket(), 1, getNumPacketBytes(), AudioMP2);
    }
}
