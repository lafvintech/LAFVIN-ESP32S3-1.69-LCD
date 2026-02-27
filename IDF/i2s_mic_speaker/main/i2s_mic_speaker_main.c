/*
 * I2S Mic & Speaker Demo - Echo Test
 *
 * Hardware:
 *   - ESP32-S3 (N16R8)
 *   - ICS-43434 I2S MEMS Microphone
 *   - MAX98357 I2S DAC Amplifier
 *
 * Function:
 *   Records audio from the ICS-43434 microphone and plays it back
 *   through the MAX98357 speaker in real-time (echo/loopback).
 *
 * Pin connections:
 *   Microphone (ICS-43434):
 *     WS   -> GPIO 40
 *     SCK  -> GPIO 41
 *     SD   -> GPIO 42
 *
 *   Speaker (MAX98357):
 *     DIN  -> GPIO 45
 *     BCLK -> GPIO 46
 *     LRCLK-> GPIO 47
 *
 * I2S Configuration:
 *   - Sample rate: 16000 Hz
 *   - Bit depth:   32-bit RX (24-bit valid), 16-bit TX
 *   - Channel:     Mono
 *   - Mode:        I2S Standard (Philips)
 * (LAFVIN TEAM)
 * Author: Night + Claude 
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "i2s_echo";

/* ==================== Hardware Pin Definitions ==================== */

/* ICS-43434 Microphone (I2S RX) */
#define MIC_WS_PIN      40      /* Word Select (L/R clock) */
#define MIC_SCK_PIN     41      /* Bit clock */
#define MIC_SD_PIN      42      /* Serial data (input from mic) */

/* MAX98357 Speaker (I2S TX) */
#define SPK_DIN_PIN     45      /* Serial data (output to amp) */
#define SPK_BCLK_PIN    46      /* Bit clock */
#define SPK_LRCLK_PIN   47      /* L/R clock (word select) */

/* ==================== Audio Parameters ==================== */
#define SAMPLE_RATE     16000   /* 16 kHz sample rate */

/*
 * ICS-43434 outputs 24-bit data in a 32-bit frame (I2S Philips).
 * We receive as 32-bit, then extract the useful bits manually.
 * MAX98357 accepts 16-bit or 32-bit, we send 16-bit to it.
 */
#define MIC_SAMPLE_BITS I2S_DATA_BIT_WIDTH_32BIT
#define SPK_SAMPLE_BITS I2S_DATA_BIT_WIDTH_16BIT
#define SLOT_MODE       I2S_SLOT_MODE_MONO

/*
 * DMA buffer: 512 samples per buffer, 6 buffers in the DMA ring.
 * Smaller frames reduce latency; more descriptors improve stability.
 */
#define DMA_FRAME_NUM   512
#define DMA_DESC_NUM    6

/* Mic DMA buffer: 32-bit per sample */
#define MIC_BUF_SIZE    (DMA_FRAME_NUM * sizeof(int32_t))
/* Speaker DMA buffer: 16-bit per sample */
#define SPK_BUF_SIZE    (DMA_FRAME_NUM * sizeof(int16_t))

/*
 * Delay buffer: stores audio data for delayed playback.
 * At 16kHz mono 16-bit, 1 second = 16000 samples = 32000 bytes.
 * 3 seconds = 96000 bytes (~94KB), fits comfortably in PSRAM or internal RAM.
 */
#define DELAY_SECONDS   3
#define DELAY_BUF_SIZE  (SAMPLE_RATE * sizeof(int16_t) * DELAY_SECONDS)

/* Channel handles for mic (RX) and speaker (TX) */
static i2s_chan_handle_t rx_chan = NULL;  /* Microphone input */
static i2s_chan_handle_t tx_chan = NULL;  /* Speaker output */

/* Saturating cast: clamp int32 to int16 range to prevent hard clipping */
static inline int16_t clamp16(int32_t x)
{
    if (x >  32767) return  32767;
    if (x < -32768) return -32768;
    return (int16_t)x;
}

/**
 * @brief Initialize the ICS-43434 microphone as an I2S RX channel.
 *
 * Uses I2S port 0 in standard Philips mode. The ICS-43434 outputs
 * 24-bit data left-justified in a 32-bit frame, so we configure
 * 32-bit width to receive the full frame and extract bits manually.
 *
 * @return ESP_OK on success
 */
