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

#include "SDLhelper.h"
#include "vanilla.h"

void addPointToBuffer(float buffer[], glm::vec3 point, int index);
void addPointToBuffer(float buffer[], glm::vec2 point, int index);
void addPointToBuffer(float buffer[], glm::vec4 point, int index);

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
typedef GLuint Buffer;

const float textureVerticies[24] = { //verticies of textures
    -1, 1, 0, 1, //top left
    1, 1, 1, 1, //top right
    1, -1, 1, 0, //bottom right
    -1, 1, 0, 1, //top left
    -1, -1, 0, 0, //bottom left
    1, -1, 1, 0 //bottom right

    };

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

    //constructor that takes in an infinite list of shaders, as many as you want. Implemented below ViewPort
    template<size_t N>
    BasicRenderPipeline(LoadShaderInfo (&&info)[N], const float* verts = basicScreenCoords,
                                              int floatsPerVertex_ = 2, int vertexAmount_ = 6);

    //construct renderpipeline from shader paths.
    explicit BasicRenderPipeline(std::string vertexPath, std::string fragmentPath, //vertex and fragment shader paths
            const float* verts = basicScreenCoords, int floatsPerVertex_ = 2, int vertexAmount_ = 6); //info for verticies, by default render a rectangle size of the screen
    //initializes BasicRenderPipeline with a bunch of shaders and vertices. Do not pass in multiple vertex shaders
    template <typename T,typename... Args>
    void draw(GLenum mode, T t1, Args... args);

    void setMatrix4fv(std::string name, const GLfloat* value); //pass in the value_ptr of the matrix
    void setVec3fv(std::string name,glm::vec3 value);
    void setVec4fv(std::string name, glm::vec4 value);
    void setVec2fv(std::string name, glm::vec2 value);
    size_t getBytesPerRequest();
    Buffer getProgram();
    Buffer getVBO();
    Buffer getVAO();
    Buffer getVerticies();
    void initVerticies(const float* verts, int floatsPerVertex_, int vertexAmount); //initiates argument 0, which is assumed to be verticies.

private:
    std::vector<char> bytes;
    size_t dataAmount = 1; //number of bytes per request
    Buffer program = 0;
    Buffer VBO;
    Buffer VAO;
    Buffer verticies; //VBO for verticies
    void initAttribDivisors(Numbers numbers); //initiates inputs, assuming first input is verticies and already set by "initVerticies"
};

using RenderProgram = BasicRenderPipeline;

class RenderCamera;
struct ViewPort //has data about visible area on screen. Make sure you initialize before you do anything involving rendering
{
    enum PROJECTION_TYPE
    {
        ORTHOGRAPHIC = 0, //2d, no depth
        PERSPECTIVE = 1 //3d (but can be used for 2d)
    };
    static RenderCamera* currentCamera; //the current camera in use
    static Buffer UBO; //view and projection matricies Uniform Buffer
    static int screenWidth, screenHeight;
    static PROJECTION_TYPE proj;
    static ViewRange baseRange;  //represents the smallest and largest values x,y,z can be. X and Y should always have 0 as the smallest value.
    static ViewRange currentRange; //represents the current range for x,y, and z
    static std::unique_ptr<BasicRenderPipeline> basicProgram; //generic shader pipeline to render sprites
    static std::unique_ptr<BasicRenderPipeline> animeProgram; //shader pipeline to render spritesheets

    static void init(int screenWidth, int screenHeight); //this init function initiates the basic renderprograms

    static glm::vec2 toAbsolute(const glm::vec2& point);//given a screen coordinate, renders it to that point on the screen regardless of zoom
    static glm::vec4 toAbsolute(const glm::vec4& rect);

    static glm::vec2 toWorld(const glm::vec2& point);//if currentCamera is not null, uses currentCamera to project screen point to world point. Otherwise, return "point"
    static glm::vec4 toWorld(const glm::vec4& point);

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
    static PROJECTION_TYPE getProj();
    static constexpr float FOV = 45;
    static void flipProj();
    static glm::vec2 getScreenDimen();
    static void linkUniformBuffer(unsigned int program); //set a program to use UBO
    static void update();

private:
    static void setViewRange(const ViewRange& range);
    static void resetProjMatrix(); //reset the projection matrix in the uniform buffer
};


//warning: if you call the constructor like this:
//BasicRenderPipeline stars({{"./shaders/gravityVertexShader.h",GL_VERTEX_SHADER,true},{"./shaders/starShader.h",GL_FRAGMENT_SHADER,true}});
//it'll default to the constructor below, which is obviously not correct. The "explicit" helps prevent the program from compiling. Instead, call like:
//BasicRenderPipeline stars({LoadShaderInfo{"./shaders/gravityVertexShader.h",GL_VERTEX_SHADER,true},{"./shaders/starShader.h",GL_FRAGMENT_SHADER,true}});
//if your array doesn't have two elements (either less or more) you do not have to worry about this.
template<size_t N>
BasicRenderPipeline::BasicRenderPipeline(LoadShaderInfo (&&info)[N],const float* verts, int floatsPerVertex_, int vertexAmount_) : vertexAmount(vertexAmount_)
{
    ViewPort::linkUniformBuffer(program);

    program = glCreateProgram();

    Numbers numbahs;
    for (auto&& shaderInfo : info)
    {
        GLuint shader = loadShaders(std::move(shaderInfo),&numbahs);
        if (shader != -1)
        {
            glAttachShader(program,shader);
            glDeleteShader(shader);
        }
    }
    glLinkProgram(program);

    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    initVerticies(verts,floatsPerVertex_,vertexAmount_);
    initAttribDivisors(numbahs);
}


