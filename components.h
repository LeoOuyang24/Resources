#ifndef COMPONENTS_H_INCLUDED
#define COMPONENTS_H_INCLUDED

#include <memory.h>
#include <unordered_map>
#include <typeinfo>


#include "glGame.h"

class Entity;
class Component;
class ResourceComponent;
class HealthComponent;
class ClickableComponent;
class RenderComponent;
template<class C>
class ComponentContainer
{
private:
    static std::unordered_map<Entity*,Component*> components;
    static std::unordered_map<ComponentContainer*, Entity*> entities;
    void insert(Entity* entity)
    {
  //          std::cout <<  typeid(C).name() << " Inserting: " <<components.size() << " " << entities.size() << std::endl;
        if (entity)
        {
           // std::cout <<"Health: " << entity << " " <<   ComponentContainer<HealthComponent>::entities.size() << " " << ComponentContainer<HealthComponent>::components.size() << std::endl;
          //  std::cout <<"Clickable: " << entity << " " <<   ComponentContainer<ClickableComponent>::entities.size() << " " << ComponentContainer<ClickableComponent>::components.size() << std::endl;
            C* ptr = static_cast<C*>(this);
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
    static Component* getComponent(Entity* e)
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
std::unordered_map<Entity*,Component*> ComponentContainer<C>::components;

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
    void setRect(const glm::vec4& rect);
    virtual void setPos(const glm::vec2& pos);
    glm::vec2 getPos();
    glm::vec2 getCenter();
    virtual ~RectComponent();
};

class MoveComponent : public RectComponent, public ComponentContainer<MoveComponent>
{
    static constexpr float distThreshold = .001; //max distance an entity can be away from a point and still be considered to be on that point
    double angle = 0; //Direction we are moving in. This would be calculated every update call if this wasn't a member variable
protected:
    double baseSpeed = 0;
    double speed = 0;
    double velocity = 0; //the actual amount moved this frame.
    glm::vec2 target; //point to move towards
public:
    MoveComponent(double speed, const glm::vec4& rect, Entity& entity);
    void teleport(const glm::vec2& point); //centers the entity at the point and sets it as the new target
    glm::vec2 getNextPoint(); //gets the projected center to move towards
    virtual void update();
    virtual bool atPoint(const glm::vec2& point); //whether or not our center is within distThreshold of the point.
    bool atTarget(); //returns atPoint(target);
    virtual void setTarget(const glm::vec2& point);
    virtual const glm::vec2& getTarget();
    double getVelocity();
    double getBaseSpeed();
    double getCurSpeed();
    void setSpeed(double newSpeed); //sets the speed for this frame only
    virtual ~MoveComponent();

};

struct ForceVector
{
    float angle;
    float magnitude;
};

class ForcesComponent : public Component, public ComponentContainer<ForcesComponent> //component that pushes MoveComponent based on what forces are currently being applied
{
    glm::vec2 finalForce; //after applying all forces, this is the final x and y displacement to move
    std::list<ForceVector> forces;
    MoveComponent* move = nullptr; //forcesComponent will only work if there is a moveComponent on the entity
public:
    ForcesComponent(Entity& entity);
    void addForce(ForceVector force);
    void update();
    glm::vec2 getFinalForce()
    {
        return finalForce;
    }


};

class RenderComponent : public Component, public ComponentContainer<RenderComponent>
{
protected:
    RenderCamera* camera = nullptr;
public:
    RenderComponent(Entity& entity, RenderCamera* camera);
    virtual void render(const SpriteParameter& param);
    virtual ~RenderComponent();
};

class Entity
{
protected:
    std::vector<std::shared_ptr<Component>> components;
public:
     Entity();
    void update();
    void collide(Entity& entity);
    template<typename T>
    T* getComponent()
    {
        return (static_cast<T*>(ComponentContainer<T>::getComponent(this)));
    }
    void addComponent(Component& comp);
    template<typename T>
    void removeComponent()
    {
        T* ptr = getComponent<T>();
        if (ptr)
        {
            int size = components.size();
            for (int i = 0; i < size; ++i)
            {
                if (components[i].get() == ptr)
                {
                    components.erase(components.begin() + i);
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



#endif // COMPONENTS_H_INCLUDED