static esp_err_t mic_init(void)
{
    /* Step 1: Create a new I2S RX channel on I2S port 0 */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_frame_num = DMA_FRAME_NUM;
    chan_cfg.dma_desc_num = DMA_DESC_NUM;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, NULL, &rx_chan), TAG, "Failed to create RX channel");

    /* Step 2: Configure standard I2S mode (Philips format, 32-bit to match ICS-43434) */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(MIC_SAMPLE_BITS, SLOT_MODE),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,       /* ICS-43434 doesn't need MCLK */
            .bclk = MIC_SCK_PIN,
            .ws   = MIC_WS_PIN,
            .dout = I2S_GPIO_UNUSED,        /* RX only, no data output */
            .din  = MIC_SD_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /*
     * ESP32-S3 quirk: the default PHILIPS_SLOT macro sets slot_mask to
     * I2S_STD_SLOT_BOTH even in mono mode, causing the DMA buffer to
     * contain interleaved L/R data (valid + zero samples).  Override
     * to LEFT only so we get pure mono data matching our L/R=GND mic.
     */
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(rx_chan, &std_cfg), TAG, "Failed to init RX STD mode");

    /* Step 3: Enable the channel to start receiving */
    ESP_RETURN_ON_ERROR(i2s_channel_enable(rx_chan), TAG, "Failed to enable RX channel");

    ESP_LOGI(TAG, "Microphone (ICS-43434) initialized on I2S0 RX");
    return ESP_OK;
}

/**
 * @brief Initialize the MAX98357 speaker as an I2S TX channel.
 *
 * Uses I2S port 1 in standard Philips mode. The MAX98357 accepts
 * standard I2S data and handles DAC conversion + amplification internally.
 *
 * @return ESP_OK on success
 */
static esp_err_t speaker_init(void)
{
    /* Step 1: Create a new I2S TX channel on I2S port 1 */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    chan_cfg.dma_frame_num = DMA_FRAME_NUM;
    chan_cfg.dma_desc_num = DMA_DESC_NUM;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &tx_chan, NULL), TAG, "Failed to create TX channel");

    /* Step 2: Configure standard I2S mode (Philips format, 16-bit for MAX98357) */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(SPK_SAMPLE_BITS, SLOT_MODE),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,       /* MAX98357 doesn't need MCLK */
            .bclk = SPK_BCLK_PIN,
            .ws   = SPK_LRCLK_PIN,
            .dout = SPK_DIN_PIN,
            .din  = I2S_GPIO_UNUSED,        /* TX only, no data input */
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(tx_chan, &std_cfg), TAG, "Failed to init TX STD mode");

    /* Step 3: Enable the channel to start transmitting */
    ESP_RETURN_ON_ERROR(i2s_channel_enable(tx_chan), TAG, "Failed to enable TX channel");

    ESP_LOGI(TAG, "Speaker (MAX98357) initialized on I2S1 TX");
    return ESP_OK;
}

/**
 * @brief Echo task with 3-second delay.
 *
 * Uses a ring buffer to store audio. New mic data is written at the
 * write pointer, while the speaker reads from a position that is
 * DELAY_SECONDS behind. Once the buffer fills up for the first time,
 * delayed playback begins automatically.
 */
