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
    ECSEngine::Rect spriteLoc;
    ECSEngine::Rect bounds;
    ECSEngine::SpriteID spriteID;
};

void LoadMap(Engine& engine, const std::string& path, const std::string& file) {
    auto &em = engine.GetEntityManager();
    auto &sm = engine.GetSpriteManager();
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
    // makes the dictonary
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

        line.spriteID = sm.RegisterTexture(path + texturePath, line.spriteLoc);
        readLines[key] = line;
    }

    ECSEngine::Point2D topLeft;
    ECSEngine::Point2D tile;
    int gridWith, gridHeight;
    // builds the map
    inf >> nextWord >> topLeft.x >> topLeft.y >> nextWord >> tile.x >> tile.y >> nextWord >> gridWith>> gridHeight;
    for (int y = 0; y < gridHeight; ++y) {
        std::string row;
        inf >> row;
        for (int x = 0; x < gridWith; ++x) {
            char chr = row[x];
            if (readLines.find(std::string{chr}) == readLines.end()) {
                continue;
            }

            LineInfo &info = readLines[std::string{chr}];
            ECSEngine::EntityID entity = em.CreateEntity(std::string{chr});
            ECSEngine::Point2D pos(x * tile.x + topLeft.x, y * tile.y + topLeft.y);

            em.AddComponent(entity, ECSEngine::LocationComponent{pos});
            if (type == 2) {
                ECSEngine::CollisionComponent collision{};
                collision.staticEntity = 1;
                if (std::string{chr} == "x") {
                    collision.doesDamage = 1;
                } else {
                    collision.doesDamage = 0;
                }
                collision.collisionBox = info.bounds;
                collision.previousCollisionBox = info.bounds;
                em.AddComponent(entity, collision);
            }

            ECSEngine::SpriteComponent spriteComp;
            spriteComp.textureID = info.spriteID;
            spriteComp.bounds = ECSEngine::Rect{ECSEngine::Point2D(0, 0), tile.x, tile.y};
            spriteComp.worldSpace = true;
            em.AddComponent(entity, spriteComp);
        }
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
    soundManager.RegisterSound(gResourcePath + "sfx_jump.ogg", "Jump Sound");
    soundManager.RegisterSound(gResourcePath + "footstep_grass_003.ogg", "Landing Sound");
    soundManager.RegisterSound(gResourcePath + "sfx_gem.ogg", "Collect Star Sound");
    soundManager.RegisterSound(gResourcePath + "footstep_snow_001.ogg", "Push Wall Sound");

    // Load maps
    LoadMap(engine, gResourcePath, "sky.map");
    LoadMap(engine, gResourcePath, "world.map");

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
