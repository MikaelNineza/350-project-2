#pragma once

#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include <SFML/Audio.hpp>

namespace ECSEngine {

class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager() = default;
    void RegisterSound(const std::string& soundPath, const std::string& soundName);
    sf::Sound& GetSound(const std::string& soundName);

private:
    std::string name = "SoundManager";
    std::unordered_map<std::string, sf::SoundBuffer> mBuffers;
    std::unordered_map<std::string, sf::Sound> mSounds;
};

}  // namespace ECSEngine
