#pragma once

#include <vector>
#include <optional>
#include <string>

#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

/// The engine as seen by a System: access to the four resource managers.
///
/// Every getter returns a reference to a manager that the engine owns. The
/// reference is valid for the engine's whole lifetime, and each call returns
/// the same object. Do not delete or copy the managers. What you get FROM a
/// manager (component references, sprites, sounds, the window handle) has its
/// own, often shorter, validity, which is documented in that manager's header:
///
///   EntityManager::GetComponent  -> until AddComponent<T>/Remove* (EntityManager.h)
///   SpriteManager::GetSprite     -> until next RegisterTexture  (SpriteManager.h)
///   SoundManager::GetSound       -> engine lifetime             (SoundManager.h)
///   WindowManager::GetWindow     -> shared_ptr; closed when the engine dies (WindowManager.h)
template <typename... Components>
class EngineView {
public:
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;
};

/// Top-level object. It owns the window, every manager, and the game loop.
///
/// `Components...` is the full list of component types the game may use. It
/// is passed through to EntityManager, System and SystemManager.
///
/// Typical setup (see platformer/main.cpp):
///   ECSEngine<COMPONENTS> engine(w, h, "title");
///   // 1. register textures/sounds, 2. create entities, 3. AddSystem(...)
///   engine.Run();
///
/// Lifetime: the managers are plain members, so they are created with the
/// engine and destroyed with it. On destruction the window is closed first,
/// and then the members are destroyed in reverse declaration order: window,
/// sounds, sprites, systems, entities. Nothing returned by any manager may
/// be used after the engine is destroyed. The engine is not copyable
/// (SystemManager owns its systems through unique_ptr), and it should live
/// on the stack in main() for the whole program.
template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    /// Opens a `width` x `height` window titled `name`, limited to 60 FPS.
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    /// Closes the window. Then all managers and systems are destroyed.
    ~ECSEngine();

    /// Game loop. Each frame it handles window events (close, and key state
    /// into every InputComponent), then runs all systems in order. It returns
    /// when the window is closed or a system returns kExit.
    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

    /// Owner-only (not on EngineView). Use it during setup to AddSystem().
    /// Valid for the engine's lifetime.
    SystemManager<Components...>& GetSystemManager() { return mSystemManager; }

private:
    bool ProcessEvents();
    void DebugDraw();

    EntityManager<Components...> mEntityManager;
    SystemManager<Components...> mSystemManager;
    SpriteManager mSpriteManager;
    SoundManager mSoundManager;
    WindowManager mWindowManager;

    // For debugging - pause and step the engine
    bool running, step;
    int debug;
};

template <typename... Components>
ECSEngine<Components...>::ECSEngine(unsigned int width, unsigned int height,
                                    const std::string& name)
    : mWindowManager(width, height, name) {
    mWindowManager.GetWindow()->setFramerateLimit(60);
}

template <typename... Components>
ECSEngine<Components...>::~ECSEngine() {
    mWindowManager.GetWindow()->close();
}

template <typename... Components>
void ECSEngine<Components...>::Run() {
    while (ProcessEvents()) {
        if (!mSystemManager.Run(this)) {
            break;
        }
    }
}

template <typename... Components>
bool ECSEngine<Components...>::ProcessEvents() {
    auto window = mWindowManager.GetWindow();

    while (const std::optional event = window->pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window->close();
            return false;
        }

        const auto* keyDown = event ->getIf<sf::Event::KeyPressed>();
        const auto* keyUp = event ->getIf<sf::Event::KeyReleased>();

        if (!keyDown && !keyUp) {
            continue;
        }

        auto code = keyDown ? keyDown->scancode : keyUp->scancode;

        for (auto &entity : mEntityManager) {
            EntityID id = entity.GetID();
            if (!mEntityManager.ValidEntity(id)) {
					continue;
			}
            if (mEntityManager.template HasComponent<InputComponent>(id)) {
                auto &input = mEntityManager.template GetComponent<InputComponent>(id);
                input.keydown[static_cast<std::size_t>(code)] = keyDown != nullptr;
            }
        }
    }

    return window->isOpen();
}

template <typename... Components>
void ECSEngine<Components...>::DebugDraw() {}

}  // namespace ECSEngine
