#ifndef RENDER_H_INCLUDED
#define RENDER_H_INCLUDED

#include <iostream>

#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <forward_list>
#include <set>

#include "glew.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "SDLhelper.h"
#include "vanilla.h"

void addPointToBuffer(float buffer[], glm::vec3 point, int index);
void addPointToBuffer(float buffer[], glm::vec2 point, int index);
void addPointToBuffer(float buffer[], glm::vec4 point, int index);


typedef int ZType; //used to represent z values
typedef std::vector<int> Numbers; //represents list of numbers where each number is how many GLfloats belong to a vertex attribute

struct LoadShaderInfo
{
    //holds info about a type of shader and how to load it
    std::string code = ""; //either the filepath to the shader or the raw code of the shader
    GLenum shaderType = GL_VERTEX_SHADER; //type of shader.
    bool isFilePath = true; //true if "code" is a filepath, false if "code" is the code of the shader
};


Numbers getVertexInputs(const std::string& vertexContents); //given a vertexShader contents, returns its inputs

//loads shader given file path. If "shaderType" is vertexshader, it will also load the inputs into "numbers". Returns the shader handle or -1 on failure
//the reason why I went with the design of loading the vertex inputs into a vector provided as part of the parameter is because I wanted to be able to open a
//vertex file path only once, and then compile the sahders and load the inputs all at once. Feel free to change this future Leo!
int loadShaders(const GLchar* source, GLenum shaderType, Numbers* numbers = 0);
int loadShaders(LoadShaderInfo&& info, Numbers* numbers = 0);

//using a shader as a template, adds some inputs and outputs. ideal for shaders that do pretty much the same thing but may need to pass an additional output to another shader
std::string templateShader(const std::string& shaderContents, bool isVertex, std::initializer_list<std::string> inputs, std::initializer_list<std::string> outputs, std::initializer_list<std::string> tasks);

 //removes comments from a shader. Critical for loadShader and templateShader
std::string stripComments(const std::string& shaderContents);

class GLContext
{
    /*
    represents OpenGL context. Keeps track if the context is still valid or not. Many of our OpenGL wrapperclasses
    will not work if GLContext::init is not called in the beginning of every program. Similarly, terminate() should be called
    right before main() ends.*/
    static bool context; //true if context is guaranteed to be true;
    static SDL_Window* window;
public:
    static void init(int screenWidth, int screenHeight, bool fullscreen = false);
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
typedef GLuint Buffer;

const float textureVerticies[24] = { //verticies of textures
    -1, 1, 0, 1, //top left
    1, 1, 1, 1, //top right
    1, -1, 1, 0, //bottom right
    -1, 1, 0, 1, //top left
    -1, -1, 0, 0, //bottom left
    1, -1, 1, 0 //bottom right

    };



struct VBOInfo
{
    //stores a VBO and how much data it expects to have per vertex
    size_t floatsPerVertex = 0; //maximum amount of floats per vertex;
    Buffer VBO;
};


//for each type of divisor, we get the corresponding data and VBO
typedef std::unordered_map<int,VBOInfo> VBOs;
typedef std::vector<char> Bytes;
typedef std::vector<int> DivisorStorage;
typedef std::unordered_map<int,Bytes> RenderPayload; //maps divisors to their total data

struct BasicRenderPipeline //made for storing simple rendering information
{
    static constexpr float basicScreenCoords[12] = { //verticies to render a rectangle the size of the screen
        -1,-1, //bot left
        1,-1, //bop right
        -1,1, //top left
        1,-1, //bot right
        -1,1, //top left
        1,1 //top right
    };

    const int vertexAmount = 0; //number of verticies

    DivisorStorage divisors; //the divisors for each attribute. the index of the vertex corresponds to the attribute index. If not specified, divisor defaults to 1 with the exception of the verticies
    Numbers numbers; //the number of floats per vertex attribute

