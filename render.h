#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include <iostream>

#include <vector>
#include <map>
#include <list>
#include "glew.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "SDLhelper.h";
#include "vanilla.h"

class RenderProgram;
void addPointToBuffer(float buffer[], glm::vec3 point, int index);
void addPointToBuffer(float buffer[], glm::vec2 point, int index);
void addPointToBuffer(float buffer[], glm::vec4 point, int index);

struct ViewRange
{
    static float getXRange(const ViewRange& range)
    {
        return abs(range.xRange[1] - range.xRange[0]);
    }
    static float getYRange(const ViewRange& range)
    {
        return abs(range.yRange[1] - range.yRange[0]);
    }
    static float getZRange(const ViewRange& range)
    {
        return abs(range.zRange[1] - range.zRange[0]);
    }
    glm::vec2 xRange = {0,0};
    glm::vec2 yRange = {0,0};
    glm::vec2 zRange = {0,0};
};

class RenderProgram
{

    unsigned int program;
    static int screenWidth, screenHeight;
    static ViewRange baseRange;  //represents the smallest and largest values x,y,z can be. X and Y should always have 0 as the smallest value.
    static ViewRange currentRange; //represents the current range for x,y, and z
public:
    static GLuint VBO, VAO;
    static RenderProgram basicProgram, lineProgram; //basic allows for basic sprite rendering. Line program is simpler and renders lines.
    static RenderProgram paintProgram; //renders something using only one color
    RenderProgram(std::string vertexPath, std::string fragmentPath);
    RenderProgram()
    {

    }
    void init(std::string vertexPath, std::string fragmentPath);
    static void init(int screenWidth, int screenHeight); //this init function initiates the basic renderprograms

    static glm::vec2 toAbsolute(const glm::vec2& point);//given a screen coordinate, renders it to that point on the screen regardless of zoom
    static glm::vec4 toAbsolute(const glm::vec4& rect);

    static const glm::vec2& getXRange();
    static const glm::vec2& getYRange();
    static const glm::vec2& getZRange();
    static void setXRange(float x1, float x2);
    static void setYRange(float y1, float y2);
    static void setZRange(float z1, float z2);
    static void resetRange();
    static glm::mat4 getOrtho(); //gets projection matrix
    static glm::vec2 getScreenDimen();
    void setMatrix4fv(std::string name, const GLfloat* value); //pass in the value_ptr of the matrix
    void setVec3fv(std::string name,glm::vec3 value);
    void setVec4fv(std::string name, glm::vec4 value);
    void setVec2fv(std::string name, glm::vec2 value);
    void use(const GLfloat* ortho); //pass in the ortho matrix (camera view)
    void use();


};

class RenderCamera
{
protected:
    glm::vec4 rect = {0,0,0,0};

public:
    static RenderCamera* currentCamera; //a pointer to the current camera in use.
    virtual void init(int w, int h); //we use an init instead of constructor since we don't always know what the dimensions are when creating the object.
    const glm::vec4& getRect() const;
    void setRect(const glm::vec4& rect_);
    void addVector(const glm::vec2& moveVector); //add vector to topleft corner
    glm::vec2 getCenter();
    void recenter(const glm::vec2& point);

    glm::vec2 toScreen(const glm::vec2& point) const; //converts a rect from the world coordinate to the screen coordinate
    glm::vec4 toScreen(const glm::vec4& rect) const;

    glm::vec2 toWorld(const glm::vec2& point) const; //converts a rect from the screen coordinate to the world coordinate
    glm::vec4 toWorld(const glm::vec4& rect) const;

    glm::vec2 toAbsolute(const glm::vec2& point) const;//given a screen coordinate, renders it to that point on the screen regardless of zoom
    glm::vec4 toAbsolute(const glm::vec4& rect) const;
};

//extern RenderProgram RenderProgram::basicProgram;
//extern RenderProgram RenderProgram::lineProgram;

enum RenderEffect
{
    NONE,
    MIRROR,
    HORIZMIRROR
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
    bool operator ==(const SpriteParameter& p2)
    {
        return this->rect == p2.rect && this->radians == p2.radians &&
        this->effect == p2.effect && this->tint == p2.tint &&
        this->program == p2.program && this->z == p2.z &&
        this->portion == p2.portion;
    }
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
    constexpr static float verticies[16] = { //verticies of the sprite
    -1, 1, 0, 1,
    1, 1, 1, 1,
    -1, -1, 0, 0,
    1, -1, 1, 0

    };
    constexpr static int indices[6] = { //order in which to render the vertices
    0,1,3,
    0,2,3
    };
public:

