#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <memory.h>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>
#include <list>

#include "glGame.h"
#include "render.h"
#include "SDLhelper.h"

class Entity;
class Component;



template<class C>
class ComponentContainer
{
private:
    static std::unordered_map<Entity*,C*> components; //use raw pointers because smart pointers are quite hard to manage, since they all have to point to the same thing and copy from one another
    static std::unordered_map<ComponentContainer*, Entity*> entities;

    void insert(Entity* entity)
    {
    //          std::cout <<  typeid(C).name() << " Inserting: " <<components.size() << " " << entities.size() << std::endl;
        if (entity)
        {
           // std::cout <<"Health: " << entity << " " <<   ComponentContainer<HealthComponent>::entities.size() << " " << ComponentContainer<HealthComponent>::components.size() << std::endl;
          //  std::cout <<"Clickable: " << entity << " " <<   ComponentContainer<ClickableComponent>::entities.size() << " " << ComponentContainer<ClickableComponent>::components.size() << std::endl;
             C* ptr = static_cast<C*>(this); //the expectation is that anything that wants to be stored in a ComponentContainer has to also inherit form it, so this cast is safe
            //components.insert(std::pair<Entity*,Component*>(entity,ptr));
            components[entity] = ptr;
            entities[this] = entity;
           // entities.insert(std::pair<ComponentContainer*,Entity*>(this,entity));
        }
          //  std::cout <<  typeid(C).name() << " Inserting2: " << components.size() << " " << entities.size() << std::endl;

    }
    void remove() //this only removes the pointers; it is expected that whatever actually owned the Component will delete it
    {
        //std::cout << typeid(C).name() << " Deleting: " << components.size() << " " << entities.size()   << std::endl;
        if (entities.count(this) > 0)
       {
            components.erase(components.find(entities[this]));
            entities.erase(entities.find(this));
       }
      // Component* ptr = reinterpret_cast<Component*>(this);

    }

public:
    ComponentContainer<C>(Entity* entity)
    {
        insert(entity);
    }
    ComponentContainer<C>(Entity& entity)
    {
        insert(&entity);
    }
    virtual ~ComponentContainer()
    {
        //std::cout << (components.find(entities[this]) == components.end()) << std::endl;
              //  std::cout << typeid(this).name() << " Deleting: " <<components.size() << " " << entities.size()<< std::endl;
        remove();
                   // std::cout << typeid(this).name() << " Deleting2: " <<components.size()  << " " << entities.size()<< std::endl;

    }
    static C* getComponent(Entity* e)
    {
        if (components.count(e) > 0)
        {
            return components[e];
        }
        else
        {
            return nullptr;
        }
    }

};

template<class C>
std::unordered_map<Entity*,C*> ComponentContainer<C>::components;

template<class C>
std::unordered_map<ComponentContainer<C>*, Entity*> ComponentContainer<C>::entities;


class Component : public ComponentContainer<Component>
{
protected:
    Entity* entity;

public:
    Component(Entity& owner);
    virtual void update();
    virtual void collide(Entity& other);
    virtual void onDeath();
    Entity& getEntity();
    virtual ~Component();
};

class RectComponent : public Component, public ComponentContainer<RectComponent>, public RectPositional
{
public:
    RectComponent(const glm::vec4& rect, Entity& entity);
    bool collides(const glm::vec4& rect);
    void setRect(const glm::vec4& rect);
    virtual void setPos(const glm::vec2& pos);
    void setCenter(const glm::vec2& center);
    glm::vec2 getPos();
    glm::vec2 getCenter();

    virtual ~RectComponent();
};

