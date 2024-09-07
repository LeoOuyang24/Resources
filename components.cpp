#include <iostream>

#include "vanilla.h"
#include "SDLHelper.h"
#include "render.h"
#include "glGame.h"

#include "components.h"

Component::Component(std::shared_ptr<Entity> entity_) : ComponentContainer<Component>(EntityPtr(nullptr)),  entity(entity_)
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

Entity* Component::getEntity()
{
    return entity.lock().get();
}

Component::~Component()
{
    //std::cout << this << std::endl;
}

PositionalComponent::PositionalComponent(const glm::vec2& pos_, EntityPtr entity) : Component(entity), ComponentContainer<PositionalComponent>(entity), pos(pos_)
{

}

glm::vec2 PositionalComponent::getPos() const
{
    return pos;
}

glm::vec2 PositionalComponent::getCenter() const
{
    return getPos();
}

float PositionalComponent::getTilt() const
{
    return tilt;
}

glm::vec4 PositionalComponent::getBoundingRect() const
{
    return glm::vec4(getPos(),0,0);
}

bool PositionalComponent::collidesRect(const glm::vec4& box) const
{
    return pointInVec(box,pos);
}

bool PositionalComponent::collidesLine(const glm::vec4& line) const
{
    return pointLineDistance(line,pos) == 0;
}

float PositionalComponent::distance(const glm::vec2& point) const
{
    return glm::length(pos - point);
}

void PositionalComponent::setPos(const glm::vec2& pos_)
{
    pos = pos_;
}

void PositionalComponent::setTilt(float tilt_)
{
    tilt = tilt_;
}

RectComponent::RectComponent(const glm::vec4& rect_, EntityPtr entity) : PositionalComponent({rect.x + rect.z/2,rect.y + rect.a/2}, entity), ComponentContainer<RectComponent>(entity), rect(rect_)
{

}

glm::vec4 RectComponent::getBoundingRect() const
{
    return rect;
}

bool RectComponent::collidesRect(const glm::vec4& box) const
{
    return vecIntersect(rect,box);
}

bool RectComponent::collidesLine(const glm::vec4& line) const
{
    return lineInVec({line.x,line.y},{line.z,line.a},rect,tilt);
}

float RectComponent::distance(const glm::vec2& point) const
{
    return pointVecDistance(rect,point.x,point.y,tilt);
}

void RectComponent::setPos(const glm::vec2& pos)
{
    glm::vec2 dimen = {rect.z,rect.a};
    setRect({pos - 0.5f*dimen, dimen});
}

void RectComponent::setRect(const glm::vec4& rect_)
{
    rect = rect_;
    PositionalComponent::setPos({rect.x + rect.z/2,rect.y + rect.a/2});

}

CircleComponent::CircleComponent(const glm::vec2& center, float radius_, EntityPtr entity) : PositionalComponent(center, entity), ComponentContainer<CircleComponent>(entity), radius(radius_)
{

}

float CircleComponent::getRadius() const
{
    return radius;
}

glm::vec4 CircleComponent::getBoundingRect() const
{
    return glm::vec4(getPos() - glm::vec2(radius),glm::vec2(radius*2));
}

bool CircleComponent::collidesRect(const glm::vec4& box) const
{
    return pointVecDistance(box,getPos().x,getPos().y,0) <= radius;
}

bool CircleComponent::collidesLine(const glm::vec4& line) const
{
    return pointLineDistance(line,getPos()) <= radius;
}

float CircleComponent::distance(const glm::vec2& point) const
{
    float distance = glm::length(getPos()-point);
    return std::max(distance - radius, distance);
}

BasicMoveComponent::BasicMoveComponent(const glm::vec4& rect, EntityPtr& entity) : RectComponent(rect, entity), ComponentContainer<BasicMoveComponent>(entity)
{

}

void BasicMoveComponent::addMoveVec(const glm::vec2& moveVec_)
{
    setMoveVec(moveVec + moveVec_);
}

void BasicMoveComponent::setMoveVec(const glm::vec2& moveVec_)
{
    moveVec = moveVec_;
}

glm::vec2 BasicMoveComponent::getNextMoveVector()
{
    return (float)(DeltaTime::deltaTime)*moveVec;
}

void BasicMoveComponent::update()
{
    glm::vec2 move = getNextMoveVector();
    setPos(getPos() + move);

    moveVec = glm::vec2(0);
}

RenderComponent::RenderComponent(EntityPtr& entity, ZType zCoord_ ) : Component(entity), ComponentContainer<RenderComponent>(entity), zCoord(zCoord_)
{

}
RenderComponent::~RenderComponent()
{

}

RectRenderComponent::RectRenderComponent(EntityPtr& entity, const glm::vec4& color_) : color(color_),  RenderComponent(entity), ComponentContainer<RectRenderComponent>(entity)
{

}

void RectRenderComponent::update()
{
    if (Entity* entity = getEntity())
    {
        if (auto rect = entity->getComponent<PositionalComponent>())
        {
            glm::vec4 box = rect->getBoundingRect();
            PolyRender::requestRect(box,color,true,rect->getTilt(),0);
        }
    }
}

BaseAnimationComponent::BaseAnimationComponent(EntityPtr& entity, Sprite& sprite_, const BaseAnimation& anime_, ZType zCoord_) : RenderComponent(entity, zCoord_), ComponentContainer<BaseAnimationComponent>(entity),
                                                                                                 anime(anime_), spriteSheet(&sprite_)
{

}

Sprite* BaseAnimationComponent::getSpriteSheet()
{
    return spriteSheet;
}

