#include "Audio.h"

Audio::Audio(MicType micType) {
    wavData = new char*[wavDataSize / dividedWavDataSize];
    for (int i = 0; i < wavDataSize / dividedWavDataSize; ++i) wavData[i] = new char[dividedWavDataSize];
    i2s = new I2S(micType);
}

Audio::~Audio() {
    for (int i = 0; i < wavDataSize / dividedWavDataSize; ++i) delete[] wavData[i];
    delete[] wavData;
    delete i2s;
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
    header[16] = 0x10;  // linear PCM
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;
    header[20] = 0x01;  // linear PCM
    header[21] = 0x00;
    header[22] = 0x01;  // monoral
    header[23] = 0x00;
    header[24] = 0x80;  // sampling rate 16000
    header[25] = 0x3E;
    header[26] = 0x00;
    header[27] = 0x00;
    header[28] = 0x00;  // Byte/sec = 16000x2x1 = 32000
    header[29] = 0x7D;
    header[30] = 0x00;
    header[31] = 0x00;
    header[32] = 0x02;  // 16bit monoral
    header[33] = 0x00;
    header[34] = 0x10;  // 16bit
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
    CreateWavHeader(paddedHeader, wavDataSize);
    int bitBitPerSample = i2s->GetBitPerSample();
    if (bitBitPerSample == 16) {
        for (int j = 0; j < wavDataSize / dividedWavDataSize; ++j) {
            i2s->Read(i2sBuffer, i2sBufferSize / 2);
            for (int i = 0; i < i2sBufferSize / 8; ++i) {
                wavData[j][2 * i] = i2sBuffer[4 * i + 2];
                wavData[j][2 * i + 1] = i2sBuffer[4 * i + 3];
            }
        }
    }
    else if (bitBitPerSample == 32) {
        for (int j = 0; j < wavDataSize / dividedWavDataSize; ++j) {
            i2s->Read(i2sBuffer, i2sBufferSize);
            for (int i = 0; i < i2sBufferSize / 8; ++i) {
                // Ajustar los datos de 32 bits a 24 bits (eliminar los 8 bits superiores)
                int32_t sample = ((int32_t*)i2sBuffer)[i];
                sample = sample >> 8;  // Desplazar 8 bits a la derecha para obtener un valor de 24 bits

                // Almacenar los datos en wavData
                wavData[j][2 * i] = (sample >> 8) & 0xFF;  // Byte alto
                wavData[j][2 * i + 1] = sample & 0xFF;     // Byte bajo
            }
        }
    }
}