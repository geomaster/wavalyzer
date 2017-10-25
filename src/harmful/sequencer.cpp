#include "sequencer.hpp"
#include "synth.hpp"
#include "common.hpp"
#include <unordered_map>
#include <sstream>

using namespace std;
using namespace harmful;

namespace harmful {
    struct note_t {
        float frequency;
        float start_beats, duration_beats;
        float velocity;
    };

    struct pattern_t {
        int duration_beats;
        vector<pair<string, note_t>> notes;
    };

    struct sequencer_state_t {
        unordered_map<string, synth_t> synths;
        unordered_map<string, pattern_t> patterns;
    };

    harmonic_t parse_harmonic(node_t* harmonic);
    void parse_synth(sequencer_state_t& state, node_t* synth);
    void parse_pattern(sequencer_state_t& state, node_t* pattern);
    float string_to_float(const string& s);
    float parse_duration(const string& s);
    void check_param_size(node_t* child, size_t n);
}

void harmful::check_param_size(node_t* child, size_t n)
{
    if (child->params.size() != n) {
        throw sequencer_exception("Block `" + child->key + "` should have " + to_string(n) + " parameters, got " + to_string(child->params.size()));
    }
}

float harmful::string_to_float(const string& s)
{
    istringstream str(s);
    float x;

    str >> x;
    if (!cin.good()) {
        throw sequencer_exception("Not a valid number: `" + s + "`");
    }

    return x;
}

float harmful::parse_duration(const string& s)
{
    if (ends_with(s, "ms")) {
        return string_to_float(s.substr(0, s.length() - 2)) / 1000.0f;
    } else if (ends_with(s, "s")) {
        return string_to_float(s.substr(0, s.length() - 1));
    } else {
        throw sequencer_exception("Invalid duration: `" + s + "`");
    }
}


harmonic_t harmful::parse_harmonic(node_t* harmonic)
{
    check_param_size(harmonic, 2);

    string mul = harmonic->params[1];
    if (mul.size() < 2 || mul.back() != 'f') {
        throw sequencer_exception("Invalid harmonic `" + mul + "`");
    }

    harmonic_t h;
    mul.pop_back();
    h.freq_multiplier = string_to_float(mul);
    h.amplitude = 1.0f;

    string type = harmonic->params[0];
    if (type == "saw") {
        h.type = WAVE_SAW;
    } else if (type == "sine") {
        h.type = WAVE_SINE;
    } else {
        throw sequencer_exception("Invalid wave `" + type + "`");
    }

    for (node_t* child : harmonic->children) {
        if (child->key == "level") {
            check_param_size(child, 1);
            h.amplitude = string_to_float(child->params[0]);
        } else if (child->key == "attack") {
            check_param_size(child, 1);
            h.volume_envelope.attack_ms = 1000.0f * parse_duration(child->params[0]);
        } else if (child->key == "release") {
            check_param_size(child, 1);
            h.volume_envelope.release_ms = 1000.0f * parse_duration(child->params[0]);
        } else if (child->key == "decay") {
            check_param_size(child, 1);
            h.volume_envelope.decay_ms = 1000.0f * parse_duration(child->params[0]);
        } else if (child->key == "sustain") {
            check_param_size(child, 1);
            h.volume_envelope.sustain_level = string_to_float(child->params[0]);
        } else {
            throw sequencer_exception("Unknown block `" + child->key + "` in harmonic");
        }
    }

    return h;
}

void harmful::parse_synth(sequencer_state_t& state, node_t* synth)
{
    check_param_size(synth, 1);

    string name = synth->params[0];
    if (state.synths.find(name) != state.synths.end()) {
        throw sequencer_exception("Duplicate synth name: `" + name + "`");
    }

    synth_t s;
    s.volume = 1.0f;
    for (node_t* child : synth->children) {
        if (child->key == "volume") {
            check_param_size(child, 1);
            s.volume = string_to_float(child->params[0]);
        } else if (child->key == "harmonic") {
            s.harmonics.push_back(parse_harmonic(child));
        } else {
            throw sequencer_exception("Unknown block `" + child->key + "` in synth");
        }
    }

    state.synths[name] = s;
}

void harmful::parse_pattern(sequencer_state_t& state, node_t* pattern)
{
    check_param_size(pattern, 2);

    string name = pattern->params[0];
    if (state.patterns.find(name) != state.patterns.end()) {
        throw sequencer_exception("Duplicate pattern name: `" + name + "`");
    }

    pattern_t p;
    p.duration_beats = string_to_float(pattern->params[1]);

    for (node_t* child : pattern->children) {
        check_param_size(child, 0);

        for (node_t* note : child->children) {
            check_param_size(note, 2);

            note_t n;
            n.frequency = note_to_frequency(note->key);
            n.start_beats = string_to_float(note->params[0]) - 1.0f;
            n.duration_beats = string_to_float(note->params[1]);
            n.velocity = 1.0f;

            p.notes.push_back(make_pair(child->key, n));
        }
    }

    state.patterns[name] = p;
}

vector<float> harmful::sequence(node_t* root, int sample_rate)
{
    sequencer_state_t state;
    node_t* song_node = nullptr;

    for (node_t* child : root->children)
    {
        if (child->key == "synth") {
            parse_synth(state, child);
        } else if (child->key == "pattern") {
            parse_pattern(state, child);
        } else if (child->key == "song") {
            if (song_node != nullptr) {
                throw sequencer_exception("Only one `song` block allowed.");
            }

            song_node = child;
        }
    }

    if (song_node == nullptr) {
        throw sequencer_exception("No `song` blocks have been found.");
    }

    check_param_size(song_node, 2);
    string bpm = song_node->params[0];
    if (!ends_with(bpm, "bpm")) {
        throw sequencer_exception("Invalid bpm given: `" + bpm + "`");
    }

    vector<float> samples;
    float tempo = string_to_float(bpm.substr(0, bpm.length() - 3));
    float duration = parse_duration(song_node->params[1]);

    samples.resize(duration * sample_rate);
    float ms_per_beat = 60.0f * 1000.0f / tempo;

    for (node_t* n : song_node->children) {
        check_param_size(n, 2);
        auto it = state.patterns.find(n->key);
        if (it == state.patterns.end()) {
            throw sequencer_exception("Unknown pattern `" + n->key + "`");
        }
        const pattern_t& pat = it->second;

        float start_beats = string_to_float(n->params[0]) - 1.0f;
        if (!ends_with(n->params[1], "x")) {
            throw sequencer_exception("Invalid repeat count: `" + n->params[0] + "`");
        }

        int repeat = string_to_integer(n->params[1].substr(0, n->params[1].size() - 1));
        for (int i = 0; i < repeat; i++) {
            float start = start_beats + i * pat.duration_beats;

            // Sequence the pattern
            for (const auto& synth_note : pat.notes) {
                auto it = state.synths.find(synth_note.first);
                if (it == state.synths.end()) {
                    throw sequencer_exception("Unknown synth `" + synth_note.first + "`");
                }

                const synth_t& synth = it->second;
                const note_t& note = synth_note.second;
                float start_ms = (start + note.start_beats) * ms_per_beat,
                      duration_ms = note.duration_beats * ms_per_beat;

                add_note(samples, note.frequency, start_ms, duration_ms, note.velocity, synth, sample_rate);
            }
        }
    }

    return samples;
}