class MoveComponent : public RectComponent, public ComponentContainer<MoveComponent>
{
    float angle = 0; //Direction we are moving in. This would be calculated every update call if this wasn't a member variable
protected:
    static constexpr float distThreshold = .001; //max distance an entity can be away from a point and still be considered to be on that point
    float baseSpeed = 0;
    float speed = 0;
    float velocity = 0; //the actual amount moved this frame.
    bool ignoreTarget = 0; //sometimes, we just want to move according to the speed and not worry about target. This allows us to do that
                            //only speed and angle will be used to calculate movement
    glm::vec2 target; //point to move towards
public:
    MoveComponent(float speed, const glm::vec4& rect, Entity& entity);
    void teleport(const glm::vec2& point); //centers the entity at the point and sets it as the new target
    virtual glm::vec2 getNextPoint(); //gets the next point to move towards. (TOP LEFT CORNER (x,y), not center)
    virtual void update();
    virtual bool atPoint(const glm::vec2& point); //whether or not our center is within distThreshold of the point.
    virtual bool atTarget(); //returns atPoint(target);
    virtual void setTarget(const glm::vec2& point);
    virtual const glm::vec2& getTarget();
    void setTiltTowardsTarget();
    virtual void setPos(const glm::vec2& pos); //sets both top left corner and target.
    void setAngle(float val); //really only useful if ignoreTarget is true, since angle is otherwise manually calculated
    float getAngle();
    float getVelocity();
    float getBaseSpeed();
    float getCurSpeed();
    void setSpeed(float newSpeed); //sets the speed for this frame only
    void setIgnoreTarget(bool val);
    virtual ~MoveComponent();

};

class RealMoveComponent : public MoveComponent, public ComponentContainer<RealMoveComponent> //move component that speeds up and slows down as it leaves/approaches the destination
{
    //class doesn't really work with ignoreTarget, so update will almost certainly have to be overriten
    float accel = 0, decel = 0; //accel and decel = maxSpeed if they are set to 0
    float maxSpeed = 0; //given a target, represents the maximum speed before having to decelerate
protected:
    void accelerate();
    void decelerate();
public:
    RealMoveComponent(float accel_, float deccel_, float speed, const glm::vec4& rect, Entity& entity);
    void setTarget(const glm::vec2& point);
    float getAccel();
    float getDecel();
    virtual void update();
    virtual ~RealMoveComponent()
    {

    }
};

class RenderComponent : public Component, public ComponentContainer<RenderComponent>
{
public:
    RenderComponent(Entity& entity);
    virtual void render(const SpriteParameter& param);
    virtual ~RenderComponent();
};

class RectRenderComponent : public RenderComponent, public ComponentContainer<RectRenderComponent>
{
    glm::vec4 color;
public:
    RectRenderComponent(Entity& entity, const glm::vec4& color_);
    virtual void render(const SpriteParameter& param);
    void update();
};

class SpriteComponent : public RenderComponent, public ComponentContainer<SpriteComponent> //also handles Animations
{
    bool animated = false; //whether it's an animation or sprite
    bool modified = false; //by default, the SpriteComponent will attempt to render at the entity's position. If either sParam or aParam are modified, this
                            //component will render according to sParam and aParam
protected:
    SpriteWrapper* sprite = nullptr;
    SpriteParameter sParam;
    AnimationParameter aParam;
    virtual SpriteParameter defaultRender(); //returns the sprite parameter used if modified is false
public:
    SpriteComponent(SpriteWrapper& sprite_, bool animated_, Entity& entity); //loads a sprite or animation
    virtual void render(const SpriteParameter& param);
    void setParam(const SpriteParameter& param, const AnimationParameter& animeParam = AnimationParameter()); //set modified to true if you don't want to call default render
    void setAParam(const AnimationParameter& animeParam); //set only aParam so sParam isn't overriden
    void update(); //renders the sprite, taking the RenderCamera into account automatically
    virtual ~SpriteComponent()
    {

    }
};

