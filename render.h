#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include <iostream>

#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <set>

#include "glew.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "SDLhelper.h";
#include "vanilla.h"

void addPointToBuffer(float buffer[], glm::vec3 point, int index);
void addPointToBuffer(float buffer[], glm::vec2 point, int index);
void addPointToBuffer(float buffer[], glm::vec4 point, int index);

class GLContext
{
    /*
    represents OpenGL context. Keeps track if the context is still valid or not. Many of our OpenGL wrapperclasses
    will not work if GLContext::init is not called in the beginning of every program. Similarly, terminate() should be called
    right before main() ends.*/
    static bool context; //true if context is guaranteed to be true;
    static SDL_Window* window;
public:
    static void init(int screenWidth, int screenHeight);
    static bool isContextValid();
    static void update();
    static void terminate();
    ~GLContext()
    {
        terminate();
    }
};
struct ViewRange //represents the extent of our view range. y is the furthest, x is the smallest
{
    static float getXRange(const ViewRange& range)
    {
        return (range.xRange[1] - range.xRange[0]);
    }
    static float getYRange(const ViewRange& range)
    {
        return (range.yRange[1] - range.yRange[0]);
    }
    static float getZRange(const ViewRange& range)
    {
        return (range.zRange[1] - range.zRange[0]);
    }
    glm::vec2 xRange = {0,0};
    glm::vec2 yRange = {0,0};
    glm::vec2 zRange = {0,0};
};
typedef unsigned int Buffer;

typedef std::initializer_list<int> Numbers; //represents list of numbers where each number is how many GLfloats belong to a vertex attribute

struct BasicRenderPipeline //extremely simple class, made for simple rendering
{
    unsigned int program = 0;
    size_t dataAmount; //number of bytes per request
    Buffer VBO;
    Buffer VAO;
    std::vector<char*> bytes;
    void init(std::string vertexPath, std::string fragmentPath, Numbers numbers = {}); //future Leo! If you ever decide to add more shaders to a pipeline, consider
                                                                                        //adding their paths to the end of this function with a default value of ""
private:
    void initAttribDivisors(Numbers numbers);
};



class Sprite;
class RenderProgram //represents a Sprite shader pipeline (by default only a vertex and a fragment shader).
{
private:
    int dataAmount = 0; //total number of GLfloats passed to shader
    void initShaders(std::string vertexPath, std::string fragmentPath,Numbers numbers);
    virtual void initBuffers(); //loads the verticies into VerticiesVBO

public:
    BasicRenderPipeline program;
    Buffer verticiesVBO; //used to store texture verticies

    RenderProgram(std::string vertexPath, std::string fragmentPath,Numbers numbers= {});
    RenderProgram()
    {

    }
    ~RenderProgram()
    {
        /*if (GLContext::isContextValid())
        glDeleteVertexArrays(1,&VAO);
        if (GLContext::isContextValid())
        glDeleteBuffers(1,&verticiesVBO);
        if (GLContext::isContextValid())
        glDeleteBuffers(1,&VBO);*/
    }
    void init(std::string vertexPath, std::string fragmentPath,Numbers numbers);
    void init(std::string vertexPath, std::string fragmentPath);

    void setMatrix4fv(std::string name, const GLfloat* value); //pass in the value_ptr of the matrix
    void setVec3fv(std::string name,glm::vec3 value);
    void setVec4fv(std::string name, glm::vec4 value);
    void setVec2fv(std::string name, glm::vec2 value);
    void use();
    virtual void draw(Sprite& sprite, void* data, int instances);

    int getRequestDataAmount(); //bytes of data needed for this render program
    unsigned int ID();




};

