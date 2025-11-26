#ifndef VOICEMANAGER_HPP
#define VOICEMANAGER_HPP
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>

class VoiceManager
{
public:
    VoiceManager() = default;
    ~VoiceManager() = default;

    // load a voice file and associate it with a key name
    // returns true on success
    bool loadVoice(const std::string &key, const std::string &filepath);

    // play a loaded voice (non-blocking)
    void play(const std::string &key);

    // stop all
    void stopAll();

private:
    std::unordered_map<std::string, sf::SoundBuffer> buffers;
    std::unordered_map<std::string, sf::Sound> sounds;
};
#endif // VOICEMANAGER_HPP