    unsigned int VBO= 0, modVBO, //vbo stores the verticies of our image, modVBO stores the transformation information.
                VAO=0;
    static const int floats; //the number of floats we pass everytime we render an instance/spriteParameter
    static const size_t floatSize; //size of floats in bytes;
    void load(std::string source);
    virtual void loadData(GLfloat* data, const SpriteParameter& parameter, int index);
    void draw( RenderProgram& program, GLfloat* data, int instances); //draws the sprite. Assumes ModVBO has already been loaded
    bool transparent = false;
    std::string source = "";
    Sprite(std::string source);
    Sprite()
    {
        texture = -1;
    }
    ~Sprite();
    std::string getSource();
    void init(std::string source);
    void loadVertices();

  //  virtual void render(RenderProgram& program, SpriteParameter parameter);
    virtual void renderInstanced(RenderProgram& program, const std::vector<SpriteParameter>& parameters);
    unsigned int getVAO();
    virtual glm::vec2 getDimen();
    virtual int getFloats(); //# of floats per SpriteParameter is different for each class, so this function just returns the version for each child of Sprite
    void reset(); //clears all buffers and resets modified back to values
};

class Sprite9 : public Sprite // This sprite has been split into 9 sections that each scale differently. The corners aren't scaled at all, the top and bottom
{                               //are only scaled horizontally, the sides are only scaled vertically, and the center can be scaled any which way.
    static const int floats9; //the number of floats per SpriteParameter for Sprite9. Each spriteParameter for Sprite9 is rendered 9 different times so this is 9*Sprite::floats
    glm::vec2 widths; //the widths of the frame on either side;
    glm::vec2 heights; //heights of the frame on either side;
    void loadData(GLfloat* data, const SpriteParameter& parameter, int index);
public:
    Sprite9(std::string source, glm::vec2 W, glm::vec2 H);
    Sprite9()
    {

    }
    int getFloats();
    void init(std::string source, glm::vec2 W, glm::vec2 H);

};

struct AnimationParameter//the main difference between this class and SpriteParameters is that this one provides the time at which the animation started and the fps
{
    int start = -1; //the frame we start at, with the first frame being 0. -1 to use SDL_Ticks to calculate the start
    float fps = -1; //the fps for the animation. -1 means use the default
    unsigned int repeat = 0; //repeats the animation "repeat" times. 0 instantly ends the animation after one frame.
    glm::vec4 subSection = glm::vec4(0); //the portion of the sprite sheet we want to render
    glm::vec4 (RenderCamera::*transform) (const glm::vec4&) const = nullptr; // a transform function for the render location
    RenderCamera* camera = nullptr; //the camera to use with the transform
};

typedef std::pair<SpriteParameter,AnimationParameter> FullAnimationParameter;

class BaseAnimation : public Sprite //the actual animation object
{
    int fps = 0;
    glm::vec2 frameDimen; //proportion of the spritesheet of each frame
    glm::vec4 subSection = {0,0,0,0}; //subsection.xy is the origin of the sprite sheet. This is a standardized value (0-1). subsection.za is the framesPerRow and the number of rows wanted.
public:
    static getFrameIndex(int startingFrame, int timePerFrame) //given the frame an animation has started at and the time to spend per frame, return which frame should be rendered
    {
        if (timePerFrame == 0)
        {
            return 0;
        }
        return (DeltaTime::getCurrentFrame() - startingFrame)/timePerFrame;
    }
    BaseAnimation(std::string source, int speed, int perRow, int rows, const glm::vec4& sub = {0,0,0,0});
    BaseAnimation()
    {

    }
    int getFPS();
    int getFrames();
    glm::vec2 getDimen(); //does not return the dimensions of the whole spritesheet but rather the size of the portion to be rendered.
    int getDuration(int speed = -1); //returns the duration of the full animation in milliseconds
    glm::vec4 getPortion(const AnimationParameter& param); //given animation parameter, returns the portion of the spreadsheet
    SpriteParameter processParam(const SpriteParameter& sParam,const AnimationParameter& aParam); //returns a spriteparameter that represents what to render
    void init(std::string source,int speed, int perRow, int rows, const glm::vec4& sub = {0,0,0,0}); //how many frames per row and how many rows there are
    using Sprite::renderInstanced;
    void renderInstanced(RenderProgram& program, const std::list<FullAnimationParameter>& parameters);
    void renderInstanced(RenderProgram& program, const std::vector<SpriteParameter>& parameters);
};

