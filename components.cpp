#include <iostream>

#include "vanilla.h"
#include "SDLHelper.h"
#include "render.h"

#include "components.h"



Component::Component(Entity& entity) : entity(&entity), ComponentContainer<Component>(nullptr)
{

}

void Component::update()
{

}
void Component::collide(Entity& other)
{

}
void Component::onDeath()
{

}

Entity& Component::getEntity()
{
    return *entity;
}

Component::~Component()
{
   // std::cout << "Deleted!" << std::endl;
}

RectComponent::RectComponent(const glm::vec4& rect, Entity& entity) : Component(entity), ComponentContainer<RectComponent>(&entity), RectPositional(rect)
{

}

void RectComponent::setRect(const glm::vec4& rect)
{
    this->rect = rect;
}

void RectComponent::setPos(const glm::vec2& pos)
{
    this->rect.x = pos.x;
    this->rect.y = pos.y;
}

glm::vec2 RectComponent::getPos()
{
    return {rect.x,rect.y};
}

glm::vec2 RectComponent::getCenter()
{
    return {rect.x + rect.z/2, rect.y + rect.a/2};
}

RectComponent::~RectComponent()
{

}

MoveComponent::MoveComponent(double speed, const glm::vec4& rect, Entity& entity) : RectComponent(rect, entity), ComponentContainer<MoveComponent>(&entity),
                                                                                    baseSpeed(speed),speed(speed)
{
    target = {rect.x + rect.z/2, rect.y + rect.a/2};
}


void MoveComponent::teleport(const glm::vec2& point)
{
    rect.x = point.x - rect.z/2; //rect.x += point.x - (rect.x + rect.z/2) -> rect.x = point.x - rect.z/2
    rect.y = point.y - rect.a/2;
    setTarget(point);
}

glm::vec2 MoveComponent::getNextPoint()
{
    glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
    angle = atan2((target.y - (center.y)),(target.x - (center.x)));
    return getCenter() + glm::vec2(absMin(cos(angle)*speed*DeltaTime::deltaTime,target.x - center.x),absMin(sin(angle)*speed*DeltaTime::deltaTime, target.y - center.y));
}

void MoveComponent::update()
{
    glm::vec2 center = getCenter();
    if (!atTarget())
    {
        glm::vec2 nextPoint = getNextPoint();
        rect.x += nextPoint.x - center.x;
        rect.y += nextPoint.y - center.y;
    }
    velocity = pointDistance({rect.x + rect.z/2, rect.y + rect.a/2}, center);
    speed = baseSpeed;
}

bool MoveComponent::atPoint(const glm::vec2& point)
{
    return pointDistance(getCenter(),point) <= distThreshold;
}

bool MoveComponent::atTarget()
{
    return atPoint(target);
}


const glm::vec2& MoveComponent::getTarget()
{
    return target;
}

void MoveComponent::setTarget(const glm::vec2& point)
{
    target = point;
}


double MoveComponent::getVelocity()
{
    return velocity;
}

double MoveComponent::getBaseSpeed()
{
    return baseSpeed;
}

double MoveComponent::getCurSpeed()
{
    return speed;
}

void MoveComponent::setSpeed(double newspeed)
{
    speed = newspeed;
}

MoveComponent::~MoveComponent()
{

}

ForcesComponent::ForcesComponent(Entity& entity) : Component(entity), ComponentContainer<ForcesComponent>(&entity), move(entity.getComponent<MoveComponent>())
{
    if (!move)
    {
        throw std::logic_error("Entity has ForcesComponent but no MoveComponent!");
    }
}

void ForcesComponent::addForce(ForceVector force)
{
    float y = sin(force.angle)*force.magnitude;
    float x = cos(force.angle)*force.magnitude;

    finalForce.x += x;
    finalForce.y += y;
    forces.push_back(force);
}

void ForcesComponent::update()
{
    if ((finalForce.x != 0 || finalForce.y != 0) && move)
    {
        if (abs(finalForce.x) > 10 || abs(finalForce.y) > 10)
        {
            for (auto it = forces.begin(); it != forces.end(); ++it)
            {
                std::cout << it->angle << " " << it->magnitude << "\n";
            }
        }
      //  glm::vec2 finalPoint = glm::vec2(move->getPos() + finalForce);
        move->setPos(move->getPos() + finalForce);
        finalForce = glm::vec2(0);
        forces.clear();
    }

}

RenderComponent::RenderComponent(Entity& entity, RenderCamera* camera) : Component(entity), ComponentContainer<RenderComponent>(&entity), camera(camera)
{

}

void RenderComponent::render(const SpriteParameter& param) //every rendercomponent can take in a SpriteParameter and render it accordingly
{

}

RenderComponent::~RenderComponent()
{

}

Entity::Entity()
{

}


void Entity::addComponent(Component& comp)
{
    components.emplace_back(&comp);
}

void Entity::update()
{
    for (int i = components.size() - 1; i >= 0; --i)
    {
        components[i]->update();
    }
}

void Entity::collide(Entity& entity)
{
    for (int i= components.size() - 1; i >= 0; --i)
    {
        components[i]->collide(entity);
    }
}

void Entity::onDeath()
{
    for (int i = components.size() - 1; i >= 0; --i)
    {
        components[i]->onDeath();
    }
}

Entity::~Entity()
{
    components.clear();
}

Entity* EntityAssembler::assemble()
{
    return nullptr;
}
