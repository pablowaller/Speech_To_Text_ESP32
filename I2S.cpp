#include "I2S.h"
#define SAMPLE_RATE (16000)
#define PIN_I2S_BCLK 26
#define PIN_I2S_LRC 22
#define PIN_I2S_DIN 35

I2S::I2S(MicType micType) {
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_16BIT;  // Cambiado de 32 a 16 bits
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,  // 16 kHz
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // Mono
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_BCLK,
        .ws_io_num = PIN_I2S_LRC,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_I2S_DIN
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_0);  // Limpiar buffer DMA
}

int I2S::Read(char* data, int numData) {
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_NUM_0, (void*)data, numData, &bytesRead, portMAX_DELAY);

    if (result == ESP_OK) {
        // 🔹 Mostrar valores en Serial Plotter
        int16_t *samples = (int16_t *)data;
        Serial.println(samples[0]);  // Imprime la primera muestra de cada lectura
    }

    return (result == ESP_OK) ? bytesRead : -1;
}


int I2S::GetBitPerSample() {
    return (int)BITS_PER_SAMPLE;
}
