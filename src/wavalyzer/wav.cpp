#include "wav.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <cstring>

using namespace wavalyzer;
using namespace std;

namespace wavalyzer {
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

    struct wav_info_t {
        riff_hdr_t          riff_hdr;
        fmt_chunk_t         fmt_chunk;
        chunk_hdr_t         data_chunk_hdr;
    };

    template<typename T>
    T convert_endianness(T) {
        assert(false && "Don't know how to convert this type");
    }

    template<>
    uint32_t convert_endianness<uint32_t>(uint32_t input) {
        return input; // Assuming a little-endian CPU architecture
    }

    template<>
    uint16_t convert_endianness<uint16_t>(uint16_t input) {
        return input; // Assuming a little-endian CPU architecture
    }

    void riff_check_magic(const riff_hdr_t& hdr)
    {
        if (convert_endianness(hdr.chunk_id) != RIFF_MAGIC) {
            throw wav_file_parse_exception("RIFF header magic invalid");
        }

        if (convert_endianness(hdr.format) != WAVE_MAGIC) {
            throw wav_file_parse_exception("RIFF header format magic invalid");
        }
    }

    void wav_file_check_sanity(const wav_info_t& hdr)
    {
        // Check if we're dealing with some weird compression
        const fmt_chunk_t& fmt = hdr.fmt_chunk;
        switch (convert_endianness(fmt.audio_format)) {
            case FORMAT_LPCM: break;
            default:
                throw wav_file_parse_exception("Unsupported audio format (only LPCM is supported)");
        }

        if (convert_endianness(fmt.sample_rate) == 0) {
            throw wav_file_parse_exception("Zero sample rate");
        }

        if (convert_endianness(fmt.num_channels) == 0) {
            throw wav_file_parse_exception("Zero channels");
        }

        size_t bits_per_sample = convert_endianness(fmt.bits_per_sample);
        if (bits_per_sample == 0) {
            throw wav_file_parse_exception("Bits per sample is 0");
        }

        if (bits_per_sample != 8 && bits_per_sample != 16) {
            throw wav_file_parse_exception("Unsupported sample bit depth");
        }
    }

    void wav_file_read_chunks(wav_info_t& destination, istream& file)
    {
        bool found_fmt = false, found_data = false;
        char buffer[sizeof(fmt_chunk_t)];

        while (!file.eof() && (!found_fmt || !found_data)) {
            union {
                chunk_hdr_t hdr;
                char        bytes[sizeof(chunk_hdr_t)];
            } h;

            file.read(h.bytes, sizeof(chunk_hdr_t));
            switch (convert_endianness(h.hdr.chunk_id)) {
            case FMT_MAGIC:
                if (convert_endianness(h.hdr.chunk_size) < FMT_CHUNK_SIZE) {
                    throw wav_file_parse_exception("Unexpected length for FMT chunk (should be at least 16)");
                }

                found_fmt = true;
                file.read(buffer, sizeof(fmt_chunk_t));
                file.ignore(convert_endianness(h.hdr.chunk_size) - FMT_CHUNK_SIZE);

                memcpy(reinterpret_cast<char*>(&destination.fmt_chunk), buffer, sizeof(fmt_chunk_t));

                break;

            case DATA_MAGIC:
                if (!found_fmt) {
                    throw wav_file_parse_exception("We don't support DATA chunks before FMT chunks");
                }
                destination.data_chunk_hdr = h.hdr;
                found_data = true;

                break;

            default:
                // Unknown chunk, skip
                file.ignore(convert_endianness(h.hdr.chunk_size));
            }
        }

        if (!found_fmt) {
            throw wav_file_parse_exception("FMT chunk not found");
        }

        if (!found_data) {
            throw wav_file_parse_exception("Data chunk not found");
        }
    }

}

wav_file::wav_file(std::istream& _file) : file(_file)
{
    union {
        riff_hdr_t hdr;
        char bytes[sizeof(riff_hdr_t)];
    } riff;

    wav_info_t info;

    file.read(riff.bytes, sizeof(riff_hdr_t));
    info.riff_hdr = riff.hdr;
    riff_check_magic(riff.hdr);

    wav_file_read_chunks(info, file);
    wav_file_check_sanity(info);

    fmt_chunk_t& fmt = info.fmt_chunk;

    bytes_per_sample = convert_endianness(fmt.bits_per_sample) / 8;
    channels = convert_endianness(fmt.num_channels);

    if (channels > 1) {
        throw wav_file_parse_exception("We currently do not handle more than 1 channel. Please downmix");
    }

    sample_rate = convert_endianness(fmt.sample_rate);

    // Get the total number of samples from the data chunk header
    size_t total_bytes = info.data_chunk_hdr.chunk_size;

    if (total_bytes % bytes_per_sample != 0) {
        throw wav_file_parse_exception("The total number of bytes in the data is not divisible by the bytes per sample");
    }

    total_samples = total_bytes / bytes_per_sample;

    // At this point, the next data to be read will be raw sample data.
    samples_read = 0;
}

template<>
void wav_file::interpret_samples<8>(const vector<uint8_t>& bytes, vector<sample_t>& destination)
{
    for (uint8_t byte : bytes) {
        destination.push_back(2.0f * (byte / 255.0f) - 1.0f);
    }
}

template<>
void wav_file::interpret_samples<16>(const vector<uint8_t>& bytes, vector<sample_t>& destination)
{
    for (size_t i = 0; i < bytes.size(); i += 2) {
        union {
            int16_t integer;
            uint8_t bytes[2];
        } u;

        u.bytes[0] = bytes[i];
        u.bytes[1] = bytes[i + 1];

        int16_t sample = u.integer;
        sample_t sample_f;

        if (sample < 0) {
            sample_f = static_cast<sample_t>(sample) / (1 << 15);
        } else {
            sample_f = static_cast<sample_t>(sample) / ((1 << 15) - 1);
        }

        destination.push_back(sample_f);
    }
}

void wav_file::read_samples(std::vector<sample_t>& destination, size_t sample_count)
{
    if (sample_count > (total_samples - samples_read)) {
        throw wav_file_parse_exception("The sample count requested is past the end of file");
    }

    vector<uint8_t> bytes;
    size_t byte_count = sample_count * bytes_per_sample;
    bytes.resize(byte_count);
    try {
        file.read(reinterpret_cast<char*>(&bytes[0]), byte_count);
        samples_read += sample_count;
    } catch (exception& e) {
        throw wav_file_parse_exception("Unexpected read error / EOF");
    }

    destination.clear();
    destination.reserve(sample_count);

    if (bytes_per_sample == 1) {
        interpret_samples<8>(bytes, destination);
    } else if (bytes_per_sample == 2) {
        interpret_samples<16>(bytes, destination);
    } else {
        throw wav_file_parse_exception("Unsupported sample bit depth");
    }
}

