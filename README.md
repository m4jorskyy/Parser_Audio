# Parser MPEG-TS

Aplikacja w C++ do parsowania plików MPEG Transport Stream i ekstraktowania danych audio z określonych strumieni PID (Program Identifier).

## Opis

Parser przetwarza pliki MPEG-TS i ekstraktuje dane audio MP2 z określonego PID (domyślnie 136). Aplikacja czyta pakiety Transport Stream, analizuje ich nagłówki i pola adaptacyjne, a następnie składa pakiety PES (Packetized Elementary Stream) w celu wyodrębnienia danych audio.

## Funkcjonalności

- Parsowanie nagłówków pakietów MPEG-TS
- Analiza pól adaptacyjnych
- Składanie pakietów PES
- Ekstraktowanie strumienia audio do formatu MP2
- Konfigurowalne limity przetwarzania pakietów
- Szczegółowe logowanie postępu parsowania

## Struktura plików

```
parser/
├── TS_parser.cpp           # Główny punkt wejścia aplikacji
├── tsCommon.h              # Wspólne definicje i narzędzia do zamiany bajtów
├── tsTransportStream.h     # Deklaracje klas Transport Stream
├── tsTransportStream.cpp   # Implementacja Transport Stream
└── CMakeLists.txt          # Konfiguracja budowania CMake
```

## Główne komponenty

### xTS_PacketHeader
Obsługuje parsowanie i analizę nagłówków pakietów MPEG-TS:
- Walidacja bajtu synchronizacji
- Ekstraktowanie PID
- Kontrola pola adaptacyjnego
- Śledzenie licznika ciągłości

### xTS_AdaptationField
Przetwarza dane pola adaptacyjnego gdy są obecne:
- Obliczanie długości pola
- Ekstraktowanie PCR (Program Clock Reference)
- Parsowanie pól opcjonalnych

### xPES_PacketHeader
Zarządza parsowaniem nagłówków pakietów PES:
- Walidacja kodu startowego pakietu
- Identyfikacja ID strumienia
- Obliczanie długości nagłówka

### xPES_Assembler
Składa kompletne pakiety PES z ładunków pakietów TS:
- Rekonstrukcja wielopakietowych PES
- Sprawdzanie ciągłości
- Zarządzanie buforem
- Ekstraktowanie danych audio

## Użycie

### Wymagania
- Kompilator C++ z obsługą C++11
- CMake 3.10 lub nowszy
- Plik wejściowy MPEG-TS o nazwie `example_new.ts`

### Budowanie
```bash
mkdir build
cd build
cmake ..
make
```

### Uruchamianie
```bash
./parser
```

Aplikacja będzie:
1. Czytać z pliku `example_new.ts` w bieżącym katalogu
2. Ekstraktować dane audio z PID 136
3. Zapisywać audio MP2 do `PID136.mp2`
4. Przetwarzać maksymalnie 10 000 pakietów domyślnie

## Konfiguracja

Kluczowe parametry można modyfikować w `TS_parser.cpp`:

```cpp
static constexpr uint16_t PID_AUDIO_MP2 = 136;
const int32_t max_packets_to_parse = 10000;
```

Nazwy plików wejściowego i wyjściowego:
```cpp
FILE* TransportStreamFile = fopen("example_new.ts", "rb");
FILE* AudioMP2 = fopen("PID136.mp2", "wb");
```

## Format wyjścia

Parser generuje szczegółowe dane wyjściowe w konsoli:
- Postęp przetwarzania pakietów
- Informacje o nagłówkach Transport Stream
- Szczegóły pól adaptacyjnych
- Status składania PES
- Warunki błędów i odzyskiwanie

Przykład wyjścia:
```
Rozpoczynanie parsowania MPEG-TS dla PID 136...
Przetwarzanie do 10000 pakietów...

Pakiet TS 0000000042: TS: SB=71 E=0 S=1 T=0 PID=136 TSC=0 AFC=1 CC=0
Assembling Started: 
PES: PSCP=0x000001 SID=0xC0 L=1234 HL=9
```

## Szczegóły techniczne

### Struktura pakietu MPEG-TS
- Rozmiar pakietu: 188 bajtów
- Nagłówek: 4 bajty
- Opcjonalne pole adaptacyjne
- Dane ładunku

### Obsługiwane typy strumieni
- Strumienie audio (MP2/MP3)
- Konfigurowalne targetowanie PID
- Rekonstrukcja pakietów PES

### Obsługa błędów
- Wykrywanie błędów I/O plików
- Walidacja ciągłości pakietów
- Ochrona przed przepełnieniem bufora
- Odzyskiwanie po błędnych pakietach

## Zależności

Projekt używa standardowych bibliotek C++:
- `<cstdio>` - Operacje na plikach
- `<cstdlib>` - Standardowe narzędzia
- `<cstdint>` - Typy całkowite o stałej szerokości
- `<cstring>` - Operacje na pamięci

Optymalizacje specyficzne dla platformy do zamiany bajtów na architekturach x86/x64.

## Licencja

Projekt dostępny na licencji MIT. Zobacz plik `LICENSE` dla szczegółów.

## Kontakt

- **Autor**: Igor Suchodolski
- **Email**: igor.suchodolskii@gmail.com