class RenderCamera;
struct ViewPort //has data about visible area on screen
{
    static Buffer UBO; //view and projection matricies Uniform Buffer
    static int screenWidth, screenHeight;
    static ViewRange baseRange;  //represents the smallest and largest values x,y,z can be. X and Y should always have 0 as the smallest value.
    static ViewRange currentRange; //represents the current range for x,y, and z
    static RenderProgram basicProgram; //generic shader pipeline to render sprites
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
    static void linkUniformBuffer(unsigned int program); //set a program to use UBO
    static void update(RenderCamera* camera = nullptr);
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

bool isTransluscent(unsigned char* sprite, int width, int height); //returns true if sprite has any pixels with an alpha value between 0 and 1, non-inclusive

class Sprite
{
protected:
    int width = 0, height = 0;
    unsigned int texture = 0;
    bool transluscent = false;
public:
    void load(std::string source);
    std::string source = "";
    Sprite(std::string source);
    Sprite()
    {
        texture = 0;
    }
    ~Sprite();
    unsigned int getTexture();
    std::string getSource();
    bool getTransluscent();
    void init(std::string source);
    virtual glm::vec2 getDimen();
};

class Sprite9 : public Sprite // This sprite has been split into 9 sections that each scale differently. The corners aren't scaled at all, the top and bottom
{                               //are only scaled horizontally, the sides are only scaled vertically, and the center can be scaled any which way.
    glm::vec2 widths; //the widths of the frame on either side;
    glm::vec2 heights; //heights of the frame on either side;
    //void loadData(GLfloat* data, const SpriteParameter& parameter, int index);
public:
    Sprite9(std::string source, glm::vec2 W, glm::vec2 H);
    Sprite9()
    {

    }
    int getFloats();
    void init(std::string source, glm::vec2 W, glm::vec2 H);

};

/*struct AnimationParameter//the main difference between this class and SpriteParameters is that this one provides the time at which the animation started and the fps
{
    int timeSince = 0; //milliseconds since beginning,
    int fps = 1;
    glm::vec4 subSection = glm::vec4(0); //the portion of the sprite sheet we want to render
};


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
};*/

class OpaqueManager //manages storing opaque (non-transluscent) fragment render requests
{
    //opaques can be rendered in any order, since they can't be blended, so OpaqueManager simply allocates contiguous memory for
    //each Sprite-RenderProgram pair and renders all the data at once
    struct OpaquePair //hash a pair of Sprite and RenderPrograms by XOring their hashes
    {
        size_t operator()(const std::pair<Sprite&,RenderProgram&>& a) const //we don't have to worry about commutativity because they have different memory addresses
        {
            return std::hash<Sprite*>()(&a.first) ^ std::hash<RenderProgram*>()(&a.second);
        }
    };
    struct OpaqueEquals //how to determine if two pairs are the same (can't believe I had to specify this tbh)
    {
        bool operator()(const std::pair<Sprite&,RenderProgram&>& a,const std::pair<Sprite&,RenderProgram&>& b) const
        {
            return &a.first == &b.first && &a.second == &b.second;
        }
    };
public:
    std::unordered_map<std::pair<Sprite&,RenderProgram&>,std::vector<char>,OpaquePair,OpaqueEquals> opaquesMap;
    //for each sprite-renderprogram pairing, they have a unique bytes buffer
    void render(); //pass each bytes buffer to rendering pipeline
};

typedef int ZType; //used to represent z values
struct TransManager //handles transluscent fragment render requests.
{
     //Transluscents must be rendered after opaques and sorted by distance from the screen from furthest
     //(smallest z) to closest (largest z) to prevent fragments from being discarded via the depth test
    struct TransRequest //bare bones info for each request: what sprite is being rendered, how to render it, the z value (for sorting) and the index in "data" where the transformations are stored
    {
        Sprite& sprite;
        RenderProgram& program;
        ZType z;
        int index;
    };
    void request(Sprite& sprite, RenderProgram& program, ZType z);
    void render();
    std::vector<char> data; //buffer used to store all vertex attributes. Unsorted.
private:
    struct TransRequestCompare //returns true if "a" is "less than" b, and thus should be rendered first
    {
        bool operator()(const TransRequest& a, const TransRequest& b) const //sort by z,then by program, then by sprite
        {
            if (a.z == b.z)
            {
                if (&a.program == &b.program)
                {
                    return &a.sprite < &b.sprite;
                }
                return &a.program < &b.program;
            }
            return a.z < b.z; //fragments with smaller zs get rendered first
        }
    };
    std::vector<char> buffer; //reusable buffer for sorting data. Used to compile all data one sprite-renderprogram pairing.
    std::multiset<TransRequest,TransRequestCompare> requests;
};

struct FullPosition //every request, regardless of sprite or render program is going to have a position (rect.xy), dimensions (rect.za), and a z coordinate (z)
{
    glm::vec4 rect;
    ZType z;
};

class SpriteManager //handles all sprite requests
{
    static OpaqueManager opaques;
    static TransManager trans;

public:
    template<typename... Args>
    static void request(Sprite& sprite, RenderProgram& program,const FullPosition& pos,Args... args)
    {
        /*exposed public function, used to make requests. Every request is expected to begin with rect and z. */
        if (sprite.getTransluscent())
        {
            trans.request(sprite,program,pos.z); //transluscent manager needs to make a request specifically for the sprite-program pairing
            fillBytesVec(trans.data,program.getRequestDataAmount(),pos.rect,pos.z,args...); //place request into transluscent manager
        }
        else
        {
            fillBytesVec(opaques.opaquesMap[{sprite,program}],program.getRequestDataAmount(),pos.rect,pos.z,args...); //place request into opaque manager
        }
    }
    static void render();

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
    static void requestLine(const glm::vec4& line, const glm::vec4& color, float z = 0, unsigned int thickness = 1);
    static void requestGradientLine(const glm::vec4& line, const glm::vec4& color1, const glm::vec4& color2, float z = 0, unsigned int thickness = 1);
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
