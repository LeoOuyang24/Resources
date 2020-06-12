#ifndef SPRITES_H_INCLUDED
#define SPRITES_H_INCLUDED

#include <iostream>

#include <vector>
#include <map>
#include "glew.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"



int loadShaders(const GLchar* source, GLenum shaderType );
double pointDistance(const glm::vec2& v1, const glm::vec2& v2);
glm::vec2 midpoint(const glm::vec4& v1); //midpoint of a line
bool vecIntersect(const glm::vec4& vec1,const glm::vec4& vec2);
glm::vec4 vecIntersectRegion(const glm::vec4& vec1, const glm::vec4& vec2); //returns the region of two colliding rects
bool vecContains(glm::vec4 r1, glm::vec4 r2); //returns true if r2 contains r1
bool pointInVec(const glm::vec4& vec1, double x, double y, double angle = 0); //angle of vec1 is by default 0
double pointVecDistance(const glm::vec4& vec, float x, float y); //shortest distance from the point to the rectangle
double pointLineDistance(const glm::vec4& line, const glm::vec2& point); //rotates line until is is parallel to the x-axis, then computes distance from point to line
bool lineInVec(const glm::vec2& p1, const glm::vec2& p2, const  glm::vec4& r1, double angle = 0); //angle of r1 is by default 0
bool lineInLine(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2);
class RenderProgram;
void drawLine(RenderProgram& program,glm::vec3 color,const std::vector<glm::vec4>& points);
void drawCircle(RenderProgram& program, glm::vec3 color,double x, double y, double radius);
void drawNGon(RenderProgram& program, const glm::vec3& color, const glm::vec2& center, double radius, int n, double angle);
glm::vec2 rotatePoint(const glm::vec2& p, const glm::vec2& rotateAround, double angle); //angle in radians
void drawRectangle(RenderProgram& program, const glm::vec3& color, const glm::vec4& rect, double angle);
void addPointToBuffer(float buffer[], glm::vec3 point, int index);
void addPointToBuffer(float buffer[], glm::vec2 point, int index);
void addPointToBuffer(float buffer[], glm::vec4 point, int index);
void printRect(const glm::vec4& rect);

class RenderProgram
{

    unsigned int program;
    static int screenWidth, screenHeight;
    static glm::vec2 xRange, yRange, zRange; //represents the smallest and largest values x,y,z can be
public:
    static GLuint VBO, VAO;
    static RenderProgram basicProgram, lineProgram; //basic allows for basic sprite rendering. Line program is simpler and renders lines.
    static RenderProgram paintProgram; //renders something using only one color
    RenderProgram(std::string vertexPath, std::string fragmentPath);
    RenderProgram()
    {

    }
    static const glm::vec2& getXRange();
    static const glm::vec2& getYRange();
    static const glm::vec2& getZRange();
    static void setXRange(float x1, float x2);
    static void setYRange(float y1, float y2);
    static void setZRange(float z1, float z2);
    static glm::mat4 getOrtho();
    static glm::vec2 getScreenDimen();
    void setMatrix4fv(std::string name, const GLfloat* value);
    void setVec3fv(std::string name,glm::vec3 value);
    void setVec4fv(std::string name, glm::vec4 value);
    void setVec2fv(std::string name, glm::vec2 value);
    void use();
    void init(std::string vertexPath, std::string fragmentPath);
    static void init(int screenWidth, int screenHeight); //this init function initiates the basic renderprograms

};

//extern RenderProgram RenderProgram::basicProgram;
//extern RenderProgram RenderProgram::lineProgram;

enum RenderEffect
{
    NONE,
    MIRROR
};

struct SpriteParameter //stores a bunch of information regarding how to render the sprite
{
     glm::vec4 rect = {0,0,0,0};
    float radians = 0;
    RenderEffect effect = NONE; //effects to do. (mirror, flip, etc)
    glm::vec4 tint = {1,1,1,1};
    RenderProgram* program = &RenderProgram::basicProgram;
    float z = 0;
    glm::vec4 portion = {0,0,1,1};
   // std::vector<float> vertices = {};
    //std::vector<int> indices = {};
};

