#include "mp3.h"
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "ac97.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char *s);

#define DRMP3_MALLOC(sz) kmalloc(sz)
#define DRMP3_REALLOC(p, sz) krealloc(p, sz)
#define DRMP3_FREE(p) kfree(p)
#define DRMP3_COPY_MEMORY(dst, src, sz) memcpy(dst, src, sz)
#define DRMP3_ZERO_MEMORY(dst, sz) memset(dst, 0, sz)

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include "dr_mp3.h"

static uint8_t *current_mp3_pcm_data = NULL;
static uint32_t current_mp3_sample_rate = 44100;
static uint32_t current_mp3_channels = 2;

void mp3_stop(void) {
    ac97_stop_playback();
    if (current_mp3_pcm_data) {
        kfree(current_mp3_pcm_data);
        current_mp3_pcm_data = NULL;
    }
}

void mp3_get_progress(uint32_t *pos_ms, uint32_t *dur_ms) {
    extern uint32_t ac97_large_pcm_total;
    extern uint32_t ac97_samples_played;
    
    uint64_t samples_per_sec = (uint64_t)current_mp3_sample_rate * current_mp3_channels;
    if (samples_per_sec == 0) samples_per_sec = 88200; // fallback
    
    if (dur_ms) {
        *dur_ms = (uint32_t)(((uint64_t)ac97_large_pcm_total * 1000ULL) / samples_per_sec);
    }
    if (pos_ms) {
        // ac97_samples_played tracks completed BDL entries accurately
        uint32_t played = ac97_samples_played;
        
        // Clamp to total
        if (played > ac97_large_pcm_total)
            played = ac97_large_pcm_total;
        
        *pos_ms = (uint32_t)(((uint64_t)played * 1000ULL) / samples_per_sec);
    }
}

int mp3_play(const uint8_t *data, uint32_t size) {
    if (!data || size == 0) {
        print_serial("MP3: Data too small\n");
        return -1;
    }
    
    mp3_stop();

    drmp3 mp3;
    if (!drmp3_init_memory(&mp3, data, size, NULL)) {
        print_serial("MP3: Failed to init memory\n");
        return -1;
    }

    uint64_t frame_count = drmp3_get_pcm_frame_count(&mp3);
    uint32_t sample_rate = mp3.sampleRate;
    uint32_t channels = mp3.channels;
    
    current_mp3_sample_rate = sample_rate;
    current_mp3_channels = channels;

    char buf[16];
    print_serial("MP3: ");
    k_itoa(sample_rate, buf); print_serial(buf); print_serial("Hz ");
    k_itoa(channels, buf); print_serial(buf); print_serial("ch ");
    k_itoa((int)frame_count, buf); print_serial(buf); print_serial(" frames\n");

    uint32_t pcm_size = (uint32_t)frame_count * channels * sizeof(int16_t);
    // Limit to ~33MB just to be safe (256 * 65535 * 2)
    uint32_t max_pcm_size = 256 * 65535 * 2; 
    if (pcm_size > max_pcm_size) {
        pcm_size = max_pcm_size;
    }

    uint32_t frames_to_read = pcm_size / (channels * sizeof(int16_t));
    current_mp3_pcm_data = kmalloc(pcm_size);
    if (!current_mp3_pcm_data) {
        print_serial("MP3: Failed to allocate PCM buffer\n");
        drmp3_uninit(&mp3);
        return -1;
    }

    drmp3_uint64 frames_read = drmp3_read_pcm_frames_s16(&mp3, frames_to_read, (drmp3_int16*)current_mp3_pcm_data);
    
    if (frames_read > 0) {
        if (sample_rate != 48000 || channels != 2) {
            uint32_t out_frames = (uint32_t)((uint64_t)frames_read * 48000 / sample_rate);
            uint32_t out_size = out_frames * 2 * sizeof(int16_t);
            int16_t *out_data = kmalloc(out_size);
            
            if (out_data) {
                int16_t *src_data = (int16_t *)current_mp3_pcm_data;
                for (uint32_t i = 0; i < out_frames; i++) {
                    uint64_t src_idx = (uint64_t)i * sample_rate * 65536 / 48000; 
                    uint32_t idx_int = (uint32_t)(src_idx >> 16);
                    uint32_t idx_frac = (uint32_t)(src_idx & 0xFFFF);
                    
                    if (idx_int >= frames_read - 1) {
                        out_data[i*2] = src_data[idx_int * channels];
                        out_data[i*2+1] = src_data[idx_int * channels + (channels > 1 ? 1 : 0)];
                        continue;
                    }
                    
                    for (int c = 0; c < 2; c++) {
                        int src_c = (channels > 1) ? c : 0;
                        int s1 = src_data[idx_int * channels + src_c];
                        int s2 = src_data[(idx_int + 1) * channels + src_c];
                        int out_s = s1 + (int)(((int64_t)(s2 - s1) * idx_frac) / 65536);
                        out_data[i*2 + c] = (int16_t)out_s;
                    }
                }
                
                kfree(current_mp3_pcm_data);
                current_mp3_pcm_data = (uint8_t *)out_data;
                current_mp3_sample_rate = 48000;
                current_mp3_channels = 2;
                
                ac97_play_pcm(current_mp3_pcm_data, out_size, 48000, 16, 2);
            } else {
                print_serial("MP3: Resample alloc failed\n");
            }
        } else {
            ac97_play_pcm(current_mp3_pcm_data, (uint32_t)frames_read * 2 * sizeof(int16_t), 48000, 16, 2);
        }
    }

    drmp3_uninit(&mp3);

    return 0;
}
