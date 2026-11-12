#include <bitset>
#include <SFML/Window/Keyboard.hpp>

struct InputComponent {
    InputComponent(EntityID whichEntity) : entity(whichEntity), lastDash(0) {
    }
    ECSEngine::EntityID entity;
    int lastDash;
    std::bitset<sf::Keyboard::ScancodeCount> keydown;
    std::bitset<sf::Keyboard::ScancodeCount> lastKeydown;
};