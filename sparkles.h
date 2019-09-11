#ifndef SPARKLES_H_INCLUDED
#define SPARKLES_H_INCLUDED
#include "render.h"

class Particle
{
    Sprite* sprite;
protected:
    glm::vec3 color;
    glm::vec2 coords;
public:
    Particle(glm::vec3 Color, Sprite& s, int x, int y);
    virtual void update();
    glm::vec3 getColor();
    glm::vec2 getCoords();


};


class ParticleSystem
{
    std::vector<Particle*> particles;
public:
    ParticleSystem();
    void addParticle(Particle* p);
    void render(RenderProgram& program, Sprite& s);
};

#endif // SPARKLES_H_INCLUDED