static void echo_task(void *arg)
{
    /* Mic buffer: 32-bit samples from ICS-43434 */
    int32_t *mic_buf = heap_caps_malloc(MIC_BUF_SIZE, MALLOC_CAP_DMA);
    /* Speaker buffer: 16-bit samples for MAX98357 */
    int16_t *spk_buf = heap_caps_malloc(SPK_BUF_SIZE, MALLOC_CAP_DMA);
    if (!mic_buf || !spk_buf) {
        ESP_LOGE(TAG, "Failed to allocate DMA buffers");
        vTaskDelete(NULL);
        return;
    }

    /* Allocate the delay ring buffer (prefer PSRAM if available) */
    int16_t *delay_buf = heap_caps_malloc(DELAY_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!delay_buf) {
        /* Fall back to internal RAM */
        delay_buf = heap_caps_malloc(DELAY_BUF_SIZE, MALLOC_CAP_DMA);
    }
    if (!delay_buf) {
        ESP_LOGE(TAG, "Failed to allocate delay buffer (%d bytes)", DELAY_BUF_SIZE);
        free(mic_buf);
        free(spk_buf);
        vTaskDelete(NULL);
        return;
    }
    memset(delay_buf, 0, DELAY_BUF_SIZE);

    size_t delay_buf_samples = DELAY_BUF_SIZE / sizeof(int16_t);  /* Total samples in ring */
    size_t write_pos = 0;       /* Current write position in ring buffer (in samples) */
    size_t samples_written = 0; /* Total samples written so far */
    bool delay_ready = false;   /* True once we've buffered enough for the delay */

    /* Silence buffer for speaker output before delay kicks in */
    int16_t *silence = heap_caps_calloc(DMA_FRAME_NUM, sizeof(int16_t), MALLOC_CAP_DMA);

    ESP_LOGI(TAG, "Echo task started (%d second delay, buffer %d bytes)", DELAY_SECONDS, DELAY_BUF_SIZE);

    size_t bytes_read = 0;
    size_t bytes_written = 0;

    while (1) {
        /* Read 32-bit samples from the microphone */
        esp_err_t ret = i2s_channel_read(rx_chan, mic_buf, MIC_BUF_SIZE, &bytes_read, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Mic read error: %s", esp_err_to_name(ret));
            continue;
        }

        int sample_count = bytes_read / sizeof(int32_t);

        /*
         * Convert 32-bit mic data to 16-bit for the delay buffer.
         * ICS-43434 outputs 24-bit left-justified in a 32-bit frame;
         * right-shift by 16 extracts the top 16 bits (D23..D8) as int32_t.
         * clamp16() saturates to [-32768, 32767] before storing, preventing
         * int16_t overflow on loud transients (e.g. blowing into the mic)
         * which would otherwise cause waveform discontinuities and may
         * trigger an undervoltage reset via the MAX98357 output stage.
         */
        for (int i = 0; i < sample_count; i++) {
            int16_t sample = clamp16(mic_buf[i] >> 16);
            delay_buf[write_pos] = sample;
            write_pos = (write_pos + 1) % delay_buf_samples;
        }

        samples_written += sample_count;

        /* Check if we've accumulated enough data for the delay */
        if (!delay_ready && samples_written >= delay_buf_samples) {
            delay_ready = true;
            ESP_LOGI(TAG, "Delay buffer full, playback starting");
        }

        if (delay_ready) {
            /*
             * Read from write_pos which points to the oldest data,
             * exactly DELAY_SECONDS behind the latest write.
             */
            size_t read_pos = write_pos;
            for (int i = 0; i < sample_count; i++) {
                spk_buf[i] = delay_buf[read_pos];
                read_pos = (read_pos + 1) % delay_buf_samples;
            }
            i2s_channel_write(tx_chan, spk_buf, sample_count * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        } else {
            /* Not enough data yet, output silence to keep the DAC happy */
            i2s_channel_write(tx_chan, silence, sample_count * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        }
    }

    free(mic_buf);
    free(spk_buf);
    free(delay_buf);
    free(silence);
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "I2S Mic & Speaker Echo Demo");
    ESP_LOGI(TAG, "Mic: ICS-43434 (WS=%d, SCK=%d, SD=%d)", MIC_WS_PIN, MIC_SCK_PIN, MIC_SD_PIN);
    ESP_LOGI(TAG, "Spk: MAX98357  (DIN=%d, BCLK=%d, LRCLK=%d)", SPK_DIN_PIN, SPK_BCLK_PIN, SPK_LRCLK_PIN);

    /* Initialize microphone and speaker */
    ESP_ERROR_CHECK(mic_init());
    ESP_ERROR_CHECK(speaker_init());

    /*
     * Start the echo task on core 1 with elevated priority.
     * Audio processing benefits from a dedicated core to avoid
     * jitter caused by other tasks (WiFi, logging, etc.).
     * Stack size 4KB is sufficient for this simple loopback.
     */
    xTaskCreatePinnedToCore(echo_task, "echo", 4096, NULL, 5, NULL, 1);
}
