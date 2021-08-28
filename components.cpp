#include <iostream>

#include "vanilla.h"
#include "SDLHelper.h"
#include "render.h"

#include "components.h"

Component::Component(Entity& entity) : ComponentContainer<Component>(nullptr),  entity(&entity)
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
    //std::cout << this << std::endl;
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

void RectComponent::setCenter(const glm::vec2& center)
{
    this->rect.x = center.x - rect.z/2;
    this->rect.y = center.y - rect.a/2;
}

glm::vec2 RectComponent::getPos()
{
    return {rect.x,rect.y};
}

glm::vec2 RectComponent::getCenter()
{
    return {rect.x + rect.z/2, rect.y + rect.a/2};
}


float RectComponent::getTilt()
{
    return tilt;
}

void RectComponent::setTilt(float newTilt) //clamp to -M_PI - +M_PI, just like C++ trig functions
{
    tilt = fmod(newTilt,2*M_PI) + 2*M_PI*(newTilt < 0);
    if (tilt >M_PI)
    {
        tilt -= 2*M_PI;
    }
}

RectComponent::~RectComponent()
{

}

MoveComponent::MoveComponent(float speed, const glm::vec4& rect, Entity& entity) : RectComponent(rect, entity), ComponentContainer<MoveComponent>(&entity),
                                                                                    baseSpeed(speed),speed(speed)
{
    target = {rect.x + rect.z/2, rect.y + rect.a/2};
}

bool MoveComponent::collides(const glm::vec4& target)
{
    return vecIntersect(target,rect,tilt,0);
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
    if (!ignoreTarget)
    {
        angle = atan2((target.y - (center.y)),(target.x - (center.x)));
    }
    glm::vec2 increment = {cos(angle)*speed*DeltaTime::deltaTime,sin(angle)*speed*DeltaTime::deltaTime};
    if (ignoreTarget)
    {
        return getCenter() + increment;
    }
    return getCenter() + glm::vec2(absMin(increment.x,target.x - center.x),absMin(increment.y, target.y - center.y));
}

