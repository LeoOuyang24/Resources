#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <memory.h>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>
#include <list>
#include <functional>

#include "render.h"
#include "SDLhelper.h"

class Entity;
class Component;

typedef std::shared_ptr<Entity> EntityPtr;


template<class C>
class ComponentContainer
{
private:
    //for now, I think raw pointers are safe.
    //The constructor ensures that our pointer can't be dangling
    //Insert and remove are both safe
    //The Entity destructor ensures that it and all its components are removed
    static std::unordered_map<Entity*,C*> components; //used to find components given an entity; use raw pointers because smart pointers are quite hard to manage, since they all have to point to the same thing and copy from one another
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
        auto found = entities.find(this);
        if (found != entities.end())
       {
            components.erase(components.find(found->second));
            entities.erase(found);
       }
      // Component* ptr = reinterpret_cast<Component*>(this);

    }

public:
    ComponentContainer(EntityPtr entity)
    {
        insert(entity.get());
    }
    virtual ~ComponentContainer()
    {
        remove();
    }
    static C* getComponent(Entity* e)
    {
        auto found = components.find(e);
        if (found != components.end())
        {
            return found->second;
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
    std::weak_ptr<Entity> entity;

public:
    Component(EntityPtr owner);
    virtual void update();
    virtual void collide(Entity& other);
    virtual void onDeath();
    Entity* getEntity();
    virtual ~Component();
};


class PositionalComponent : public Component, public ComponentContainer<PositionalComponent>
{
protected:
    glm::vec2 pos;
    float tilt = 0;
public:
    PositionalComponent(const glm::vec2& pos, EntityPtr entity);

    virtual glm::vec2 getPos() const;
    virtual glm::vec2 getCenter() const;
    virtual float getTilt() const;
    virtual glm::vec4 getBoundingRect() const;

    virtual bool collidesRect(const glm::vec4& box) const; //how to determine whether or not this thing collides with a rect (used in quadtree)
    virtual bool collidesLine(const glm::vec4& line) const; //returns if this collides with a line
    virtual float distance(const glm::vec2& point) const; //finds how far this positional is from a point (used in to find all objects within a distance

    virtual void setPos(const glm::vec2& pos);
    void setTilt(float tilt_);
};

//rectangular hitbox.
//"pos" becomes the center of the rectangle
class RectComponent : public PositionalComponent, public ComponentContainer<RectComponent>
{
    glm::vec4 rect;
public:
    RectComponent(const glm::vec4& rect, EntityPtr entity);

    glm::vec4 getBoundingRect() const;

    bool collidesRect(const glm::vec4& box) const;
    bool collidesLine(const glm::vec4& line) const;
    float distance(const glm::vec2& point) const;

    void setPos(const glm::vec2& pos);
    void setRect(const glm::vec4& rect);
};

class CircleComponent : public PositionalComponent, public ComponentContainer<CircleComponent>
{
    float radius = 1;
public:
    CircleComponent(const glm::vec2& point, float radius, EntityPtr entity);
    float getRadius() const;

    glm::vec4 getBoundingRect() const;

    bool collidesRect(const glm::vec4& box) const;
    bool collidesLine(const glm::vec4& line) const;
    float distance(const glm::vec2& point) const;
};

class BasicMoveComponent : public RectComponent, public ComponentContainer<BasicMoveComponent>
{
protected:
    glm::vec2 moveVec = glm::vec2(0); //the direction we will be moving in this frame, reset every frame
public:
    BasicMoveComponent(const glm::vec4& rect, EntityPtr& entity);
    void addMoveVec(const glm::vec2& moveVec_); //add to moveVec
    void setMoveVec(const glm::vec2& moveVec_);
    virtual glm::vec2 getNextMoveVector(); //returns the next moveVector to add to position. Usually just moveVec*deltaTime
    void update(); //move moveVec amount and then reset moveVec
};

class RenderComponent : public Component, public ComponentContainer<RenderComponent>
{
protected:
    ZType zCoord = 0; //the z to render at, optional usage
public:
    RenderComponent(EntityPtr& entity, ZType zCoord_ = 0);
    virtual ~RenderComponent();

};

class RectRenderComponent : public RenderComponent, public ComponentContainer<RectRenderComponent>
{
    glm::vec4 color;
public:
    RectRenderComponent(EntityPtr& entity, const glm::vec4& color_);
    //virtual void render(const SpriteParameter& param);
    void update();
};

class BaseAnimationComponent : public RenderComponent, public ComponentContainer<BaseAnimationComponent>
{
    //very basic animation rendering component
protected:
    Uint32 start = 0; //millisecond we started rendering animation, 0 if we haven't started
    BaseAnimation anime;
    Sprite* spriteSheet = 0;
public:
    BaseAnimationComponent(EntityPtr& entity, Sprite& sprite, const BaseAnimation& anime, ZType zCoord_ = 0);
    template<typename... T>
    void request(BasicRenderPipeline& program, const glm::vec4& rect, ZType z, const glm::vec4& subSection, T... stuff) //pass subSection to SpriteManager, as well as any other arguments
    {
        //a shader that expects a BaseAnimation request needs a rect, z, and the subsection of the sprite sheet
        SpriteManager::requestSprite({program,spriteSheet},rect,z,subSection,stuff...);
    }
    Sprite* getSpriteSheet();
    virtual void update();
};

class BaseHealthComponent : public Component, public ComponentContainer<BaseHealthComponent>
{
protected:
    DeltaTime invuln;
    float invulnTime; //how long to be invincible for after taking damage (milliseconds)
    float maxHealth = 0; //set maxHealth to be negative one to basically have no max health limit
    float health = 0;
public:
    BaseHealthComponent(float invulnTime_,float health_, float maxHealth_, EntityPtr& entity);
    virtual void addHealth(float damage); //damage can be positive or negative
    float getHealth();
    float getMaxHealth();
    bool isInvuln(); //return true if still invulnerable
    virtual ~BaseHealthComponent()
    {

    }
};

typedef int TASK_ID;

class Entity
{
    //for now, please do not copy nor move Entities
    Entity(Entity& )
    {

    }
    Entity(Entity&&)
    {

    }
    void operator=(Entity&)
    {

    }
    void operator=(Entity&&)
    {

    }
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

namespace {
    void checkComponentsBase(auto func)
    {
        //func();
    }

    /*template<typename Component1>
    void checkComponentsHelper(Entity* entity, auto func)
    {
        if (Component1* ptr = entity->getComponent<Component1>())
        {
            checkComponentsBase(entity,func);
        }
    }*/

    template<typename Component1, typename... Components>
    void checkComponentsHelper(Entity* entity, auto func)
    {
        if (Component1* ptr = entity->getComponent<Component1>())
        {
            if constexpr ((sizeof...(Components) == 0))
            {
                std::bind_front(func,ptr)();
            }
            else
            {
                checkComponentsHelper<Components...>(entity,std::bind_front(func,ptr));
            }
        }
    }

}
template<typename... Components>
void checkComponents(Entity* entity, auto func)
{
    if (entity)
    {
        checkComponentsHelper<Components...>(entity,std::bind_front(func,entity));
    }
}


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
    IDComponent(EntityPtr& entity, const std::shared_ptr<EntityAssembler>& assembler_, std::string name_ = "", int id_ = -1);
    IDComponent(EntityPtr& entity, std::string name_ = "", int id_ = -1); //used if you don't care about assembler
};

class EntityManager //convenient class for storing and updating each entity
{
protected:
    std::unordered_map<Entity*,std::shared_ptr<Entity>> entities;
    typedef std::unordered_map<Entity*,std::shared_ptr<Entity>>::iterator EntityIt;
    virtual bool forEachEntity(Entity& it); //used to allow child managers to easily change how they manage entities without forlooping twice
                                            //returns true if the entity should be deleted after this function call
public:
    ~EntityManager();
    virtual void addEntity(const std::shared_ptr<Entity>& entity);
    virtual EntityIt removeEntity(Entity* entity);
    std::shared_ptr<Entity> getEntity(Entity* ptr);
    virtual void update();
};

class SpatialGrid;
class EntityPosManager : public EntityManager//Entity Manager that also keeps a quadtree to track entity position
{
    std::unique_ptr<SpatialGrid> grid;
protected:
    virtual bool forEachEntity(Entity& entity);
public:
    ~EntityPosManager();
    virtual void init(int gridSize);
    SpatialGrid* getContainer(); //can return null, most likely because init was never called
    virtual void addEntity(const std::shared_ptr<Entity>& ptr);
    virtual void addEntity(const std::shared_ptr<Entity>& (entity), float x, float y, bool centered = true);
    EntityIt removeEntity(Entity* entity);
    void update();
    void reset();
};


#endif // COMPONENTS_H_INCLUDED
