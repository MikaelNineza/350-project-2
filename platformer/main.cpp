#include <fstream>
#include <unordered_map>

#include "CameraSystem.h"
#include "CollisionComponent.h"
#include "CollisionLateSystem.h"
#include "CollisionSystem.h"
#include "ECSEngine.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "GravitySystem.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "MovementComponent.h"
#include "MovementSystem.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "ScoreSystem.h"
#include "SpawnComponent.h"
#include "SpawnSystem.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SpriteSystem.h"

std::string gResourcePath = "../assets/";
#define COMPONENTS                                                                             \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent, \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,     \
        ECSEngine::CameraFollower, ECSEngine::InputComponent, ECSEngine::GravityComponent,     \
        ECSEngine::CameraShake, ECSEngine::ScoreComponent, ECSEngine::RespawnComponent
using Engine = ECSEngine::ECSEngine<COMPONENTS>;

struct LineInfo {
    std::string path;
    ECSEngine::Rect spriteLoc;
    ECSEngine::Rect bounds;
};

void LoadMap(Engine& engine, const std::string& path, const std::string& file) {
    std::string fullPath = path + file;
    // https://www.learncpp.com/cpp-tutorial/basic-file-io/
    std::ifstream inf{ fullPath };

    if (!inf) {
        std::cerr << "Could not load in a map file\n";
        return;
    }

    std::string nextWord;
    std::unordered_map<std::string, LineInfo> readLines;
    int type;
    inf >> nextWord >> type;

    while (inf >> nextWord) {
        if (nextWord == "map") {
            break;
        }

        LineInfo line;
        std::string key = nextWord;
        std::string texturePath;
        inf >> texturePath >> line.spriteLoc.topLeft.x >> line.spriteLoc.topLeft.y >> line.spriteLoc.width >> line.spriteLoc.height;
        if (type == 2) {
            inf >> line.bounds.topLeft.x >> line.bounds.topLeft.y >> line.bounds.width >> line.bounds.height;
        }
        readLines[key] = line;
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    // Engine test code
    Engine engine(1024, 768, "Test Engine");

    // Load sounds
    auto &soundManager = engine.GetSoundManager();
    soundManager.RegisterSound("./assets/sfx_jump.ogg", "Jump Sound");
    soundManager.RegisterSound("./assets/footstep_grass_003.ogg", "Landing Sound");
    soundManager.RegisterSound("./assets/sfx_gem.ogg", "Collect Star Sound");
    soundManager.RegisterSound("./assets/footstep_snow_001.ogg", "Push Wall Sound");

    // Load maps

    // Place player

    // Setup score component

    // Setup Camera

    // Add all systems
    auto& sm = engine.GetSystemManager();
    sm.AddSystem(std::make_unique<ECSEngine::CollisionLateSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::InputSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::GravitySystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::MovementSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::ScoreSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CameraSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpriteSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpawnSystem<COMPONENTS>>());

    engine.Run();
    return 0;
}