class SpriteWrapper;
class Sprite
{
    friend SpriteWrapper;
protected:
    int width = 0, height = 0;
    unsigned int texture = -1;
    float values[16] = {
    -1, 1, 0, 1,
    1, 1, 1, 1,
    -1, -1, 0, 0,
    1, -1, 1, 0

    };
    int indices[6] = {
    0,1,3,
    0,2,3
    };
    bool defaultVerticies = false; //whether or not the last time this image rendered, the values or indices were changed
    glm::vec3 tint;
    unsigned int VBO=-1,modVBO = -1, VAO=-1;
    static const int floats; //the number of floats we pass everytime we render an instance/spriteParameter
    static const int floatSize; //size of floats in bytes;
    void load(std::string source,bool transparent);
    virtual void loadData(GLfloat* data, const SpriteParameter& parameter, int index);
    void draw( RenderProgram& program, GLfloat* data, int instances); //draws the sprite. Assumes ModVBO has already been loaded
public:
    Sprite(std::string source, bool transparent);
    Sprite()
    {
        texture = -1;
    }
    ~Sprite();
    void init(std::string source, bool transparent);
    void loadVertices();
    void loadVertices(const std::vector<float>& verticies);
    template<class T>void loadBuffer(unsigned int& buffer, int location, T arr[], int size, int dataSize, int divisor = 0); //data size is how much data per entry. Divisor is how for glvertexAttribDivisor. Default 0
  //  virtual void render(RenderProgram& program, SpriteParameter parameter);
    virtual void renderInstanced(RenderProgram& program, const std::vector<SpriteParameter>& parameters);
    unsigned int getVAO();
    virtual int getFloats(); //# of floats per SpriteParameter is different for each class, so this function just returns the version for each child of Sprite
    void reset(); //clears all buffers and resets modified back to values
    void setTint(glm::vec3 color);
    void mirror();
    void flip();
   // void map(RenderProgram& program,double width, double height,const glm::vec4& base, const std::vector<glm::vec2>& points);
};

class Sprite9 : public Sprite // This sprite has been split into 9 sections that each scale differently. The corners aren't scaled at all, the top and bottom
{                               //are only scaled horizontally, the sides are only scaled vertically, and the center can be scaled any which way.
    static const int floats9; //the number of floats per SpriteParameter for Sprite9. Each spriteParameter for Sprite9 is rendered 9 different times so this is 9*Sprite::floats
    glm::vec2 widths; //the widths of the frame on either side;
    glm::vec2 heights; //heights of the frame on either side;
    void loadData(GLfloat* data, const SpriteParameter& parameter, int index);
public:
    Sprite9(std::string source, bool transparent, glm::vec2 W, glm::vec2 H);
    Sprite9()
    {

    }
    int getFloats();
    void init(std::string source, bool transparent, glm::vec2 W, glm::vec2 H);

};

struct AnimationParameter : public SpriteParameter//the main difference between this class and SpriteParameters is that this one provides the time at which the animation started and the fps
{
    double start = -1; //the time at which the animation started, -1 if it hasn't started
    double fps = -1; //the fps for the animation. -1 means use the default


};

class BaseAnimation : public Sprite //the actual animation object
{
    double fps = 0;
    glm::vec2 frameDimen; //proportion of the spritesheet of each frame
    glm::vec4 subSection = {0,0,0,0}; //subsection.xy is the origin of the sprite sheet. subsection.za is the framesPerRow and the number of rows wanted. This is a standardized value (0-1).
public:
    BaseAnimation(std::string source, bool transparent, double speed, int perRow, int rows, const glm::vec4& sub = {0,0,0,0});
    BaseAnimation()
    {

    }
    void init(std::string source, bool transparent, double speed, int perRow, int rows, const glm::vec4& sub = {0,0,0,0}); //how many frames per row and how many rows there are
    void renderInstanced(RenderProgram& program, const std::vector<AnimationParameter>& parameters);
};

class SpriteWrapper
{
protected:
    Sprite* spr = nullptr;
    std::vector<SpriteParameter> parameters;
public:
    void init(std::string source, bool transparent);
    void init(Sprite* spr);
    virtual void reset();
    virtual void render();
    void request(SpriteParameter&& param);
    ~SpriteWrapper();
};

class AnimationWrapper : public SpriteWrapper
{
    std::vector<AnimationParameter> aParameters;
public:
    void init(BaseAnimation* a);
    void reset();
    void render();
    void request(AnimationParameter&& param);
    ~AnimationWrapper();
};

class SpriteManager
{
    static std::vector<SpriteWrapper*> sprites;
public:
    static void addSprite(SpriteWrapper& spr);
    static void render();

};

struct PolyRender
{
    static std::vector<std::pair<glm::vec3,glm::vec4>> lines; //lines and their colors
    static std::vector<std::pair<int,glm::vec4>> polygons; //number of edges and color of each polygon
    static std::vector<glm::vec3> polyPoints; //points of polygons
    static RenderProgram polyRenderer;
    static unsigned int VAO;
    static unsigned int lineVBO;
    static unsigned int polyVBO;
    static unsigned int colorVBO;
    static void init(int screenWidth, int screenHeight);
    static void requestLine(const glm::vec4& line, const glm::vec4& color, float z = 0);
    static void requestCircle(const glm::vec4& color,double x, double y, double radius);
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z);
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z); //draws a regular n gon. Angle is in radians
    static void requestPolygon(const std::vector<glm::vec3>& points, const glm::vec4& color);
    static void render();
    static void renderMesh(float* mesh, int w, int h);
private:
    static void renderLines();
    static void renderPolygons();
    static unsigned short restart; //restart indice
};

class RenderController //controls all rendering
{
    //std::map
};

#endif // SPRITES_H_INCLUDED