    //constructor that takes in an infinite list of shaders, as many as you want. Implemented below ViewPort
    template<size_t N>
    BasicRenderPipeline(LoadShaderInfo (&&info)[N], const DivisorStorage& divisors_ = {0}, const float* verts = basicScreenCoords,
                                              int floatsPerVertex_ = 2, int vertexAmount_ = 6);

    //construct renderpipeline from shader paths.
    explicit BasicRenderPipeline(std::string vertexPath, std::string fragmentPath,  //vertex and fragment shader paths
                const DivisorStorage& divisors_ = {0}, const float* verts = basicScreenCoords, int floatsPerVertex_ = 2, int vertexAmount_ = 6); //info for verticies, by default render a rectangle size of the screen

    //draw the stuff. Recommended more for testing purposes; if you want to draw something you should prioritize using SpriteManager
    template <typename T,typename... Args>
    void draw(GLenum mode, T t1, Args... args);


    void bufferPayload(RenderPayload& payload); //buffers the payload's data in the VBOs in preparation for rendering
    int getDivisor(unsigned int index); //get the attrib divisor of the "index-th" attribute

    //set uniforms
    void setMatrix4fv(std::string name, const GLfloat* value); //pass in the value_ptr of the matrix
    void setVec3fv(std::string name,glm::vec3 value);
    void setVec4fv(std::string name, glm::vec4 value);
    void setVec2fv(std::string name, glm::vec2 value);

    size_t getBytesPerRequest();
    Buffer getProgram();
    Buffer getVBO(int divisor);
    Buffer getVAO();
    Buffer getVerticies();

    template <typename T,typename... Args>
    RenderPayload packData(T t1, Args... args); //packs arguments into a RenderPayload based on divisors
    void packData(RenderPayload& payload, char* bytes ); //packs a series of bytes that is assumed to be the attributes into payload
    void initVerticies(const float* verts, int floatsPerVertex_, int vertexAmount); //initiates argument 0, which is assumed to be verticies.
private:
    size_t dataAmount = 1; //number of bytes per request
    Buffer program = 0;
    VBOs vbos;
    Buffer VAO;
    Buffer verticies; //VBO for verticies
    void initAttribDivisors(Numbers numbers); //initiates inputs, assuming first input is verticies and already set by "initVerticies"
    void packDataHelper(RenderPayload& payload, int divisorsIndex, int vertexIndex);
    template <typename T,typename... Args>
    void packDataHelper(RenderPayload& payload, int divisorsIndex, int vertexIndex, T t1, Args... args);
};

using RenderProgram = BasicRenderPipeline;

class RenderCamera;

struct __attribute__((packed, aligned(1))) UBOContents
{
    //tightly packed structure that represents the contents of our UBO
    //tightly packed so we can pass the struct as is into our rendering pipeline
    glm::mat4 perspectiveMatrix;
    glm::mat4 viewMatrix;
    glm::vec2 screenDimen;
    float cameraZ;

    //TODO: Uniform buffers have wack ass layouts. This current structure is set up to avoid any issues but adding more entries may cause them. Look up std140 layout

};

struct ViewPort //has data about visible area on screen. Make sure you initialize before you do anything involving rendering
{
    enum PROJECTION_TYPE
    {
        ORTHOGRAPHIC = 0, //2d, no depth
        PERSPECTIVE = 1 //3d (but can be used for 2d)
    };
    static RenderCamera* currentCamera; //the current camera in use
    static Buffer UBO; //Uniform Buffer

    static int screenWidth, screenHeight;
    static PROJECTION_TYPE proj;
    static ViewRange baseRange;  //represents the smallest and largest values x,y,z can be. X and Y should always have 0 as the smallest value.
    static ViewRange currentRange; //represents the current range for x,y, and z
    static std::unique_ptr<BasicRenderPipeline> basicProgram; //generic shader pipeline to render sprites
    static std::unique_ptr<BasicRenderPipeline> animeProgram; //shader pipeline to render spritesheets

