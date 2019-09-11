#include "sparkles.h"

    Particle::Particle(glm::vec3 Color, Sprite& s, int x, int y)
    {
        color = Color;
        sprite = &s;
        coords.x = x;
        coords.y = y;
    }
    void Particle::update()
    {

    }
    glm::vec3 Particle::getColor()
    {
        return color;
    }
    glm::vec2 Particle::getCoords()
    {
        return coords;
    }

    ParticleSystem::ParticleSystem()
    {
    }
    void ParticleSystem::addParticle(Particle* p)
    {
        particles.push_back(p);
    }
    void ParticleSystem::render(RenderProgram& program,Sprite& s)
    {
        int size = particles.size();
        float colors[size*3];
        std::vector<SpriteParameter> pos;
        int j = 0;
        for (int i = 0; i < size; ++i)
        {
            particles[i]->update();
            glm::vec3 color = particles[i]->getColor();
            colors[j] = color.x;
            colors[j+1] = color.y;
            colors[j+2] = color.z;
            j+= 3;
            pos.push_back({{particles[i]->getCoords(),16,16}});
        }
        GLuint VAO = s.getVAO();
        glBindVertexArray(VAO);
    GLuint colorBuffer;
        glGenBuffers(1,&colorBuffer);

        glBindBuffer(GL_ARRAY_BUFFER,colorBuffer);
        glBufferData(GL_ARRAY_BUFFER,size*3*sizeof(float), colors,GL_STATIC_DRAW);
        glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,0);
        glVertexAttribDivisor(2,1);
        glEnableVertexAttribArray(2);
       s.renderInstanced(program,pos);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);

    }