class BaseHealthComponent : public Component, public ComponentContainer<BaseHealthComponent>
{
protected:
    DeltaTime invuln;
    float invulnTime; //how long to be invincible for after taking damage (milliseconds)
    float maxHealth = 0; //set maxHealth to be negative one to basically have no max health limit
    float health = 0;
public:
    BaseHealthComponent(float invulnTime_,float health_, float maxHealth_, Entity& entity);
    virtual void addHealth(float damage); //damage can be positive or negative
    float getHealth();
    bool isInvuln(); //return true if still invulnerable
    virtual ~BaseHealthComponent()
    {

    }
};


class Entity
{
protected:
    std::unordered_map<Component*,std::shared_ptr<Component>> components;
    TASK_ID taskID = 0; //this ID makes it easy to track if an entity has already called update this frame.
public:
     Entity();
    void update();
    void updateOnce(TASK_ID ID); //given a task ID, check if this entity matches it; if so, then this entity has already been updated this frame and should not be updated again), otherwise, update() as usual
    void setTaskID(TASK_ID ID)
    {
        taskID = ID;
    }
    TASK_ID getTaskID()
    {
        return taskID;
    }
    void collide(Entity& entity);
    template<typename T>
    T* getComponent()
    {
        return (static_cast<T*>(ComponentContainer<T>::getComponent(this)));
    }
    std::shared_ptr<Component> getComponent(Component* comp)
    {
        return components[comp];
    }
    template<typename T>
    std::shared_ptr<T> getComponentPtr() //returns the smart pointer
    {
        return std::static_pointer_cast<T>(getComponent(getComponent<T>()));
    }
    void addComponent(Component& comp);
    template<typename T>
    void removeComponent()
    {
        T* ptr = getComponent<T>();
        if (ptr)
        {
            auto end = components.end();
            for (auto i = components.begin(); i != end; ++i)
            {
                if (i->second.get() == ptr)
                {
                    components.erase(i);
                    break;
                }
            }
        }
    }
    void onDeath();
    virtual ~Entity();

};

class EntityAssembler //returns an entity with certain components attached. Unique to different entities
{
public:
    virtual Entity* assemble(); //returns an entity with components attached on the heap. Does not clean up the memory!
};

class IDComponent : public Component, public ComponentContainer<IDComponent> //helps with storing ID info about entity
{
public:
    const std::weak_ptr<EntityAssembler> assembler;
    const std::string name;
    const int id;
    IDComponent(Entity& entity, const std::shared_ptr<EntityAssembler>& assembler_, std::string name_ = "", int id_ = -1);
    IDComponent(Entity& entity, std::string name_ = "", int id_ = -1); //used if you don't care about assembler
};

class EntityManager //convenient class for storing and updating each entity
{
protected:
    std::unordered_map<Entity*,std::shared_ptr<Entity>> entities;
    typedef std::unordered_map<Entity*,std::shared_ptr<Entity>>::iterator EntityIt;
    virtual bool forEachEntity(Entity& it); //used to allow child managers to easily change how they manage entities without forlooping twice
                                            //returns true if the entity should be deleted after this function call
public:
    virtual void addEntity(Entity& entity);
    virtual void addEntity(const std::shared_ptr<Entity>& entity);
    virtual EntityIt removeEntity(Entity* entity);
    std::shared_ptr<Entity> getEntity(Entity* ptr);
    virtual void update();
};

class EntityPosManager : public EntityManager//Entity Manager that also keeps a quadtree to track entity position
{
    std::unique_ptr<SpatialGrid> grid;
protected:
    virtual bool forEachEntity(Entity& entity);
public:
    virtual void init(int gridSize);
    SpatialGrid* getContainer(); //can return null, most likely because init was never called
    using EntityManager::addEntity;
    virtual void addEntity(const std::shared_ptr<Entity>& ptr);
    virtual void addEntity(Entity& entity, float x, float y, bool centered = true); //sets center position if centered is true, otherwise sets top left corner
    EntityIt removeEntity(Entity* entity);
    void update();
    void reset();
};


#endif // COMPONENTS_H_INCLUDED