    static void init(int screenWidth, int screenHeight); //this init function initiates the basic renderprograms

    static glm::vec2 toAbsolute(const glm::vec2& point);//given a screen coordinate, renders it to that point on the screen regardless of zoom
    static glm::vec4 toAbsolute(const glm::vec4& rect);

    static glm::vec2 toWorld(const glm::vec2& point, ZType z = 0);//if currentCamera is not null, uses currentCamera to project screen point to world point. Otherwise, return "point"
    static glm::vec4 toWorld(const glm::vec4& point, ZType z = 0);

    static glm::vec2 toScreen(const glm::vec2& point); //same as toWorld but for toScreen
    static glm::vec4 toScreen(const glm::vec4& point);

    static const glm::vec2& getXRange();
    static const glm::vec2& getYRange();
    static const glm::vec2& getZRange();

    static const float getViewWidth();
    static const float getViewHeight();
    static const float getViewDepth();

    static void setXRange(float x1, float x2);
    static void setYRange(float y1, float y2);
    static void setZRange(float z1, float z2);
    static void resetViewRange();
    static glm::mat4 getProjMatrix(); //gets projection matrix
    static glm::mat4 getViewMatrix();
    static PROJECTION_TYPE getProj();
    static constexpr float FOV = 45;
    static void flipProj();
    static glm::vec2 getScreenDimen();
    static void linkUniformBuffer(unsigned int program); //set a program to use UBO
    static void update();

private:
    static UBOContents uniforms; //the uniforms passed to the UBO. Most values are initialized and change very little, but view matrix updates every frame

    static void setViewRange(const ViewRange& range);
    static void resetUniforms(); //reset all uniforms
};


//warning: if you call the constructor like this:
//BasicRenderPipeline stars({{"./shaders/gravityVertexShader.h",GL_VERTEX_SHADER,true},{"./shaders/starShader.h",GL_FRAGMENT_SHADER,true}});
//it'll default to the constructor below, which is obviously not correct. The "explicit" helps prevent the program from compiling. Instead, call like:
//BasicRenderPipeline stars({LoadShaderInfo{"./shaders/gravityVertexShader.h",GL_VERTEX_SHADER,true},{"./shaders/starShader.h",GL_FRAGMENT_SHADER,true}});
//if your array doesn't have two elements (either less or more) you do not have to worry about this.
template<size_t N>
BasicRenderPipeline::BasicRenderPipeline(LoadShaderInfo (&&info)[N],const DivisorStorage& divisors_,const float* verts, int floatsPerVertex_, int vertexAmount_) : vertexAmount(vertexAmount_), divisors(divisors_)
{

    program = glCreateProgram();
    ViewPort::linkUniformBuffer(program);


    for (auto&& shaderInfo : info)
    {
        GLuint shader = loadShaders(std::move(shaderInfo),&numbers);
        if (shader != -1)
        {
            glAttachShader(program,shader);
            glDeleteShader(shader);
        }
    }
    glLinkProgram(program);

    glGenVertexArrays(1,&VAO);

    initVerticies(verts,floatsPerVertex_,vertexAmount_);
    initAttribDivisors(numbers);

}


template <typename T,typename... Args>
void BasicRenderPipeline::draw(GLenum mode, T t1, Args... args) //pass in a bunch of data and then draw
{
    //maybe consider making this a separate function that takes in a BasicRenderPipeline and draws rather than calling it from the Pipeline itself
    RenderPayload payload = packData(t1,args...);
    bufferPayload(payload);

    glUseProgram(program);
    glDrawArrays(mode,0,vertexAmount);
}

template <typename T,typename... Args>
void BasicRenderPipeline::packDataHelper(RenderPayload& payload, int divisorsIndex, int vertexIndex, T t1, Args... args)
{
    std::vector<char>* vecPtr = &payload[getDivisor(divisorsIndex)];

    char* bytesBuffer = reinterpret_cast<char*>(&t1); //convert to string of bytes
    vecPtr->insert(vecPtr->end(),bytesBuffer, bytesBuffer + sizeof(t1));

    //if we have put in data for each vertex, move onto the next divisors index, which should correspond with the next attribute
    divisorsIndex += (vertexIndex >= vertexAmount - 1);

    //if the divisor is 0, we move to the next vertex, which could potentially be the 0th vertex if "divisorIndex" just hit 0.
    //otherwise, we stay at "vertexAmount" - 1, which means we push the next argument assuming that it has a divisor of 1.
    vertexIndex = (divisors[divisorsIndex] == 0 ? (vertexIndex + 1)%vertexAmount : vertexAmount - 1);
    packDataHelper(payload,divisorsIndex,vertexIndex, args...);
}


template <typename T,typename... Args>
RenderPayload BasicRenderPipeline::packData(T t1, Args... args)
{
    RenderPayload payload;
    packDataHelper(payload,1,(divisors[1] == 0 ? 0 : vertexAmount - 1),t1,args...); //start at "divisorsIndex" 1 because verticies are processed separately
    return payload;
}

class RenderCamera
{
protected:
    glm::vec3 pos;

public:
    virtual void init(const glm::vec3& pos_); //we use an init instead of constructor since we don't always know what the dimensions are when creating the object.
    ~RenderCamera(); //make sure to update the current camera if this camera was the current camera
    const glm::vec3& getPos() const;
    glm::vec4 getRect() const;
    glm::vec2 getTopLeft() const; //coordinate of the top left corner of the screen
    void setPos(const glm::vec3& pos_);
    void setPos(const glm::vec2& pos_);
    void addVector(const glm::vec2& moveVector); //add vector to topleft corner
    void addVector(const glm::vec3& moveVector);