class SpriteWrapper
{

protected:
    Sprite* spr = nullptr;
public:
    virtual void init(std::string source);
    virtual void init(Sprite* spr);
    virtual void reset();
    virtual void render(const std::list<SpriteParameter>& parameters, float zMod =0, RenderCamera* camera = nullptr); //zMod is a final modifier to the z. Used by spriteManager to decrease the chances of sprites overlapping
    Sprite* getSprite();
    glm::vec2 getDimen();
    bool isReady(); //returns whether or not spr is null
    virtual void request(const SpriteParameter& param);
    virtual ~SpriteWrapper();

};


class AnimationWrapper : public SpriteWrapper //this class actually doesn't call BaseAnimation::renderInstanced, but rather converts FullAnimationParameters into SpriteParameters
{
    std::list<FullAnimationParameter> aParameters; //we use a linkedList for this because we often times only want to remove some Animation Parameters. We could use forward_list for slight efficiency but I'm too lazy to deal with erase_after :)
public:
    BaseAnimation* getAnimation();
    void init(BaseAnimation* a);
    void reset();
    using SpriteWrapper::request;
    void request(const SpriteParameter& param); //calls requests a default AnimationParameter
    void request(const SpriteParameter& sparam,const AnimationParameter& aparam);
    ~AnimationWrapper();
};

typedef std::pair<float,SpriteWrapper*> zWrapper; //used to sort spriteWrappers

class SpriteManager
{
    struct ZWrapperComparator
    {
        bool operator ()(const zWrapper& a, const zWrapper& b) const
        {
            if (a.first == b.first)
            {
                return a.second < b.second;
            }
            return a.first < b.first;
        }
    };
    static std::map<zWrapper,std::list<SpriteParameter>,ZWrapperComparator> params;
    static std::unordered_map<std::string,SpriteWrapper*> sprites;
public:
    constexpr static float zIncrement = .001; //slight increment so sprites don't overlap

    static void addSprite(SpriteWrapper& spr);
    static void request(SpriteWrapper& wrapper,const SpriteParameter& param);
    static SpriteWrapper* getSprite(std::string source); //will attempt to return a spritewrapper with the source loaded, null otherwie
    static void render(RenderCamera* camera = nullptr);

};

template<typename T>
using PolyStorage = std::vector<T>;
struct PolyRender
{
    static std::vector<std::pair<glm::vec3,glm::vec4>> lines; //lines and their colors
    static PolyStorage<glm::vec4> polyColors; //color of each polygon. Color is repeated once for each edge of the polygon
    static PolyStorage<glm::vec3> polyPoints; //points of polygons
    static PolyStorage<GLuint> polyIndices;
    static int polygonRequests; //number of requests for a polygon
    static RenderProgram polyRenderer;
    static unsigned int VAO;
    static unsigned int lineVBO;
    static unsigned int polyVBO;
    static unsigned int colorVBO;
    static void init(int screenWidth, int screenHeight);
    static void requestLine(const glm::vec4& line, const glm::vec4& color, float z = 0, unsigned int thickness = 1, RenderCamera* camera = 0);
    static void requestGradientLine(const glm::vec4& line, const glm::vec4& color1, const glm::vec4& color2, float z = 0, unsigned int thickness = 1, RenderCamera* camera = 0);
    static void requestCircleSegment(float segHeight,float angle, const glm::vec4& color,const glm::vec2& center, double radius, bool filled, float z); //draw a CircleSegment. Angle = 0 means that only the top of the circle will be drawn.
    static void requestCircle(const glm::vec4& color,const glm::vec2& center, double radius, bool filled, float z);
    static void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z);
    static void requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool radius = false); //draws a regular n gon. Angle is in radians. If radius is true, then side is the radius length rather than the side length
    static void requestPolygon(const std::vector<glm::vec3>& points, const glm::vec4& color);
    static void render();
    static void renderMesh(float* mesh, int w, int h);
    static void renderLines(); //renders lines. Can be called from other functions to render all lines currently requested
    static void renderPolygons();
private:
    static int getIndiciesNumber() //number of indicies minus restarts
    {
        return polyIndices.size() - polygonRequests;
    }
    static void addPointAndIndex(const glm::vec3& point) //adds a point to polyPoints as well as the index based on how many indicies have already been added
    {
        polyPoints.push_back(point);
        polyIndices.push_back(getIndiciesNumber());
    }
    static unsigned short restart; //restart indice
};



#endif // RENDER_H_INCLUDED
