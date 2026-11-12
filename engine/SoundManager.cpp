#include "SoundManager.h"

namespace ECSEngine {

void SoundManager::RegisterSound(const std::string& soundPath, const std::string& soundName) {
    bool exists = mBuffers.find(soundName) != mBuffers.end();
    if (!exists) {
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(soundPath)) {
            std::cerr << "Warning: Could not load sound" << std::endl;
        }
        else {
            sf::Sound sound(buffer);
            mBuffers[soundName] = buffer;
            mSounds[soundName] = sound; 
        }
    }
}

sf::Sound& SoundManager::GetSound(const std::string& soundName) {
    if (mBuffers.find(soundName) == mBuffers.end()) {
        throw std::invalid_argument("SoundName has not been registered");
    }

    return mSounds[soundName];
}

}  // namespace ECSEngine