    glm::vec2 toScreen(const glm::vec2& point) const; //converts a rect from the world coordinate to the screen coordinate
    glm::vec4 toScreen(const glm::vec4& rect) const;

    glm::vec2 toWorld(const glm::vec2& point, ZType z = 0) const; //converts a rect from the screen coordinate to the world coordinate. if using perspective mode, assumes the point is at the given z
    glm::vec4 toWorld(const glm::vec4& rect, ZType z = 0) const; //
    glm::vec2 toAbsolute(const glm::vec2& point) const;//given a screen coordinate, renders it to that point on the screen regardless of zoom
    glm::vec4 toAbsolute(const glm::vec4& rect) const;
};

//extern BasicRenderPipeline BasicRenderPipeline::basicProgram;
//extern BasicRenderPipeline BasicRenderPipeline::lineProgram;

enum RenderEffect
{
    NONE,
    MIRROR,
    HORIZMIRROR
};

bool isTransluscent(unsigned char* sprite, int width, int height); //returns true if sprite has any pixels with an alpha value that is not 1

class Sprite
{
protected:
    int width = 0, height = 0;
    unsigned int texture = 0;
    bool transluscent = false; //returns true if sprite has any pixels with an alpha value that is not 1
public:
    void load(std::string source);
    std::string source = "";
    Sprite(std::string source);
    Sprite()
    {
        texture = 0;
    }
    ~Sprite();
    unsigned int getTexture() const;
    std::string getSource() const;
    bool getTransluscent() const;
    void init(std::string source);
    virtual glm::vec2 getDimen() const;
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
};*/

typedef Uint32 UINT;

struct BaseAnimation //represents data used to animate a portion of a sprite sheet.
{
    glm::vec4 subSection = {0,0,1,1}; //The subsection of the spritesheet we want. All values should be between 0 - 1
    UINT perRow = 1;//frames per row
    UINT rows = 1;
    UINT fps = 1;

    UINT getTotalFrames(); //number of frames in the animation

