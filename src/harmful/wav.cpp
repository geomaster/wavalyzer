#include "wav.hpp"
#include <fstream>
#include <iostream>
using namespace std;

const uint32_t RIFF_MAGIC = 0x46464952;
const uint32_t FMT_MAGIC = 0x20746d66;
const uint32_t DATA_MAGIC = 0x61746164;
const uint32_t WAVE_MAGIC = 0x45564157;
const size_t   FMT_CHUNK_SIZE = 16;

enum audio_format_t {
    FORMAT_LPCM = 1
};

struct __attribute__((packed)) riff_hdr_t {
    uint32_t            chunk_id;
    uint32_t            chunk_size;
    uint32_t            format;
};

struct __attribute__((packed)) chunk_hdr_t {
    uint32_t            chunk_id;
    uint32_t            chunk_size;
};

struct __attribute__((packed)) fmt_chunk_t {
    uint16_t            audio_format;
    uint16_t            num_channels;
    uint32_t            sample_rate;
    uint32_t            byte_rate;
    uint16_t            block_align;
    uint16_t            bits_per_sample;
};

struct __attribute__((packed)) wav_file_hdr_t {
    riff_hdr_t          riff_hdr;
    chunk_hdr_t         fmt_chunk_hdr;
    fmt_chunk_t         fmt_chunk;
    chunk_hdr_t         data_chunk_hdr;
};

int harmful::write_wav(const string& filename, const vector<float>& samples, int sample_rate)
{
    ofstream f(filename);
    if (!f.good()) {
        cerr << "Cannot open output file." << endl;
        return -3;
    }

    int total_sample_bytes = samples.size() * 2;

    wav_file_hdr_t hdr;
    hdr.riff_hdr.chunk_id = RIFF_MAGIC;
    hdr.riff_hdr.chunk_size = 36 + total_sample_bytes;
    hdr.riff_hdr.format = WAVE_MAGIC;

    hdr.fmt_chunk_hdr.chunk_id = FMT_MAGIC;
    hdr.fmt_chunk_hdr.chunk_size = FMT_CHUNK_SIZE;

    hdr.fmt_chunk.audio_format = FORMAT_LPCM;
    hdr.fmt_chunk.num_channels = 1;
    hdr.fmt_chunk.sample_rate = sample_rate;
    hdr.fmt_chunk.byte_rate = sample_rate * 2;
    hdr.fmt_chunk.block_align = 2;
    hdr.fmt_chunk.bits_per_sample = 16;

    hdr.data_chunk_hdr.chunk_id = DATA_MAGIC;
    hdr.data_chunk_hdr.chunk_size = total_sample_bytes;

    f.write(reinterpret_cast<char*>(&hdr), sizeof(hdr));

    for (float sample : samples) {
        // Ignoring -32768 for simplicity
        int16_t sample_int = static_cast<int16_t>(sample * 32767.0f);
        f.write(reinterpret_cast<char*>(&sample_int), 2);
    }

    f.close();

    return 0;
}
