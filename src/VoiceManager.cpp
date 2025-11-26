#include "VoiceManager.hpp"
#include <iostream>
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>

bool VoiceManager::loadVoice(const std::string &key, const std::string &filepath) {
    sf::SoundBuffer buf;
    if (!buf.loadFromFile(filepath)) {
        std::cerr << "VoiceManager: failed to load '" << filepath << "' for key '" << key << "'\n";
        return false;
    }
    // store buffer and a sound that uses it
    buffers[key] = buf;
    // Create sound and set its buffer (must reference stored buffer)
    sf::Sound s;
    s.setBuffer(buffers[key]);
    sounds[key] = s;
    return true;
}

void VoiceManager::play(const std::string &key) {
    auto it = sounds.find(key);
    if (it == sounds.end()) {
        // debug only
        // std::cerr << "VoiceManager: no sound for key '" << key << "'\n";
        return;
    }
    // stop and replay to ensure it plays even if already playing
    it->second.stop();
    it->second.play();
}

void VoiceManager::stopAll() {
    for (auto &p : sounds) p.second.stop();
}