    static glm::vec4 getNthFrame(UINT n, BaseAnimation& anime); //return the nth frame, 0 being the first frame. if "n" is bigger than the totalFrames, then it loops back around (giving index 4 for a 4 frame animation is the same as giving 0)
    static UINT getFrameIndex(UINT start, BaseAnimation& anime); //given the millisecond an animation has started at and the time to spend per frame, return which frame should be rendered
    static glm::vec4 getFrameFromStart(UINT startinFrame, BaseAnimation& anime); //given game frame we started at, return the frame of the animation we are in
    static glm::vec4 normalizePixels(const glm::vec4& rect, Sprite& sprite); //given a subsection of an image in pixels, return the normalized subsection. Any numbers that are already 0-1 will be assumed to already be normalized
};

struct RenderRequest //bare bones info for each request: what sprite is being rendered and how to render it
{
    BasicRenderPipeline& program;
    Sprite const* sprite = nullptr; //if null, then not rendering a sprite
    GLenum mode = GL_TRIANGLES; //primitive we are rendering in

    bool operator==( const RenderRequest& r2) const
    {
        return &program == &r2.program && sprite == r2.sprite && mode == r2.mode;
    }

    bool operator<(const RenderRequest& b) const
    {
         if (&program == &b.program)
        {
            return mode < b.mode;
        }
        return &program < &b.program;
    }
};

struct TransManager //handles transluscent fragment render requests.
{
     //Transluscents must be rendered after opaques and sorted by distance from the screen from furthest
     //(smallest z) to closest (largest z) to prevent fragments from being discarded via the depth test
    struct TransRequest //each request must be sorted the z value (for sorting) and we need to know the index in "data" where the transformations are stored
    {
        RenderRequest request;
        ZType z;
        int index;
        bool operator<(const TransRequest& b) const
        {
            return (z < b.z);
        }
    };
    /**
      *   \brief Creates the request
      *
      *   \param request: the actual request
      *   \param z: the z that this request will be rendered at
      *
      *   \return nothing
      **/
    void request(const RenderRequest& request, ZType z);
    void render();
    std::vector<char> data; //buffer used to store all vertex attributes. Unsorted.
private:
    /**
      *   \brief Renders every transluscent request, attempting to render all sprites/program pairs in one go for efficiency
      *
      *   \param request: the actual request itself
      *   \param bytes: a pointer to the point in memory where all the data for this rendering is
      *   \param size: how many bytes we are sending
      *
      *   \return nothing
      **/
    void render(const RenderRequest& request, RenderPayload& payload, int instances); //renders all data in payload

    RenderPayload buffer; //reusable buffer for sorting data. Used to compile all data one sprite-renderprogram pairing.
    std::list<TransRequest> requests; //unsorted list of requests, sorted before we render
};

class SpriteManager //handles all sprite requests
{
public:
    static TransManager trans;
    /**
      *   \brief Creates a rendering request, which will be rendered at the end of every game loop
      *
      *   \param request: the actual request itself
      *   \param args: any values that the vertex shader needs
      *
      *   \return nothing
      **/
    template<typename... Args>
    static void request(const RenderRequest& request, ZType z, Args... args) //request for non-sprites
    {
        trans.request(request,z); //transluscent manager needs to make a request specifically for the sprite-program pairing
        fillBytesVec(trans.data,request.program.getBytesPerRequest(),args...); //place request into transluscent manager
    }

    ///Non-sprite requests are automatically put in TransManager
    ///Recommended for sprites, though can be used for non-sprites
    template<typename... Args>
    static void requestSprite(const RenderRequest& request_, const glm::vec4& rect, ZType z, Args... args) //request for sprites
    {
        //many sprite shaders have a rect as their 2nd parameter, so this just makes that easier to account for
        request(request_,z,rect,z,args...);
    }


    /**
      *   \brief Renders all requests
      *
      *   \return nothing
      **/
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
    static std::unique_ptr<BasicRenderPipeline> polyRenderer;
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
