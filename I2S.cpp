#include "I2S.h"
#define SAMPLE_RATE (16000)
#define PIN_I2S_BCLK 26
#define PIN_I2S_LRC 22
#define PIN_I2S_DIN 35

I2S::I2S(MicType micType) {
  if (micType == M5GO || micType == M5STACKFIRE) {
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 2,
      .dma_buf_len = 1024
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_6);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_STEREO);
    i2s_adc_enable(I2S_NUM_0);
  } else if (micType == ADMP441 || micType == ICS43434) {
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_32BIT;
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 16,
      .dma_buf_len = 60
    };
    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = PIN_I2S_BCLK;
    pin_config.ws_io_num = PIN_I2S_LRC;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = PIN_I2S_DIN;
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_STEREO);
  } else if (micType == INMP441) {                // Configuración para INMP441
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_32BIT;  // INMP441 es de 24 bits, pero 32 bits también funciona
    i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // Solo canal izquierdo (L/R en GND)
      .communication_format = I2S_COMM_FORMAT_I2S,  // Formato I2S estándar
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,  // Número de buffers
      .dma_buf_len = 1024   // Tamaño de cada buffer
    };
    i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_I2S_BCLK,  // Pin 26 (BCLK)
      .ws_io_num = PIN_I2S_LRC,    // Pin 22 (LRC)
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = PIN_I2S_DIN  // Pin 35 (DIN)
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_MONO);  // Mono, ya que solo usamos el canal izquierdo
  }
}

int I2S::Read(char *data, int numData) {
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_NUM_0, (void *)data, numData, &bytesRead, portMAX_DELAY);

  if (result == ESP_OK) {
    // 🔹 Mostrar valores en Serial Plotter
    int32_t *samples = (int32_t *)data;
    for (int i = 0; i < 10; i++) {      // Imprime las primeras 10 muestras para depuración
      Serial.println(samples[i] >> 8);  // Ajustar a 24 bits
    }
  }

  return (result == ESP_OK) ? bytesRead : -1;
}

int I2S::GetBitPerSample() {
  return (int)BITS_PER_SAMPLE;
}