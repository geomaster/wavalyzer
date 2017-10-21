#pragma once
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <exception>

namespace wavalyzer {
    class wav_file_parse_exception : public std::exception {
    private:
        std::string message;

    public:
        wav_file_parse_exception(const std::string& _message)
            : message(_message) {}

        wav_file_parse_exception(const std::string&& _message)
            : message(std::move(_message)) {}

        virtual const char* what() const throw() {
            return message.c_str();
        }
    };

    class wav_file {
    public:
        typedef float sample_t;

        wav_file(std::istream& _file);

        size_t get_total_samples() const {
            return total_samples;
        }

        size_t get_sample_rate() const {
            return sample_rate;
        }

        size_t get_channels() const {
            return channels;
        }

        size_t get_samples_read() const {
            return samples_read;
        }

        void read_samples(std::vector<sample_t>& destination, size_t sample_count);

    private:
        size_t total_samples, sample_rate, channels, bytes_per_sample, samples_read;
        std::istream& file;

        template<size_t BitDepth>
        void interpret_samples(const std::vector<std::uint8_t>& bytes, std::vector<sample_t>& destination);
    };
}
