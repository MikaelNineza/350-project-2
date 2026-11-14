#pragma once

#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include <SFML/Audio.hpp>

namespace ECSEngine {

/// Owns all sound effects, looked up by a name you choose.
///
/// In an ECSEngine, the engine owns the single SoundManager. Get it from
/// EngineView::GetSoundManager(). It lives exactly as long as the engine.
///
/// Usage:
///   // setup (main.cpp):
///   soundManager.RegisterSound(gResourcePath + "sfx_jump.ogg", "jump");
///   // in a system:
///   engine->GetSoundManager().GetSound("jump").play();
///
/// Ownership / lifetime terms:
///  - Each name maps to exactly one decoded sf::SoundBuffer (the audio data)
///    and one sf::Sound (a player for that buffer). The manager owns both
///    until it is destroyed. Sounds cannot be unregistered.
///  - Both are stored in unordered_maps, so their addresses stay the same as
///    more sounds are registered. A reference from GetSound() therefore stays
///    valid for the whole lifetime of the engine.
///  - Each sf::Sound holds a non-owning pointer to its buffer. Sounds are
///    destroyed before buffers (member order below). Do not reorder the
///    members.
class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager() = default;

    /// Loads `soundPath` and makes it available under `soundName`.
    ///
    /// @param soundPath  Audio file readable by SFML (e.g. .ogg, .wav). It is
    ///                   decoded fully into memory now, not streamed.
    /// @param soundName  Key used by GetSound(). Copied.
    ///
    /// If `soundName` is already registered, the call does nothing, even if
    /// `soundPath` is different. The first registration wins.
    ///
    /// Errors: if the file cannot be loaded, a warning goes to std::cerr and
    /// the name is NOT registered. A later GetSound(soundName) will throw.
    /// This function itself does not throw for a missing file.
    ///
    /// Invalidates: nothing. Existing GetSound() references stay valid.
    void RegisterSound(const std::string& soundPath, const std::string& soundName);

    /// Returns the sound player registered under `soundName`. The manager
    /// still owns it.
    ///
    /// The reference stays valid until the SoundManager (i.e. the engine) is
    /// destroyed, so it may be cached. Do not use it after the engine is gone.
    ///
    /// The sf::Sound is SHARED by everyone who asks for that name, and only
    /// one copy of the sound can play at a time. Calling play() while it is
    /// already playing restarts it from the beginning. Changing volume, pitch,
    /// etc. affects every later use of that name.
    ///
    /// @throws std::invalid_argument if `soundName` was never successfully
    ///         registered.
    sf::Sound& GetSound(const std::string& soundName);

private:
    std::string name = "SoundManager";
    // Declaration order matters: mSounds is destroyed first, so no sound ever
    // outlives the buffer it plays.
    std::unordered_map<std::string, sf::SoundBuffer> mBuffers;  // keyed by soundName
    std::unordered_map<std::string, sf::Sound> mSounds;         // keyed by soundName
};

}  // namespace ECSEngine
