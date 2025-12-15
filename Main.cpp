#include "EngineCore.h"
#include "Entity.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "MovementComponent.h"
#include "PhysicsComponent.h"


int main() {

    EngineCore engine;

    // SAFE: Access scene through EngineCore function
    Entity* player = engine.CreateEntity();

    TransformComponent* transform = player->addComponent<TransformComponent>(100.f, 100.f);

    player->addComponent<SpriteComponent>("Assets/player.png", transform);

    player->addComponent<MovementComponent>(transform, &engine.tilemap);
    player->addComponent<PhysicsComponent>(transform, &engine.tilemap);


    engine.run();

    return 0;
}
