#include "Audio.h"

static const int i2sBufferSize = 4096; // Reducir de 12000 a 4096 si es necesario

Audio::Audio(MicType micType) {
    // Asegurar que wavData es NULL antes de asignar memoria
    wavData = nullptr;

    // Intentar asignar memoria y verificar si fue exitoso
    wavData = new char*[wavDataSize / dividedWavDataSize];
    if (!wavData) {
        Serial.println("Error: No se pudo asignar memoria para wavData.");
        return;
    }

    for (int i = 0; i < wavDataSize / dividedWavDataSize; ++i) {
        wavData[i] = new char[dividedWavDataSize];
        if (!wavData[i]) {
            Serial.println("Error: No se pudo asignar memoria para wavData[i].");
            return;
        }
    }

    i2s = new I2S(micType);
    if (!i2s) {
        Serial.println("Error: No se pudo inicializar I2S.");
        return;
    }
}

Audio::~Audio() {
    if (wavData) {
        for (int i = 0; i < wavDataSize / dividedWavDataSize; ++i) {
            if (wavData[i]) {
                delete[] wavData[i];
            }
        }
        delete[] wavData;
    }
    if (i2s) {
        delete i2s;
    }
}

void Audio::CreateWavHeader(byte* header, int waveDataSize) {
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;
    header[4] = (byte)(fileSizeMinus8 & 0xFF);
    header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
    header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
    header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 0x10;
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;
    header[20] = 0x01;
    header[21] = 0x00;
    header[22] = 0x01;
    header[23] = 0x00;
    header[24] = 0x80;
    header[25] = 0x3E;
    header[26] = 0x00;
    header[27] = 0x00;
    header[28] = 0x00;
    header[29] = 0x7D;
    header[30] = 0x00;
    header[31] = 0x00;
    header[32] = 0x02;
    header[33] = 0x00;
    header[34] = 0x10;
    header[35] = 0x00;
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = (byte)(waveDataSize & 0xFF);
    header[41] = (byte)((waveDataSize >> 8) & 0xFF);
    header[42] = (byte)((waveDataSize >> 16) & 0xFF);
    header[43] = (byte)((waveDataSize >> 24) & 0xFF);
}

void Audio::Record() {
    if (!wavData) {
        Serial.println("Error: wavData no ha sido inicializado.");
        return;
    }

    CreateWavHeader(paddedHeader, wavDataSize);
    Serial.println("Grabando audio...");

    int bitBitPerSample = i2s->GetBitPerSample();
    for (int j = 0; j < wavDataSize / dividedWavDataSize; ++j) {
        int bytesRead = i2s->Read(i2sBuffer, i2sBufferSize);
        if (bytesRead <= 0) {
            Serial.println("Error: No se pudo leer datos del micrófono.");
            continue;
        }

        for (int i = 0; i < i2sBufferSize / 4; ++i) {  
            int32_t sample32 = (i2sBuffer[4 * i] << 24) | (i2sBuffer[4 * i + 1] << 16) |
                               (i2sBuffer[4 * i + 2] << 8) | i2sBuffer[4 * i + 3];
            int16_t sample16 = sample32 >> 16;  
            wavData[j][2 * i] = sample16 & 0xFF;
            wavData[j][2 * i + 1] = (sample16 >> 8) & 0xFF;
        }
    }

    // 🔹 Imprimir los primeros valores de audio para depuración
    Serial.println("Primeros valores de audio capturados:");
    for (int i = 0; i < 20; i++) {
        Serial.print(wavData[0][i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}
