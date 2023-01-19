#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include <iostream>

#include <vector>
#include <map>
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
typedef unsigned int Buffer;

struct ViewPort //has data about visible area on screen
{
    static int screenWidth, screenHeight;
    static ViewRange baseRange;  //represents the smallest and largest values x,y,z can be. X and Y should always have 0 as the smallest value.
    static ViewRange currentRange; //represents the current range for x,y, and z
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
};

class Sprite;
class RenderProgram //represents a shader pipeline (by default only a vertex and a fragment shader).
{
private:
    int dataAmount = 0; //total number of GLfloats passed to shader
    unsigned int program;
    void initShaders(std::string vertexPath, std::string fragmentPath);
    void initBuffers(); //initializes VAO and VBOs as well as loads the verticies into VerticiesVBO

protected:
    typedef std::initializer_list<int> Numbers; //represents list of numbers where each number is how many GLfloats belong to a vertex attribute

    void initTransforms(int total, Numbers numbers);//initTransforms allows us to specify our vertex attribute data using one function.
                                                    //"total" is the total amount of numbers(usually floats) we pass
                                                    //"numbers" is a list of ints to specify how many ints per attribute
public:
    int preDataAmount = 0; //total number of GLFloats per request; so called because they have yet to be processed (pre-processed).

    Buffer VBO, //used to store transform data
    VAO,
    verticiesVBO; //used to store verticies

    RenderProgram(std::string vertexPath, std::string fragmentPath);
    RenderProgram()
    {

    }
    void init(std::string vertexPath, std::string fragmentPath,int total, Numbers numbers);
    void init(std::string vertexPath, std::string fragmentPath,int a);
    void init(std::string vertexPath, std::string fragmentPath);

    void setMatrix4fv(std::string name, const GLfloat* value); //pass in the value_ptr of the matrix
    void setVec3fv(std::string name,glm::vec3 value);
    void setVec4fv(std::string name, glm::vec4 value);
    void setVec2fv(std::string name, glm::vec2 value);
    void use(const GLfloat* ortho); //pass in the ortho matrix (camera view)
    void use();
    void draw(Sprite& sprite, void* data, int instances);

    int getRequestDataAmount();




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
     glm::vec4 rect = {0,0,1,1};
    float radians = 0;
    float z = 0;
    RenderEffect effect = NONE; //effects to do. (mirror, flip, etc)
};

bool isTransluscent(unsigned char* sprite, int width, int height); //returns true if sprite has any pixels with an alpha value between 0 and 1, non-inclusive

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

    Buffer VBO= 0, modVBO, //vbo stores the verticies of our image, modVBO stores the transformation information.
                VAO=0;
    static const int floats; //the number of floats we pass everytime we render an instance/spriteParameter
    void load(std::string source);
    virtual void loadData(GLfloat* data, const SpriteParameter& parameter, int index); //loads information from SpriteParameter into "data", starting at "index"
    template<typename Iterator>
    void loadData(GLfloat* data, const Iterator& a, const Iterator& b, int index); //load a bunch of data from an STL container of SpriteParameters, starting at "a" and going up to "b".
                                                                                    //"Iterator" is an object that has the ++ and * operators (usually iterators)
    void draw( RenderProgram& program, float* data, int instances); //draws the sprite. Assumes ModVBO has already been loaded
    bool transluscent = false;
    std::string source = "";
    Sprite(std::string source);
    Sprite()
    {
        texture = -1;
    }
    ~Sprite();
    unsigned int getTexture();
    std::string getSource();
    void init(std::string source);
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
    int timeSince = 0; //milliseconds since beginning,
    int fps = 1;
    glm::vec4 subSection = glm::vec4(0); //the portion of the sprite sheet we want to render
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
};
typedef std::pair<Sprite*,SpriteParameter> SpriteRequest;

struct OpaqueSpriteRequest
{
    Sprite& sprite;
    RenderProgram& program;
    int index;


};

class SpriteManager
{
    struct SpriteRequestComparator
    {
        bool operator()(const SpriteRequest& a, const SpriteRequest& b) const //returns true is a < b, which means "a" gets rendered before "b"
        {
            if (a.first->transluscent == b.first->transluscent) //if they are both equally transluscent, compare by z
            {
                if (!a.first->transluscent) //if a and b are both opaque
                {
                    return a.first < b.first; //sort by image address, ensures that the same images will be rendered together
                }
                else //otherwise we have to sort by furthest transluscent fragments to nearest
                {
                    if (a.second.z == b.second.z)
                    {
                        return a.first < b.first; //if we can compare by z, compare by sprite address to ensure that same sprites are always bundeled together.
                    }
                    return a.second.z < b.second.z;
                }

            }
            return !a.first->transluscent; //opaque sprites are rendered first, so if a is transcluscent but b isn't, then b goes a is greater than b (b is rendered first).
        }
    };
    struct OpaqueCompare
    {
        bool operator()(const OpaqueSpriteRequest& a, const OpaqueSpriteRequest& b) const //returns true is a < b, which means "a" gets rendered before "b"
        {
            return &a.sprite < &b.sprite; //just need to make sure that the same sprites are grouped together
        }
    };
    static std::multiset<SpriteRequest,SpriteRequestComparator> params;
    static std::multiset<OpaqueSpriteRequest,OpaqueCompare> opaques;
    static std::vector<char> data; //used to store transformations for all the sprite parameters
    static std::vector<char> opaqueData; //used to store all data for opaques
    static std::vector<float>floatsData;
    static void requestWork(Sprite& sprite, RenderProgram& program,size_t bytes)
    {
        if (bytes < program.getRequestDataAmount())
        {
            opaqueData.resize( opaqueData.size() + program.getRequestDataAmount() - bytes,0);
        }
        for (int i = 0; i < program.getRequestDataAmount(); i += sizeof(float))
        {
            float num = 0;
            memcpy(&num,&opaqueData[opaqueData.size() - program.getRequestDataAmount() + i],sizeof(float));
        }
        opaques.insert({sprite,program, opaqueData.size()- program.getRequestDataAmount()});
    }
    template<typename T, typename... Args>
    static void requestWork(Sprite& sprite, RenderProgram& program,size_t bytes,T t1, Args... args)
    {
        //std::cout << opaqueData.size()/28 << "\n";
        if (bytes >=program.getRequestDataAmount())
        {
            requestWork(sprite,program,bytes); //terminate early if too many arguments were provided
        }
        else
        {
            char* bytesBuffer = reinterpret_cast<char*>(&t1);
            opaqueData.insert(opaqueData.end(), bytesBuffer,bytesBuffer + sizeof(t1));
            //std::cout << opaqueData.size()<< " " << sizeof(t1) << "\n";
            requestWork(sprite,program,bytes + sizeof(t1), args...);
        }
    }
public:
    constexpr static float zIncrement = .001; //slight increment so sprites don't overlap
    static void init();
    static void request(Sprite& wrapper,const SpriteParameter& param);

    template<typename T, typename... Args>
    static void request(Sprite& sprite, RenderProgram& program,T t1, Args... args)
    {
        requestWork(sprite,program,0,t1,args...);
    }

    static void render(RenderProgram& program, RenderCamera* camera = nullptr);

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