template <typename T,typename... Args>
void BasicRenderPipeline::draw(GLenum mode, T t1, Args... args) //pass in a bunch of data and then draw
{ //maybe consider making this a separate function that takes in a BasicRenderPipeline and draws rather than calling it from the Pipeline itself
    bytes.clear();
    fillBytesVec(bytes,dataAmount,t1,args...);
    glBindVertexArray(VAO);
    if (dataAmount)
    {
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER,dataAmount,&bytes[0],GL_DYNAMIC_DRAW);
    }
    glUseProgram(program);
    glDrawArrays(mode,0,vertexAmount);
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

    glm::vec2 toWorld(const glm::vec2& point) const; //converts a rect from the screen coordinate to the world coordinate
    glm::vec4 toWorld(const glm::vec4& rect) const;

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

class OpaqueManager //manages storing opaque (non-transluscent) fragment render requests
{
    //opaques can be rendered in any order, since they can't be blended, so OpaqueManager simply allocates contiguous memory for
    //each Sprite-BasicRenderPipeline pair and renders all the data at once
    struct OpaquePair //hash a pair of Sprite and BasicRenderPipelines by XOring their hashes
    {
        size_t operator()(const std::pair<Sprite*,BasicRenderPipeline&>& a) const //we don't have to worry about commutativity because they have different memory addresses
        {
            return std::hash<Sprite*>()(a.first) ^ std::hash<BasicRenderPipeline*>()(&a.second);
        }
    };
    struct OpaqueEquals //how to determine if two pairs are the same (can't believe I had to specify this tbh)
    {
        bool operator()(const std::pair<Sprite*,BasicRenderPipeline&>& a,const std::pair<Sprite*,BasicRenderPipeline&>& b) const
        {
            return a.first == b.first && &a.second == &b.second;
        }
    };
public:
    std::unordered_map<std::pair<Sprite*,BasicRenderPipeline&>,std::vector<char>,OpaquePair,OpaqueEquals> opaquesMap;
    //for each sprite-renderprogram pairing, they have a unique bytes buffer
    void render(); //pass each bytes buffer to rendering pipeline
};

struct RenderRequest //bare bones info for each request: what sprite is being rendered and how to render it
{
    BasicRenderPipeline& program;
    Sprite* sprite = nullptr; //if null, then not rendering a sprite
    GLenum mode = GL_TRIANGLES; //primitive we are rendering in
};

typedef int ZType; //used to represent z values
struct TransManager //handles transluscent fragment render requests.
{
     //Transluscents must be rendered after opaques and sorted by distance from the screen from furthest
     //(smallest z) to closest (largest z) to prevent fragments from being discarded via the depth test
    struct TransRequest //each request must be sorted the z value (for sorting) and we need to know the index in "data" where the transformations are stored
    {
        RenderRequest request;
        ZType z;
        int index;
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
    struct TransRequestCompare //returns true if "a" is "less than" "b", and thus should be rendered first
    {
        bool operator()(const TransRequest& a, const TransRequest& b) const //sort by z, then by render mode, then by program, then by sprite, then by primitive
        {
            if (a.z == b.z)
            {
                if (&a.request.program == &b.request.program)
                {
                    if (a.request.sprite == b.request.sprite)
                    {
                        return a.request.mode < b.request.mode; //mode changes the least between requests, usually sprite-program pairs have the same mode, so we sort it last
                    }
                    return &a.request.sprite < &b.request.sprite;
                }
                return &a.request.program < &b.request.program;
            }
            return a.z < b.z; //fragments with smaller zs get rendered first
        }
    };
    /**
      *   \brief Renders every transluscent request, attempting to render all sprites/program pairs in one go for efficiency
      *
      *   \param request: the actual request itself
      *   \param bytes: a pointer to the point in memory where all the data for this rendering is
      *   \param size: how many bytes we are sending
      *
      *   \return nothing
      **/
    void render(const RenderRequest& request, char* bytes, int size);
    std::vector<char> buffer; //reusable buffer for sorting data. Used to compile all data one sprite-renderprogram pairing.
    std::multiset<TransRequest,TransRequestCompare> requests;
};

class SpriteManager //handles all sprite requests
{
    static OpaqueManager opaques;
    static TransManager trans;

public:
    /**
      *   \brief Creates a rendering request, which will be rendered at the end of every game loop
      *
      *   \param request: the actual request itself
      *   \param transluscent: only relevant if "request.sprite" is null. Indicates whether a non-sprite request is transluscent
      *   \param args: any values that the vertex shader needs
      *
      *   \return nothing
      **/
    template<typename... Args>
    static void request(const RenderRequest& request, ZType z, bool transluscent, Args... args) //request for non-sprites
    {
        if ((request.sprite && request.sprite->getTransluscent()) || transluscent)
        {
            trans.request(request,z); //transluscent manager needs to make a request specifically for the sprite-program pairing
            fillBytesVec(trans.data,request.program.getBytesPerRequest(),args...); //place request into transluscent manager
        }
        else
        {
            fillBytesVec(opaques.opaquesMap[{request.sprite,request.program}],request.program.getBytesPerRequest(),args...); //place request into opaque manager
        }
    }
    ///Same function as above but you don't have to provide the "transluscent" parameter. Non-sprite requests are automatically put in TransManager
    ///Recommended for sprites, though can be used for non-sprites
    template<typename... Args>
    static void requestSprite(const RenderRequest& request_, const glm::vec4& rect, ZType z, Args... args) //request for sprites
    {
        //many sprite shaders have a rect as their 2nd parameter, so this just makes that easier to account for
        request(request_,z,true,rect,z,args...);
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
