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

template <typename... Components>
class EngineView {
public:
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;
};

template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    ~ECSEngine();

    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

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
        mWindowManager.GetWindow()->display();
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