void MoveComponent::update()
{
    glm::vec2 center = getCenter();
    if (!atTarget() || ignoreTarget)
    {
        glm::vec2 nextPoint = getNextPoint();
        rect.x += nextPoint.x - center.x;
        rect.y += nextPoint.y - center.y;
    }
    velocity = pointDistance({rect.x + rect.z/2, rect.y + rect.a/2}, center); //distance between new center vs old center
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

void MoveComponent::setAngle(float val)
{
    angle = val;
}


float MoveComponent::getAngle()
{
    return angle;
}

float MoveComponent::getVelocity()
{
    return velocity;
}

float MoveComponent::getBaseSpeed()
{
    return baseSpeed;
}

float MoveComponent::getCurSpeed()
{
    return speed;
}

void MoveComponent::setSpeed(float newspeed)
{
    speed = newspeed;
}

void MoveComponent::setIgnoreTarget(bool val)
{
    ignoreTarget = val;
}

MoveComponent::~MoveComponent()
{

}

void RealMoveComponent::accelerate()
{
    speed = std::max(0.0f,std::min(speed + accel*DeltaTime::deltaTime,baseSpeed)); //accelerate
}

void RealMoveComponent::decelerate()
{
    speed = std::max(0.0f,std::min(speed - decel*DeltaTime::deltaTime,baseSpeed));
}

RealMoveComponent::RealMoveComponent(float accel_, float deccel_, float speed, const glm::vec4& rect, Entity& entity) : MoveComponent(speed, rect, entity),
                                                                                                                            ComponentContainer<RealMoveComponent>(entity),
                                                                                                                            accel(accel_),
                                                                                                                            decel(deccel_)
{
    speed = 0;
}

void RealMoveComponent::setTarget(const glm::vec2& point)
{
    if (point != target)
    {
        glm::vec2 center= getCenter();
        speed =0;
          maxSpeed = std::min(baseSpeed,(float)sqrt(2*pointDistance(point,center)*accel*decel/(accel + decel)));
          tilt = atan2(point.y - center.y ,point.x - center.x);
    }
    MoveComponent::setTarget(point);
}

float RealMoveComponent::getAccel()
{
    return accel;
}

float RealMoveComponent::getDecel()
{
    return decel;
}

void RealMoveComponent::update()
{
    if (!atTarget())
    {
       // float dist = pointDistance(getCenter(),target);
        float decelDist = maxSpeed/decel*maxSpeed/2.0; //Distance traveled while decelerating; integral of speed vs time with a constant deceleration rate.
        if (pointDistance(getCenter(),target) > decelDist)
        {
            accelerate();
        }
        else
        {
            decelerate();
            speed = std::max(MoveComponent::distThreshold,speed);
        }
    }
  //  std::cout << speed << "\n";
    float oldSpeed = speed;
    MoveComponent::update(); //we want to use every part of MoveComponent::update except the part where it overrides our speed
    speed = oldSpeed;
}

void ForcesComponent::applyForce(ForceVector force)
{
   float y = sin(force.angle)*force.magnitude;
    float x = cos(force.angle)*force.magnitude;

    finalForce.x += x;
    finalForce.y += y;
}

void ForcesComponent::applyAllForces()
{
        auto end = forces.end();
       for (auto start = forces.begin(); start != end;)
       {
           applyForce(*start);
           start->frames -= 1;
           if (start->frames == 0)
           {
               start = forces.erase(start);
           }
           else
           {
               start->magnitude *= 1 + start->increment/100.0;
               ++start;
           }
       }
}

ForcesComponent::ForcesComponent(Entity& entity) : Component(entity), ComponentContainer<ForcesComponent>(&entity), move(entity.getComponent<MoveComponent>())
{
    if (!move)
    {
        throw std::logic_error("Entity has ForcesComponent but no MoveComponent!");
    }
}

bool ForcesComponent::getBeingPushed()
{
    return forces.size() > 0 || finalForce.x != 0 || finalForce.y != 0;
}

void ForcesComponent::addForce(ForceVector force)
{
    if (force.frames > 1)
    {
        force.frames -= 1;
        forces.push_back(force);
    }
    else
    {
        applyForce(force); //we can save some time by automatically applying one-time forces
    }
}

void ForcesComponent::update()
{

    if ((finalForce.x != 0 || finalForce.y != 0 || forces.size() > 0) && move)
    {
        applyAllForces();
        move->setPos(move->getPos() + finalForce);
        finalForce = glm::vec2(0);
       // forces.clear();
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

SpriteParameter SpriteComponent::defaultRender()
{
    SpriteParameter param;
    RectComponent* rect = entity->getComponent<RectComponent>();
    if (rect)
    {
        if (camera)
        {
            param.rect = camera->toScreen(rect->getRect());
        }
        else
        {
            param.rect = rect->getRect();
        }
        param.radians = rect->getTilt();
    }
    return param;
}

SpriteComponent::SpriteComponent(SpriteWrapper& sprite_, bool animated_, Entity& entity, RenderCamera* camera) : RenderComponent(entity, camera),
                                                                                                                ComponentContainer<SpriteComponent>(entity),
                                                                                                                        sprite(&sprite_),
                                                                                                                        animated(animated_)
{

}

void SpriteComponent::render(const SpriteParameter& param)
{
    if (sprite)
    {
        sprite->request(param);
    }
}

void SpriteComponent::setParam(const SpriteParameter& param, const AnimationParameter& animeParam)
{
    this->sParam = param;
    this->aParam = animeParam;
    modified = true;
}

void SpriteComponent::update()
{
    if (sprite)
    {
        if (!modified) //if sParam.rect was not modified at all, attempt to render at the entity's position, if possible
        {
            sParam = defaultRender();
        }
        if (animated)
        {
            static_cast<AnimationWrapper*>(sprite)->request(sParam,aParam);
        }
        else
        {
            sprite->request(sParam);
        }
        setParam(SpriteParameter(),AnimationParameter()); //reset params
        modified = false;
    }
}

Entity::Entity()
{

}


void Entity::addComponent(Component& comp)
{
    if (components.find(&comp) == components.end())
    {
        components[&comp] = std::shared_ptr<Component>(&comp);

    }
}

void Entity::update()
{
    auto end = components.end();
    for (auto i = components.begin(); i != end; ++i)
    {
        i->first->update();
    }
}

void Entity::collide(Entity& entity)
{
    auto end = components.end();
    for (auto i = components.begin();i != end; ++i)
    {
        i->first->collide(entity);
    }
}

void Entity::onDeath()
{
    auto end = components.end();
    for (auto i = components.begin();i != end; ++i)
    {
        i->first->onDeath();
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

void EntityManager::addEntity(Entity& entity)
{
    std::shared_ptr<Entity> ptr(&entity);
    addEntity(ptr);
}

void EntityManager::addEntity(std::shared_ptr<Entity>& entity)
{
    if (entities.find(entity.get()) == entities.end() && entity.get())
    {
        entities[entity.get()] = entity;
    }
}

void EntityManager::update()
{
    auto end = entities.end();
    for (auto it = entities.begin(); it != end; ++it)
    {
        it->first->update();
    }
}
