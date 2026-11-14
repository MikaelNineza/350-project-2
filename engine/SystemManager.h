#pragma once

#include <vector>
// #include "ECSEngine.h"

namespace ECSEngine {

/// Value a System returns from Run(): keep the game loop going, or stop it.
enum tSystemReturn { kContinue, kExit };

template <typename... Components>
class EngineView;

/// Base class for game logic. The engine calls each system once per frame.
///
/// Terms of use for implementers:
///  - `scene` (the EngineView) is non-owning. It points at the engine, which
///    outlives every system, but it is only meant to be used inside Run().
///    Do not delete it or store it.
///  - Get manager references (GetEntityManager() etc.) inside Run(). They
///    are valid for the engine's lifetime, but references to individual
///    components and sprites are much shorter-lived. See EntityManager.h and
///    SpriteManager.h.
///  - Systems are destroyed after the resource managers (see ECSEngine), so a
///    system's destructor must not use any manager or any resource it got
///    from one.
template <typename... Components>
class System {
public:
    virtual ~System() = default;
    /// Runs one frame of this system. Return kExit to stop the game loop.
    /// No later system runs in that frame.
    virtual tSystemReturn Run(EngineView<Components...>* scene) = 0;
    /// Name used in log messages. The returned reference must stay valid for
    /// the system's lifetime (existing systems return a function-local static).
    virtual const std::string& GetName() const = 0;
};

/// Owns the list of systems and runs them in the order they were added.
///
/// In an ECSEngine, get it from ECSEngine::GetSystemManager(). It is not on
/// EngineView, so systems cannot add or remove other systems.
template <typename... Components>
class SystemManager {
public:
    /// Runs every system once, in insertion order.
    /// @return false if a system returned kExit (remaining systems are skipped).
    bool Run(EngineView<Components...>* scene) {
        for (auto& system : mSystems) {
            if (system->Run(scene) == kExit)  // system wants to exit
            {
                std::cout << system->GetName() << " requested exit from system manager\n";
                return false;
            }
        }
        return true;
    }
    /// Takes ownership of `system` and appends it to the run order.
    /// Must not be called while Run() is in progress (it would invalidate the loop).
    void AddSystem(std::unique_ptr<System<Components...>>&& system) {
        mSystems.push_back(std::move(system));
    }
    /// Destroys all systems. Must not be called while Run() is in progress.
    void ClearSystems() { mSystems.clear(); }

private:
    std::vector<std::unique_ptr<System<Components...>>> mSystems;
};

}  // namespace ECSEngine