void BaseAnimationComponent::update()
{
    if (Entity* entity = getEntity())
    if (PositionalComponent* rect = entity->getComponent<PositionalComponent>())
    {
        request(*ViewPort::animeProgram,rect->getBoundingRect(),zCoord,BaseAnimation::getFrameFromStart(start,anime),rect->getTilt());
    }
    if (start == 0)
    {
        start = SDL_GetTicks();
    }
}

/*SpriteParameter SpriteComponent::defaultSParam()
{
    SpriteParameter param;
    RectComponent* rect = entity->getComponent<RectComponent>();
    if (rect)
    {
        if (RenderCamera::currentCamera)
        {
            param.rect = RenderCamera::currentCamera->toScreen(rect->getRect());
        }
        else
        {
            param.rect = rect->getRect();
        }
        param.radians = rect->getTilt();
    }
    return param;
}*/

/*SpriteComponent::SpriteComponent(Sprite& sprite_, bool animated_, EntityPtr& entity) : RenderComponent(entity),
                                                                                                                ComponentContainer<SpriteComponent>(entity),
                                                                                                                        sprite(&sprite_),
                                                                                                                        animated(animated_)
{

}

void SpriteComponent::render(const SpriteParameter& param)
{
    if (sprite)
    {
            if (animated)
            {
               // static_cast<AnimationWrapper*>(sprite)->request(param,aParam);
            }
            else
            {
                SpriteManager::request(*sprite,param);
            }
    }

}

void SpriteComponent::setParam(const SpriteParameter& param, const AnimationParameter& animeParam)
{
    this->sParam = param;
    setAParam(animeParam);
}

void SpriteComponent::setAParam(const AnimationParameter& animeParam)
{
    this->aParam = animeParam;
}

void SpriteComponent::update()
{
        render(sParam);
        setParam(defaultSParam(),defaultAParam()); //reset params
}
*/
BaseHealthComponent::BaseHealthComponent(float invulnTime_, float health_,float maxHealth_, EntityPtr& entity) :
                                                                Component(entity), ComponentContainer<BaseHealthComponent>(entity),
                                                                invulnTime(invulnTime_), health(health_), maxHealth(maxHealth_) //health
{

}

void BaseHealthComponent::addHealth(float damage)
{
    if ((damage < 0 && !isInvuln()) || damage > 0) //only modify health if it is positive or if we are taking damage and are not invulnerable
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


}

float BaseHealthComponent::getHealth()
{
    return health;
}

float BaseHealthComponent::getMaxHealth()
{
    return maxHealth;
}

bool BaseHealthComponent::isInvuln()
{
    return !invuln.timePassed(invulnTime) && invuln.isSet(); //invulnerable if invincibility frames have passed and the timer was set
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

void Entity::updateOnce(int ID)
{
    if (taskID != ID)
    {
        update();
        setTaskID(ID);
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

IDComponent::IDComponent(EntityPtr& entity, const std::shared_ptr<EntityAssembler>& assembler_, std::string name_, int id_) :
                                                                                                Component(entity),
                                                                                                ComponentContainer<IDComponent>(entity),
                                                                                                assembler(assembler_),
                                                                                                name(name_),
                                                                                                id(id_)
{

}

IDComponent::IDComponent(EntityPtr& entity, std::string name_, int id_) : Component(entity),
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

EntityManager::~EntityManager()
{
    entities.clear();
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
    return it;
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
    int i = 0;
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
       i++;
    }
}

bool EntityPosManager::forEachEntity(Entity& entity)
{
    if (PositionalComponent* rect = entity.getComponent<PositionalComponent>())
    {
        glm::vec4 oldPos = rect->getBoundingRect();
        entity.update();
        grid->findNearest(*rect,[&entity](PositionalComponent& r1){
                              Entity* e1 = static_cast<PositionalComponent&>(r1).getEntity();
                              if (e1)
                              e1->collide(entity);
                              });
        grid->update(*rect,oldPos);
    }
    else
    {
        entity.update();
    }
    return false;
}

EntityPosManager::~EntityPosManager()
{
    if (grid.get())
    {
        grid->clear();
    }
}

void EntityPosManager::init(int gridSize)
{
    grid.reset(new SpatialGrid(gridSize));
}

SpatialGrid* EntityPosManager::getContainer()
{
    return grid.get();
}

void EntityPosManager::addEntity(const std::shared_ptr<Entity>& entity)
{
    EntityManager::addEntity(entity);
    if (auto rect = entity->getComponent<PositionalComponent>())
    {
        grid->insert(*rect);
    }
}

void EntityPosManager::addEntity(const std::shared_ptr<Entity>& (entity), float x, float y, bool centered)
{
    if (PositionalComponent* rect = entity->getComponent<PositionalComponent>())
    {
        rect->setPos({x - centered*rect->getBoundingRect().z/2, y - centered*rect->getBoundingRect().a/2}); //if centered, we have to adjust the position so that our center is over x and y
        addEntity(entity);
    }
}

EntityPosManager::EntityIt EntityPosManager::removeEntity(Entity* entity)
{
    if (entity && entities.find(entity) != entities.end())
    {
        if (PositionalComponent* pos = entity->getComponent<PositionalComponent>())
        {
            grid->remove(*pos);
        }
        return EntityManager::removeEntity(entity);
    }
    return entities.end();
}

void EntityPosManager::update()
{
    EntityManager::update(); //REFACTOR: slightly inefficient to update all entities then find collisions for each entity
}

void EntityPosManager::reset()
{
    entities.clear();
    grid->clear();
}

