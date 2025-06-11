#include "tsCommon.h"
#include "tsTransportStream.h"
#include <cstdio>
#include <cstdlib>

static constexpr uint16_t PID_AUDIO_MP2 = 136;

int main(int argc, char *argv[ ], char *envp[ ]) {
    FILE* TransportStreamFile = fopen("example_new.ts", "rb");
    FILE* AudioMP2 = fopen("PID136.mp2", "wb");

    if (TransportStreamFile == NULL) {
        printf("Error: Nie można otworzyć pliku strumienia transportowego\n");
        return EXIT_FAILURE;
    }

    if (AudioMP2 == NULL) {
        printf("Error: Nie można utworzyć wyjściowego pliku audio'\n");
        fclose(TransportStreamFile);
        return EXIT_FAILURE;
    }

    uint8_t TS_PacketBuffer[xTS::TS_PacketLength];
    xTS_PacketHeader TS_PacketHeader;
    xTS_AdaptationField TS_AdaptationField;

    xPES_Assembler PES_Assembler136;
    PES_Assembler136.Init(PID_AUDIO_MP2);

    int32_t TS_PacketId = 0;
    const int32_t max_packets_to_parse = 10000;

    printf("Rozpoczynanie parsowania MPEG-TS dla PID %u...\n", PID_AUDIO_MP2);
    printf("Przetwarzanie do %d pakietów...\n\n", max_packets_to_parse);

    while (true) {
        if (TS_PacketId >= max_packets_to_parse) {
            printf("Osiągnięto limit %d przetworzonych pakietów. Zatrzymywanie parsowania.\n", max_packets_to_parse);
            break;
        }

        size_t NumRead = fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, TransportStreamFile);

        if (NumRead != xTS::TS_PacketLength) {
            if (feof(TransportStreamFile)) {
                printf("Osiągnięto koniec pliku strumienia transportowego.\n");
            } else {
                printf("Błąd: Niekompletny odczyt pakietu lub błąd pliku. Odczytano %zu bajtów zamiast %u.\n", NumRead, xTS::TS_PacketLength);
            }
            break;
        }

        TS_PacketHeader.Reset();
        TS_PacketHeader.Parse(TS_PacketBuffer);

        if (TS_PacketHeader.getSyncByte() == 0x47 && TS_PacketHeader.getPID() == PID_AUDIO_MP2) {
            TS_AdaptationField.Reset();

            if (TS_PacketHeader.hasAdaptationField()) {
                TS_AdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAFC());
            }

            printf("Pakiet TS %010d: ", TS_PacketId);
            TS_PacketHeader.Print();

            if (TS_PacketHeader.hasAdaptationField()) {
                TS_AdaptationField.Print();
            }

            PES_Assembler136.assemblerPes(TS_PacketBuffer, &TS_PacketHeader, &TS_AdaptationField, AudioMP2);
        }

        TS_PacketId++;
    }

    fclose(TransportStreamFile);
    fclose(AudioMP2);

    printf("\nParsowanie zakończone. Dane audio zapisane.\n");

    return EXIT_SUCCESS;
}