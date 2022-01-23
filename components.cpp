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


bool RectComponent::collides(const glm::vec4& target)
{
    return vecIntersect(target,rect,0,tilt);
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

RectComponent::~RectComponent()
{

}

MoveComponent::MoveComponent(float speed, const glm::vec4& rect, Entity& entity) : RectComponent(rect, entity), ComponentContainer<MoveComponent>(&entity),
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
    if (!ignoreTarget)
    {
        angle = atan2((target.y - (center.y)),(target.x - (center.x)));
    }
    glm::vec2 increment = {cos(angle)*speed*DeltaTime::deltaTime,sin(angle)*speed*DeltaTime::deltaTime};

    if (ignoreTarget)
    {
        return glm::vec2(rect.x, rect.y) + increment;
    }
    return glm::vec2(rect.x + absMin(increment.x,target.x - center.x),rect.y + absMin(increment.y, target.y - center.y));
}

void MoveComponent::update()
{
    glm::vec2 center = getCenter();
    if (!atTarget() || ignoreTarget)
    {
        glm::vec2 nextPoint = getNextPoint();

        rect.x = nextPoint.x;
        rect.y = nextPoint.y;
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

void MoveComponent::setTarget(const glm::vec2& point)
{
    target = point;
}

const glm::vec2& MoveComponent::getTarget()
{
    return target;
}

void MoveComponent::setTiltTowardsTarget()
{
     glm::vec2 center = getCenter();
    tilt = atan2(target.y - center.y ,target.x - center.x);
}

void MoveComponent::setPos(const glm::vec2& pos)
{
    rect.x =pos.x;
    rect.y = pos.y;
    target.x = pos.x - rect.z/2;
    target.y = pos.y - rect.a/2;
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
                                                                                                                            accel(accel_ == 0 ? speed : accel_),
                                                                                                                            decel(deccel_ == 0 ? speed : deccel_)
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
        setTiltTowardsTarget(); //convenient to constnatly set this angle in case it doesn't get set incorrectly
    }

  //  std::cout << speed << "\n";
    float oldSpeed = speed;
    MoveComponent::update(); //we want to use every part of MoveComponent::update except the part where it overrides our speed
    speed = oldSpeed;
}

RenderComponent::RenderComponent(Entity& entity, RenderCamera* camera) : Component(entity), ComponentContainer<RenderComponent>(&entity), camera(camera)
{

}

void RenderComponent::render(const SpriteParameter& param) //every rendercomponent can take in a SpriteParameter and render it accordingly
{

}

RenderCamera* RenderComponent::getCamera()
{
    return camera;
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

void SpriteComponent::setParam(const SpriteParameter& param, const AnimationParameter& animeParam, bool modified_)
{
    this->sParam = param;
    this->aParam = animeParam;
    modified = modified_;
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

BaseHealthComponent::BaseHealthComponent(float invulnTime_, float health_,float maxHealth_, Entity& entity) :
                                                                Component(entity), ComponentContainer<BaseHealthComponent>(entity),
                                                                invulnTime(invulnTime_), health(health_), maxHealth(maxHealth_) //health
{

}

void BaseHealthComponent::addHealth(float damage)
{
    if (maxHealth == -1)
    {
        health = std::max(0.0f, health + damage);
    }
    else
    {
        health = std::max(0.0f, std::min(maxHealth, health + damage));
    }
    if (damage < 0)
    {
        invuln.set();
    }
}

float BaseHealthComponent::getHealth()
{
    return health;
}

bool BaseHealthComponent::getInvuln()
{
    return invuln.timePassed(invulnTime);
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

IDComponent::IDComponent(Entity& entity, const std::shared_ptr<EntityAssembler>& assembler_, std::string name_, int id_) :
                                                                                                Component(entity),
                                                                                                ComponentContainer<IDComponent>(entity),
                                                                                                assembler(assembler_),
                                                                                                name(name_),
                                                                                                id(id_)
{

}

IDComponent::IDComponent(Entity& entity, std::string name_, int id_) : Component(entity),
                                                                        ComponentContainer<IDComponent>(entity),
                                                                        name(name_),
                                                                        id(id_)
{

}

bool EntityManager::forEachEntity(Entity& entity)
{
    entity.update();
    return false;
}

void EntityManager::addEntity(Entity& entity)
{
    std::shared_ptr<Entity> ptr(&entity);
    addEntity(ptr);
}

void EntityManager::addEntity(const std::shared_ptr<Entity>& entity)
{
    if (entities.find(entity.get()) == entities.end() && entity.get())
    {
        entities[entity.get()] = entity;
    }
}

EntityManager::EntityIt EntityManager::removeEntity(Entity* entity)
{
    EntityIt it = entities.find(entity);
    if (it != entities.end())
    {
        return entities.erase(it);
    }
}

std::shared_ptr<Entity> EntityManager::getEntity(Entity* ptr)
{
    auto found = entities.find(ptr);
    if (found != entities.end())
    {
        return found->second;
    }
    else
    {
        return std::shared_ptr<Entity>();
    }
}

void EntityManager::update()
{
    auto end = entities.end();
    for (auto it = entities.begin(); it != end;)
    {
       if (forEachEntity(*it->first))
       {
           it = removeEntity(it->first);
       }
       else
       {
           ++it;
       }
    }
}

bool EntityPosManager::forEachEntity(Entity& entity)
{
    if (RectComponent* rect = entity.getComponent<RectComponent>())
    {
        /*QuadTree* old = quadtree->find(*rect);
        if (old)
        {
            auto nearest = old->getNearest(*rect);
            auto end =nearest.end();
            for (auto it = nearest.begin(); it != end; ++it)
            {
                if ((*it)->collides(rect->getRect()))
                {
                    entity.collide(static_cast<RectComponent*>(*it)->getEntity());
                }
            }
        }
        entity.update();
        if (old)
        {
            quadtree->update(*rect,*old);
        }*/
    }
    else
    {
        entity.update();
    }
    return false;
}

void EntityPosManager::init(const glm::vec4& rect)
{
    bitree.reset(new BiTree(rect));
}

BiTree* EntityPosManager::getBiTree()
{
    return bitree.get();
}

void EntityPosManager::addEntity(const std::shared_ptr<Entity>& entity)
{
    if (auto rect = entity->getComponent<RectComponent>())
    {
        bitree->insert(*rect);
    }
    EntityManager::addEntity(entity);
}

void EntityPosManager::addEntity(Entity& entity, float x, float y, bool centered)
{
    if (RectComponent* rect = entity.getComponent<RectComponent>())
    {
        rect->setPos({x - centered*rect->getRect().z/2, y - centered*rect->getRect().a/2}); //if centered, we have to adjust the position so that our center is over x and y
        addEntity(std::shared_ptr<Entity>(&entity));
    }
}

EntityPosManager::EntityIt EntityPosManager::removeEntity(Entity* entity)
{
    if (entities.find(entity) != entities.end() && entity)
    {
        if (RectComponent* rect = entity->getComponent<RectComponent>())
        {
            bitree->remove(*rect);
        }
        return EntityManager::removeEntity(entity);
    }
}

void EntityPosManager::reset()
{
    entities.clear();
   // bitree->clear();
}

