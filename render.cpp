#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "SDL.h"

#include "geometry.h"
#include "render.h"
#include "SDLhelper.h"
#include "resourcesMaster.h"

bool GLContext::context = false;
SDL_Window* GLContext::window = 0;

void GLContext::init(int screenWidth, int screenHeight)
{
    window = SDL_CreateWindow("Project",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    SDL_GL_CreateContext(window);
    context = true;
}

bool GLContext::isContextValid()
{
    return context;
}

void GLContext::update()
{
    SDL_GL_SwapWindow(window);
}

void GLContext::terminate()
{
    context = false;
   if (window)
   {
       SDL_DestroyWindow(window);
       window = 0;
   }
}

void addPointToBuffer(float buffer[], glm::vec4 point, int index)
{
    buffer[index] = point.x;
    buffer[index+1] = point.y;
    buffer[index+2] = point.z;
    buffer[index+3] = point.a;
}
void addPointToBuffer(float buffer[], glm::vec3 point, int index) //given a buffer, will add point to it at the specified index. Will not attempt to catch index out of bounds errors
{
    buffer[index] = point.x;
    buffer[index+1] = point.y;
    buffer[index+2] = point.z;
}
void addPointToBuffer(float buffer[], glm::vec2 point, int index)
{
    addPointToBuffer(buffer, {point.x, point.y, 0}, index);
}

std::string glslTypesCaptureGroup = "(bool|int|float|vec([2-4])|mat([2-4]))"; //regex for capturing glsl types
std::string findVertexInputsRgx = "layout \\(location = ([0-9]+)\\) in " + glslTypesCaptureGroup +" .+;\\s*"; //regex for vertex shader inputs
Numbers getVertexInputs(const std::string& vertexFile)
{
    Numbers numbers;
    regexSearch(findVertexInputsRgx,vertexFile,[&numbers](std::sregex_iterator& it){
    std::string type = (*it)[2];
    if (type == "bool" || type == "int" || type == "float")
    {
        numbers.push_back(1);
    }
    else if (type.substr(0,3) == "vec")
    {
        numbers.push_back(std::stoi((*it)[3]));
    }
    else if (type.substr(0,3) == "mat")
    {
        numbers.push_back(pow(std::stoi((*it)[3]),2));
    }
    }); //replace environment variables
    return numbers;
}


int loadShaders(const GLchar* source, GLenum shaderType, Numbers* numbers )
{
    return loadShaders({source,shaderType},numbers);
}

int loadShaders(LoadShaderInfo&& info, Numbers* numbers)
{
    std::string* shaderCode = nullptr;
    std::pair<std::string, bool> result;
    //std::cout << info.isFilePath << " " << info.code << "\n";
    if (info.isFilePath)
    {
        result = readFile(info.code);

        int shader = -1;
        if (result.second)
        {
            shaderCode = &result.first;
        }
        else
        {
            std::cerr << "Can't find shader! Source: " << info.code << std::endl;
            return -1;
        }
    }
    else
    {
        shaderCode = &info.code;
    }

    //strip comments, makes processing easier
    *shaderCode = stripComments(*shaderCode);

    //lambda to replace environment variables.
    auto lambda = [&shaderCode](std::sregex_iterator& it){
                std::string varName = (*it)[1];
                if (ResourcesConfig::config.find(varName) != ResourcesConfig::config.end())
                {
                    *shaderCode = shaderCode->substr(0,it->position(0)) + ResourcesConfig::config[varName] + shaderCode->substr(it->position(0) + it->length(), shaderCode->size() - 1);
                }
                };

    //replace environment variables
    std::string varDefRegex = "\\$\\{(.*)\\}"; //regex for finding variables
    regexSearch(varDefRegex,*shaderCode,lambda);


    //import include files
    std::ifstream openIncludeFile;
    regexSearch("#include \"((.+\/)*[^\/]+)\"",*shaderCode,[&openIncludeFile,&shaderCode,&info](std::sregex_iterator& it){
                std::string fileName = (*it)[1];//(*it)[2];
                openIncludeFile.open(fileName);
                if (openIncludeFile.is_open())
                {
                    std::stringstream stream;
                    stream << openIncludeFile.rdbuf();
                    *shaderCode = shaderCode->substr(0,it->position(0)) + stream.str() + shaderCode->substr(it->position(0) + it->length(), shaderCode->size() - 1);
                }
                else
                {
                    std::cout << "Error loading shader " << (info.isFilePath ? info.code : "Unknown Shader") << ": Failed to find open include file " << fileName << std::endl;
                }
                });
    //replace environment variables inside include files
    regexSearch(varDefRegex,*shaderCode,lambda);

    //get vertex shader inputs
    if (info.shaderType == GL_VERTEX_SHADER && numbers)
    {
        *numbers = getVertexInputs(*shaderCode);
    }

    //actually create and compile the shader
    GLuint shader = glCreateShader(info.shaderType);

    const char* code = shaderCode->c_str();
    glShaderSource(shader,1, &code, NULL);
    glCompileShader(shader);

    //check for errors with the compilation process
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Error loading shader " << (info.isFilePath ? info.code : "Unknown Shader") << ": " << infoLog << std::endl;
        return -1;
    }

    return shader;

}

std::string templateShader(const std::string& shaderContents,
                           bool isVertex, //whether or not the shader is a vertex shader, determines the syntax of the inputs
                           std::initializer_list<std::string> inputs,
                           std::initializer_list<std::string> outputs,
                           std::initializer_list<std::string> tasks)
{
    //each iterator list has to have the type and name of the variable (e.g. "vec2 pos")

    //it is assumed that all inputs in a file will be grouped together as will all outputs. This is not a huge deal except for vertex shaders, where this
    //function has to find the largest layout number. For simplicity, it is assumed that all the layouts are in the same group of lines, right after one another
    //and the there are no "holes" in the layouts, meaning if the largest layout number is 10, numbers 0-10 are all used. It is also expected that the layouts
    //are organized in numerical order, so that the last layout input has the largest index. If you're not organizing your code like this already you should
    //probably be put down. Finally, comments break this function. Please call stripComments first
    std::string findLastInputRegex = isVertex ? "(" + findVertexInputsRgx + ")+" : //finds all the existing vertex inputs
                                     "(in " + glslTypesCaptureGroup + " .+\\n)+";

    std::string finalShader = "";
    //handle inputs
    regexSearch(findLastInputRegex,shaderContents,[&shaderContents,&finalShader,&inputs,isVertex](std::sregex_iterator& it)
                {
                    int layout = 0; //largest layout parameter, only used for vertex shaders


                    if (isVertex) //find the largest layout parameter
                    {
                        layout = std::stoi((*it)[2]); //the 2nd capture group is the last number in the regex. Not sure how this works exactly but it seems to
                    }

                    int i = 1;
                    for (auto input : inputs)
                    {
                        if (isVertex)
                        {
                            finalShader += "layout (location = " + std::to_string(layout + i) + ") ";
                        }
                        finalShader += "in " + input + ";\n";
                        i++;
                    }
                    finalShader = shaderContents.substr(0, it->position(0) + it->length()) + finalShader + shaderContents.substr(it->position(0) + it->length(), shaderContents.size());
                });

    //handle outputs now
    regexSearch("(out " + glslTypesCaptureGroup + " .+\\n)+", finalShader,[&finalShader,&outputs](std::sregex_iterator& it){
                    std::string newOutputs = "";
                    for (auto output : outputs)
                    {
                        newOutputs += "out " + output + ";\n";
                    }
                    finalShader = finalShader.substr(0, it->position(0) + it->length()) + newOutputs + finalShader.substr(it->position(0) + it->length(), finalShader.size());
                } );

    //modify the main function
     regexSearch("void main\\(\\)\\s*\\{([\\s\\S]*)\\}", finalShader,[&finalShader,&tasks](std::sregex_iterator& it){
                    std::string newTasks = "";
                    for (auto task : tasks)
                    {
                        newTasks += task + ";\n"; //no need to provide semicolons, we provide them for you!
                    }
                    finalShader = finalShader.substr(0, it->position(1) + it->length(1)) + "\n" + newTasks + finalShader.substr(it->position(1) + it->length(1), finalShader.size());
                } );
    return finalShader;
}

std::string stripComments(const std::string& shaderContents)
{
    return std::regex_replace (shaderContents,std::regex("\\/\\/.+\\n"),"\n");

}

BasicRenderPipeline::BasicRenderPipeline(std::string vertexPath, std::string fragmentPath, const float* verts, int floatsPerVertex_ , int vertexAmount_) : BasicRenderPipeline({{vertexPath,GL_VERTEX_SHADER},{fragmentPath,GL_FRAGMENT_SHADER}},
                                                                                                                                                                               verts,floatsPerVertex_,vertexAmount_)
{

}

void BasicRenderPipeline::initVerticies(const float* verts, int floatsPerVertex_ , int vertexAmount_)
{
    if (verts && floatsPerVertex_ > 0 && vertexAmount_ > 0)
    {
        glBindVertexArray(VAO);
        glGenBuffers(1,&verticies);
        glBindBuffer(GL_ARRAY_BUFFER,verticies);

        glBufferData(GL_ARRAY_BUFFER, floatsPerVertex_*vertexAmount_*sizeof(float),verts, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0,floatsPerVertex_,GL_FLOAT,GL_FALSE,0,0);
    }
}

void BasicRenderPipeline::initAttribDivisors(Numbers numbers)
{
    int total = 0;
    for (auto num: numbers)
    {
        total += num;
    }

    dataAmount = total*sizeof(GLfloat);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    int index = 1; //we start at 1 because RenderProgram uses index 0 to store verticies
    int aggregate = 0;

    while (index < numbers.size()) //we skip the first input, assuming it is the vertex. If your vertex shader's first input is not verticies, this function will not work
    {
        int num = numbers[index];
        for (int i = 0; i < num; i+= 4) //usually this only runs once, but if you have something that's larger than 4 floats (matricies) this will pass in every 4 floats in
        {
            int amount = std::min(num-i,4); //can't store larger than a vec4 at a time
            glVertexAttribPointer(index, amount, GL_FLOAT, GL_FALSE,total*sizeof(float), (void*)(aggregate*sizeof(float)));
            glEnableVertexAttribArray(index);
            glVertexAttribDivisor(index, 1);
            index ++;
            aggregate += amount;

        }
    }
}

RenderCamera* ViewPort::currentCamera = nullptr;
Buffer ViewPort::UBO = 0;
int ViewPort::screenWidth = 0;
int ViewPort::screenHeight = 0;
ViewPort::PROJECTION_TYPE ViewPort::proj = ORTHOGRAPHIC;

ViewRange ViewPort::baseRange;
ViewRange ViewPort::currentRange;

std::unique_ptr<RenderProgram> ViewPort::basicProgram;
std::unique_ptr<RenderProgram> ViewPort::animeProgram;


void ViewPort::init(int screenWidth, int screenHeight)
{
    ViewPort::screenWidth = screenWidth;
    ViewPort::screenHeight = screenHeight;

    glewExperimental = true;

    GLenum err=glewInit();
      if(err!=GLEW_OK) {
        // Problem: glewInit failed, something is seriously wrong.
        std::cout << "glewInit failed: " << glewGetErrorString(err) << std::endl;
      }

    baseRange =
    {
        {0,screenWidth},
        {0,screenHeight},
        {0.0f,100.0f}     //magic numbers. Can be anything for orthographic views but perspective views will clamp the x value to 0.1, as it must be positive
    };
    currentRange = baseRange;

glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);

    glDepthFunc(GL_LEQUAL);



    basicProgram = std::make_unique<RenderProgram>(ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/vertex/betterShader.h",
                      ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/fragment/fragmentShader.h");
    animeProgram = std::make_unique<RenderProgram>(ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/vertex/animationShader.h",
                      ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/fragment/fragmentShader.h");

    glGenBuffers(1,&UBO);




}


glm::vec4 RenderCamera::toScreen(const glm::vec4& change) const
{
        //REFACTOR: Finish this for perspective mode

    glm::vec2 point = toScreen({change.x,change.y});
    return {point.x,point.y,change.z, change.a};
}

glm::vec2 RenderCamera::toScreen(const glm::vec2& point) const
{
    //REFACTOR: Finish this for perspective mode

    return (point - getTopLeft())/glm::vec2(ViewPort::getViewWidth(),ViewPort::getViewHeight())*ViewPort::getScreenDimen();
}

glm::vec4 RenderCamera::toWorld(const glm::vec4& change) const
{
    //REFACTOR: Finish this for perspective mode

    glm::vec2 point = toWorld({change.x,change.y});
    return {point.x,point.y , change.z, change.a};
}

glm::vec2 RenderCamera::toWorld(const glm::vec2& point) const
{    //REFACTOR: Finish this for perspective mode

     glm::vec2 screenDimen = ViewPort::getScreenDimen();
    return getTopLeft() + glm::vec2(point.x/screenDimen.x*ViewPort::getViewWidth(), (point.y/screenDimen.y*ViewPort::getViewHeight()));
}

glm::vec4 RenderCamera::toAbsolute(const glm::vec4& rect) const
{
    if (ViewPort::getProj() == ViewPort::PERSPECTIVE)
    {
        //returns toAbsolute if rect is rendered at 1 distance from the camera
        glm::vec2 screenDimen = ViewPort::getScreenDimen();

        float screenHeight = 2*(1)*(tan(ViewPort::FOV/180*M_PI/2));
        float screenWidth = screenHeight/screenDimen.y*screenDimen.x;

        return glm::vec4(-screenWidth/2 + pos.x + (rect.x)/screenDimen.x*screenWidth,
                -screenHeight/2 + pos.y + (rect.y)/screenDimen.y*screenHeight,
                rect.z/screenDimen.x*screenWidth,
                rect.a/screenDimen.y*screenHeight);
    }
    return glm::vec4(getTopLeft() + glm::vec2(rect.x,rect.y),rect.z,rect.a);
}
glm::vec2 RenderCamera::toAbsolute(const glm::vec2& point) const
{
    glm::vec4 rect = toAbsolute(glm::vec4(point,0,0));
    return {rect.x,rect.y};
}
glm::vec2 ViewPort::toAbsolute(const glm::vec2& point)
{
    return currentCamera ? currentCamera->toAbsolute(point) : point;
}
glm::vec4 ViewPort::toAbsolute(const glm::vec4& rect)
{
    return currentCamera ? currentCamera->toAbsolute(rect) : rect;
}

glm::vec2 ViewPort::toWorld(const glm::vec2& point)
{
    return currentCamera ? currentCamera->toWorld(point) : point;
}

glm::vec4 ViewPort::toWorld(const glm::vec4& rect)
{
    return glm::vec4(toWorld({rect.x,rect.y}),rect.z,rect.a);
}

glm::vec2 ViewPort::toScreen(const glm::vec2& point)
{
    return currentCamera ? currentCamera->toScreen(point) : point;
}

glm::vec4 ViewPort::toScreen(const glm::vec4& rect)
{
    return glm::vec4(toScreen({rect.x,rect.y}),rect.z,rect.a);
}

const glm::vec2& ViewPort::getXRange()
{
    return currentRange.xRange;
}
const glm::vec2& ViewPort::getYRange()
{
    return currentRange.yRange;
}
const glm::vec2& ViewPort::getZRange()
{
    return currentRange.zRange;
}

const float ViewPort::getViewWidth()
{
    return currentRange.xRange.y - currentRange.xRange.x;
}

const float ViewPort::getViewHeight()
{
    return currentRange.yRange.y - currentRange.yRange.x;
}

const float ViewPort::getViewDepth()
{
    return currentRange.zRange.y - currentRange.zRange.x;
}

void ViewPort::setXRange(float x1, float x2)
{
    setViewRange({
                 {x1,x2},
                 currentRange.yRange,
                 currentRange.zRange
                 });
}
void ViewPort::setYRange(float y1, float y2)
{
    setViewRange({
                 currentRange.xRange,
                 {y1,y2},
                 currentRange.zRange
                 });
}
void ViewPort::setZRange(float z1, float z2)
{
    setViewRange({
                 currentRange.xRange,
                 currentRange.yRange,
                 {z1,z2}
                 });
}

void ViewPort::resetViewRange()
{
    setViewRange(baseRange);
}

glm::mat4 ViewPort::getProjMatrix()
{
    if (proj) //return PROJECTION view
    {
        return glm::scale(glm::perspective(glm::radians(FOV), (float)screenWidth/(float)screenHeight, std::max(0.1f,currentRange.zRange.x), currentRange.zRange.y),
                      glm::vec3(1,-1,1)); //flip y because otherwise sprites will be upside down
    } //otherwise return orthographic view
    return (glm::ortho(currentRange.xRange.x, currentRange.xRange.y, currentRange.yRange.y, currentRange.yRange.x, currentRange.zRange.x, currentRange.zRange.y));
}

ViewPort::PROJECTION_TYPE ViewPort::getProj()
{
    return proj;
}

void ViewPort::flipProj()
{
    proj = static_cast<PROJECTION_TYPE>(!(static_cast<bool>(proj)));

    resetProjMatrix();
}

glm::vec2 ViewPort::getScreenDimen()
{
    return {screenWidth,screenHeight};
}

void ViewPort::linkUniformBuffer(unsigned int program)
{
    unsigned int index =glGetUniformBlockIndex(program,"Matrices");
    //std::cout << glGetError() << "\n";
    if (index != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(program, index, 0);
    }

}

void ViewPort::update()
{
    glBindBuffer(GL_UNIFORM_BUFFER,UBO);
    glm::mat4 view;
    if (currentCamera) //REFACTOR: might be faster to not have to recalculate this every frame
    {
        glm::vec3 pos = currentCamera->getPos() - static_cast<float>(proj == ORTHOGRAPHIC)*0.5f*glm::vec3(ViewPort::getViewWidth(),ViewPort::getViewHeight(),0); //for some reason, looking at point(x,y) in ortho view causes point(x,y) to be on the top left corner of the screen
                                                                                                                 //not sure why, but that's a problem for future leo, this here is a work around
        view = glm::lookAt(pos,glm::vec3(glm::vec2(pos),currentRange.zRange.x),glm::vec3(0,1,0));
    }
    else
    {
        //glm::vec2 pos = {screenWidth/2,screenHeight/2};
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, currentRange.zRange.y)); //if no camera, put view matrix at farthest possible Z
        //glm::lookAt(glm::vec3(pos,currentRange.zRange.y),glm::vec3(pos,currentRange.zRange.x),glm::vec3(0,1,0));
    }
    glBufferSubData(GL_UNIFORM_BUFFER,sizeof(glm::mat4),sizeof(glm::mat4),glm::value_ptr(view));
    float cameraZ = currentCamera ? currentCamera->getPos().z : currentRange.zRange.y;
    glBufferSubData(GL_UNIFORM_BUFFER,2*sizeof(glm::mat4),sizeof(float),&cameraZ);

}

void ViewPort::setViewRange(const ViewRange& range)
{
    currentRange = range;
    resetProjMatrix();
}

void ViewPort::resetProjMatrix()
{
    glBindBuffer(GL_UNIFORM_BUFFER,UBO);
    //float* projection = glm::value_ptr(getOrtho());
    glBufferData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4) + sizeof(float), glm::value_ptr(getProjMatrix()), GL_STATIC_DRAW); // allocate enough memory for two 4x4 matricies and the camera position. Remember that a glm::vec4 is 16 bytes, so each matrix is 64 bytes
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

RenderProgram::RenderProgram(std::string vertexShader, std::string fragmentShader, const float* verts, int floatsPerVertex_, int vertexAmount_) : BasicRenderPipeline({{vertexShader,GL_VERTEX_SHADER},{fragmentShader,GL_FRAGMENT_SHADER}},
                                                                                                                                                                               verts,floatsPerVertex_,vertexAmount_)
{

}

void RenderProgram::use()
{
    //setMatrix4fv("view",view);
    //setMatrix4fv("projection",value_ptr(RenderProgram::getOrtho()));
    glUseProgram(program);
}

void RenderProgram::setMatrix4fv(std::string name, const GLfloat* value)
{
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program,name.c_str()),1,GL_FALSE,value);
    glUseProgram(0);
}
void RenderProgram::setVec3fv(std::string name,glm::vec3 value)
{
    glUseProgram(program);
    glUniform3fv(glGetUniformLocation(program,name.c_str()),1,glm::value_ptr(value));
    glUseProgram(0);
}
void RenderProgram::setVec4fv(std::string name,glm::vec4 value)
{
    glUseProgram(program);
    glUniform4fv(glGetUniformLocation(program,name.c_str()),1,glm::value_ptr(value));
    glUseProgram(0);
}
void RenderProgram::setVec2fv(std::string name, glm::vec2 value)
{
    glUseProgram(program);
    glUniform2fv(glGetUniformLocation(program,name.c_str()),1,glm::value_ptr(value));
    glUseProgram(0);
}

void RenderProgram::drawInstanced(Buffer sprite, void* data, int instances)
{
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D,sprite);

    if (data)
    {
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER,dataAmount*instances,data,GL_DYNAMIC_DRAW);
    }

    use();

    glDrawArraysInstanced(GL_TRIANGLES,0,6,instances);

    //glDrawElementsInstanced(GL_TRIANGLES,6,GL_UNSIGNED_INT,indices,instances);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

int RenderProgram::getRequestDataAmount()
{
    return dataAmount ? dataAmount : 1; //return 1 instead of 0 as 0 messes things up.
}

RenderCamera::~RenderCamera()
{
    if (ViewPort::currentCamera == this)
    {
        ViewPort::currentCamera = 0;
    }
}

void RenderCamera::init(const glm::vec3& pos_)
{
    setPos(pos_);
}

const glm::vec3& RenderCamera::getPos() const
{
    return pos;
}

glm::vec2 RenderCamera::getTopLeft() const
{
    glm::vec2 dimen = ViewPort::getScreenDimen();

    return {pos.x - dimen.x/2, pos.y - dimen.y/2};
}

glm::vec4 RenderCamera::getRect() const
{
    glm::vec2 dimen = ViewPort::getScreenDimen();
    return glm::vec4(getTopLeft(), dimen.x,dimen.y);
}

void RenderCamera::setPos(const glm::vec3& pos_)
{
    pos = pos_;
}

void RenderCamera::setPos(const glm::vec2& pos_)
{
    pos = glm::vec3(pos_,pos.z);
}

void RenderCamera::addVector(const glm::vec2& vec)
{
    setPos(glm::vec3(pos.x + vec.x, pos.y + vec.y, pos.z));
}

void RenderCamera::addVector(const glm::vec3& vec)
{
    setPos(pos + vec);
}

bool isTransluscent(unsigned char* sprite, int width, int height)
{
    for (int i = 3; i < width*height*4; i +=4)
    {
        if (sprite[i] != 255)
        {
        //note: it's possible to make this check for alpha values of 0 as well, and then discard fragments of pure transparency in the shader,
        //to avoid z-sorting. the issue is that as long as sprites use GL_LINEAR instead of GL_NEAREST for
        //GL_TEXTURE_MIN_FILTER and GL_TEXTURE_MAG_FILTER, a purely opaque or transparent image might have
        //blurred pixels due to linear interpolation.
            return true;
        }
    }
    return false;
}


   void Sprite::load(std::string source)
    {
        if (GLContext::isContextValid())
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D,texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


            int channels;
            //auto imageData = cv::imread(source);
            unsigned char* data = stbi_load(source.c_str(),&width, &height, &channels, 0);
            int rgb = 0;// GL_RGB*!transparent + GL_RGBA*transparent;
            switch (channels)
            {
            case 1:
                rgb = GL_RED;
                break;
            case 2:
                rgb = GL_RG;
                break;
            case 3:
                rgb = GL_RGB;
                break;
            case 4:
                rgb = GL_RGBA;
                transluscent = isTransluscent(data,width,height);
                //std::cout << source << " " << transluscent << "\n";
                break;
            }

            if (data)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, rgb,width, height, 0, rgb, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                std::cout << "Error loading texture: " << source << std::endl;
            }
            stbi_image_free(data);
        }
        else
        {
            throw std::runtime_error("WOAW MISS\n\nPUMP THE BRAKES\n\nARE YOU TRYING TO INITIALIZE A SPRITE WITHOUT AN OPENGL CONTEXT???\n\nI'm going to have to respectfully decline\n\nYou can\'t initialize textures before initializing an OpenGL Context(https://www.khronos.org/opengl/wiki/Common_Mistakes#The_Object_Oriented_Language_Problem)");
        }

    }

Sprite::Sprite( std::string source)
    {
        init(source);
    }
    Sprite::~Sprite()
    {
        /*if there is no opengl context glDeleteTextures crashes. "context" = false means that our main function has ended, in which case there is no longer
        a guarnatee to be an openGL context (and the OS will free our memory anyway) so don't delete the texture*/
        //if (GLContext::isContextValid())
        //glDeleteTextures(1,&texture);
    }

void Sprite::init(std::string source_)
    {
        load(source_);
        source = source_;
    }
unsigned int Sprite::getTexture()
{
    return texture;
}
std::string Sprite::getSource()
{
    return source;
}

bool Sprite::getTransluscent()
{
    return transluscent;
}

glm::vec2 Sprite::getDimen()
{
    return {width,height};
}

Sprite9::Sprite9(std::string source, glm::vec2 W, glm::vec2 H) : Sprite(source)
{
    widths = W;
    heights = H;
}
void Sprite9::init(std::string source,glm::vec2 W, glm::vec2 H)
{
    Sprite::init(source);
    widths = W;
    heights = H;
}


/*void Sprite9::loadData(GLfloat* data, const SpriteParameter& parameter, int index)
{
        const SpriteParameter* current = &parameter;
        glm::vec4 rect = current->rect;
        bool tooWide = (widths.x+widths.y>rect.z); //whether or not the requested width is bigger than the frame portion;
        bool tooHigh = (heights.x+heights.y>rect.a);
        glm::vec2 modW = {widths.x-(widths.x+widths.y-rect.z)*.5*tooWide,widths.y-(widths.y+widths.x-rect.z)*.5*tooWide}; //widths of the non-scalable portions
        glm::vec2 modH = {heights.x-(heights.x+heights.y-rect.z)*.5*tooHigh,heights.y-(heights.y+heights.x-rect.z)*.5*tooHigh};
        //std::cout << modW.x << " " << modW.y << std::endl;
        for ( int g = 0; g < 3; g ++)
        {
            float horiz = (rect.z-modW.x-modW.y)*!tooWide; //the width of the scalable parts. We also have to consider if the width is higher than the frame width
            float vert = (rect.a- modH.x-modH.y)*!tooHigh;//the height of the scalable parts
            for (int h = 0; h < 3; h ++)
            {
                glm::vec4 r = {rect.x + modW.x*(h > 0) + (horiz)*(h>1), //where to display this portion
                                rect.y + modH.x*(g>0) + (vert)*(g>1),
                                modW.x*(h == 0) + horiz*(h==1)+modW.y*(h==2),
                                modH.x*(g==0) + vert * (g == 1) + modH.y*(g==2)};
               glm::vec2 center = rotatePoint({r.x+r.z/2,r.y+r.a/2},{rect.x + rect.z/2, rect.y + rect.a/2},current->radians);
               Sprite::loadData(data,{{center.x - r.z/2,center.y - r.a/2,r.z,r.a},
                                   current->radians,parameter.z,NONE,{1.0/3*h,(1.0/3)*g,1.0/3,1.0/3}}, index + (g*3+h)*floats);
            }
        }
}*/



UINT BaseAnimation::getTotalFrames()
{
    return perRow*rows;
}

UINT BaseAnimation::getFrameIndex(UINT start, BaseAnimation& anime)
{
    if (anime.fps == 0)
    {
        return 0;
    }
    return ((SDL_GetTicks() - start)/1000.0)*anime.fps;
}

glm::vec4 BaseAnimation::getNthFrame(UINT n, BaseAnimation& anime)
{
    int xFrame = n%anime.perRow; //the number of frames horizontally
    int yFrame = (n/anime.perRow)%anime.rows; //the number of frames horizontally
    float width = anime.subSection.z/anime.perRow;
    float height = anime.subSection.a/anime.rows;
    return {anime.subSection.x + width*xFrame,anime.subSection.y + height*yFrame,width,height};
}

glm::vec4 BaseAnimation::getFrameFromStart(UINT startingFrame, BaseAnimation& anime)
{
    return getNthFrame(getFrameIndex(startingFrame,anime),anime);
}

glm::vec4 BaseAnimation::normalizePixels(const glm::vec4& rect, Sprite& sprite)
{
    glm::vec2 dimens = sprite.getDimen();
    glm::vec4 answer = {rect.x > 1 ? rect.x/dimens.x : rect.x,rect.y > 1 ? rect.y/dimens.y : rect.y ,1,1};
    answer.z = rect.z > 1 ? rect.z/dimens.x : rect.z;
    if (answer.z + answer.x > 1)
    {
        answer.z = 1 - answer.x;
    }
    answer.a = rect.a > 1 ? rect.a/dimens.y : rect.a;
    if (answer.a + answer.y > 1)
    {
        answer.a = 1 - answer.y;
    }
    return answer;
}
/*glm::vec4 BaseAnimation::getPortion(const AnimationParameter& param)
{
        //int current =  SDL_GetTicks();
        /*glm::vec4 backup = subSection;
        if (param.subSection.z != 0 || param.subSection.a != 0) //if the param.subSection is invalid, use our preset subsection
        {
            subSection = param.subSection;
        }
        int perRow = subSection.z; //frame per rows
        int rows = subSection.a; //number of rows
        int framesSince = BaseAnimation::getFrameIndex(param.start,((param.fps == -1)*fps + (param.fps != -1)*param.fps));
        if (param.start < 0 )
        {
            framesSince = (SDL_GetTicks())/1000.0; //frames that have passed
        }

        glm::vec4 answer = {frameDimen.x*(framesSince%(perRow)) + subSection.x,
        (frameDimen.y*((framesSince/perRow)%rows)) + subSection.y,
        frameDimen.x,
        frameDimen.y};

        subSection = backup;

        return answer;
/*}

SpriteParameter BaseAnimation::processParam(const SpriteParameter& sParam,const AnimationParameter& aParam)
{
    /*Returns a SpriteParameter that represents what to render. sParam.portion is interpreted as the portion of the sprite sheet to render*/
    //SpriteParameter param = sParam;
    /*if (aParam.transform && aParam.camera) //call a camera function on our rect to render it according to the camera
    {
        param.rect = ((aParam.camera)->*(aParam.transform))(param.rect);
    }
    //double timeSince = current - ptr->start;
   // int framesSince = ((ptr->fps == -1)*fps + (ptr->fps != -1)*ptr->fps)*timeSince; //frames that have passed

    glm::vec4 portion = getPortion(aParam);
    param.portion = {param.portion.x + portion.x, param.portion.y + portion.y, param.portion.z*portion.z, param.portion.a*portion.a};*/
/*    return param;
}


/*void BaseAnimation::renderInstanced(RenderProgram& program, const std::list<FullAnimationParameter>& parameters)
{
    auto size = parameters.end();
    std::vector<SpriteParameter> params;
    int perRow = subSection.z; //frame per rows
    int rows = subSection.a; //number of rows
    for (auto i = parameters.begin(); i != size; ++i)
    {
        params.push_back(processParam(i->first,i->second));
    }
    Sprite::renderInstanced(program, params);
}

void BaseAnimation::renderInstanced(RenderProgram& program, const std::vector<SpriteParameter>& parameters)
{
    int size = parameters.size();
    std::vector<SpriteParameter> params;
    int perRow = subSection.z; //frame per rows
    int rows = subSection.a; //number of rows
    for (int i = 0; i < size; i ++)
    {
        SpriteParameter param = parameters[i];
        //double timeSince = current - ptr->start;
       // int framesSince = ((ptr->fps == -1)*fps + (ptr->fps != -1)*ptr->fps)*timeSince; //frames that have passed
        param.portion = getPortion({(int)SDL_GetTicks(),-1});
        params.push_back({param});
    }
    Sprite::renderInstanced(program, params);
}*/



void TransManager::request(Sprite& sprite, RenderProgram& program, ZType z)
{
    requests.insert({sprite,program,z,data.size()});
}

void TransManager::render()
{
    for (auto it = requests.begin(); it != requests.end(); ++it)
    {
        buffer.insert(buffer.end(),&data[it->index],&data[it->index] + it->program.getRequestDataAmount()); //for each request, store the data into buffer
        if (it == std::prev(requests.end()) || &std::next(it)->sprite != &it->sprite || &std::next(it)->program != &it->program) //if we have run out of requests, or if the next request requires a different sprite/renderprogram
        {
            /*std::cout << it->sprite.getSource() << ": ";
            for (int i = 0; i < buffer.size(); i += sizeof(float))
            {
                int f;
                memcpy(&f,&buffer[i],sizeof(float));
                std::cout << f << " ";
            }
            std::cout << "\n";*/
            it->program.drawInstanced(it->sprite.getTexture(),&buffer[0],buffer.size()/it->program.getRequestDataAmount()); //draw
            buffer.clear(); //clear our buffer
        }
    }
    requests.clear();
    data.clear();
}

OpaqueManager SpriteManager::opaques;
TransManager SpriteManager::trans;
void OpaqueManager::render()
{
    /*go through each sprite-program pairing and render their data. A lot simpler than TransManager because the data
    is already sorted and contiguous*/
    auto opaqueEnd = opaquesMap.end();
    int size =  opaquesMap.size();
   for (auto it = opaquesMap.begin(); it != opaqueEnd; ++it)
    {
       if (it->second.size() > 0) //render all current sprite parameters in one go, assuming there are any
       {
            int requestAmount = it->first.second.getRequestDataAmount();
            it->first.second.drawInstanced(it->first.first.getTexture(),&it->second[0],it->second.size()/(requestAmount));
            it->second.clear();
       }
    }
}

void SpriteManager::render()
{

    opaques.render();
    trans.render();
}

unsigned int PolyRender::VAO = -1;
unsigned int PolyRender::lineVBO = -1;
unsigned int PolyRender::polyVBO = -1;
unsigned int PolyRender::colorVBO = -1;
unsigned short PolyRender::restart = 65535;
std::unique_ptr<RenderProgram> PolyRender::polyRenderer;
std::vector<std::pair<glm::vec3,glm::vec4>> PolyRender::lines;
PolyStorage<glm::vec4> PolyRender::polyColors;
PolyStorage<glm::vec3> PolyRender::polyPoints;
PolyStorage<GLuint> PolyRender::polyIndices;
int PolyRender::polygonRequests = 0;
void PolyRender::init(int screenWidth, int screenHeight)
{
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&lineVBO);
    glGenBuffers(1,&colorVBO);
    glGenBuffers(1,&polyVBO);



    polyRenderer = std::make_unique<RenderProgram>("../../resources/shaders/vertex/polygonVertex.h","../../resources/shaders/fragment/simpleFragment.h");

    glPrimitiveRestartIndex(restart);
    glEnable(GL_PRIMITIVE_RESTART);

}

void PolyRender::requestLine(const glm::vec4& line, const glm::vec4& color, float z, unsigned int thickness)
{
    requestGradientLine(line,color,color,z,thickness);
}

void PolyRender::requestGradientLine(const glm::vec4& line, const glm::vec4& color1, const glm::vec4& color2, float z, unsigned int thickness)
{
    /*here, we have to draw "thickness" amount of lines, each of which are parallel translated. https://math.stackexchange.com/a/2594547
    is the full explanation.*/
    float dist = sqrt(pow(line.y - line.a,2) + pow(line.x - line.z,2)); //distance between 2 lines
    glm::vec2 perpVector = {-(line.y - line.a)/dist,(line.x - line.z)/dist}; //calculate the unit vector perpendicular to our line
    for (int i = 0; i < thickness; ++i)
    {
        glm::vec2 p1 = glm::vec2(line.x,line.y);
        glm::vec2 p2 = glm::vec2(line.z,line.a);
        if (thickness > 1) //only do fancy stuff if we have to; for thickness == 1, we don't have to use linear algebra and doing so introduces decimals that can cause inaccuracy
        {
            glm::vec2 disp = perpVector*(thickness/2.0f-i); //displacement of both points
            p1 += disp;
            p2 += disp;
        }
        lines.push_back(std::pair<glm::vec3,glm::vec4>(glm::vec3(p1,z),color1));
        lines.push_back(std::pair<glm::vec3, glm::vec4>(glm::vec3(p2,z),color2));
    }
}

void PolyRender::requestCircleSegment(float segHeight,float angle, const glm::vec4& color,const glm::vec2& center, double radius, bool filled, float z)
{
    /*std::cout << "UNIMPLEMENTED!";
    if (segHeight == 2*radius)
    {
        requestCircle(color,center,radius,filled,z);
    }
    else
    {
        glm::vec2 topPoint = center - glm::vec2(0,radius); //top most point of the circle
        float chordLength = 2*(sqrt(segHeight*(2*radius - segHeight))); //length of the chord (https://en.wikipedia.org/wiki/Circular_segment#Chord_length_and_height)
        glm::vec2 finalPoint = topPoint + glm::vec2(chordLength/2,segHeight); //we will always use this point to form a rectanble
        glm::vec2 prevPoint = topPoint + glm::vec2(-chordLength/2,segHeight); //previous point that we added to polyPoints
        float turn = M_PI/180; //cycle angle of 360-gon. Literally 1 degree

        int i = 0; //number of points we've pushed in, also coincides with the index number
        glm::vec2 temp; //used to store the previous point
        addPointAndIndex(prevPoint);
        polyColors.push_back(color);
        while (prevPoint != finalPoint)
        {
            temp = next;
            glm::vec2 nextPoint = rotatePoint(prevPoint, center,turn*((i%2)*2-1));
            prevPoint = temp;
            addPointAndIndex(nextPoint);
            polyColors.push_back(color);
        }
        polyIndices.push_back(restart);
        polygonRequests++;

        addPointAndIndex(prevPoint);
        addPointAndIndex()

        polygonRequests++;
        polyIndicies.push_back(restart);
    }*/
}

void PolyRender::requestCircle( const glm::vec4& color,const glm::vec2& center, double radius, bool filled, float z)
{
    requestNGon(360,center,radius,color,0,filled,z,true);
}

glm::vec2 floorVec(const glm::vec2& vec)
{
    return {floor(vec.x),floor(vec.y)};
}

void PolyRender::requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z)
{
    glm::vec2 topLeft = {rect.x,rect.y};
    glm::vec2 topRight = {rect.x + rect.z, rect.y};
    glm::vec2 botLeft = {rect.x,rect.y + rect.a};
    glm::vec2 botRight = {topRight.x,botLeft.y};
    if (angle != 0)
    {
        glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
        topLeft = floorVec(rotatePoint(topLeft,center,angle));
        topRight = floorVec(rotatePoint(topRight,center,angle));
        botLeft = floorVec(rotatePoint(botLeft,center,angle));
        botRight = floorVec(rotatePoint(botRight, center, angle));
    }
    if (filled)
    {
        int size = getIndiciesNumber(); //the number of indicies minus the restart indicies
        for (int i = 0;i < 4; ++i)
        {
            polyColors.push_back(color);
            polyIndices.push_back(size + i);
        }
        polyIndices.push_back(restart);
        polyPoints.push_back({topLeft.x,topLeft.y,z});
        polyPoints.push_back({topRight.x,topRight.y,z});
        polyPoints.push_back({botLeft.x,botLeft.y,z});
        polyPoints.push_back({botRight.x,botRight.y,z});
        polygonRequests++;
    }
    else
    {
        requestLine({topLeft.x,topLeft.y,topRight.x,topRight.y},color,z);
        requestLine({topLeft.x,topLeft.y,botLeft.x,botLeft.y},color,z);
        requestLine({botLeft.x,botLeft.y,botRight.x,botRight.y},color,z);
        requestLine({botRight.x,botRight.y,topRight.x,topRight.y},color,z);
    }
}

void PolyRender::requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z, bool radius)
{
    //this function divides a regular NGon into n-2 triangles so we can render them using a triangle strip
    double cycleAngle = 2.0/n*M_PI;
    if (radius)
    {
        side = 2*side*sin(M_PI/n);
        //the angle in the center of a regular polygon divided in half is always M_PI/n, we then use trigonometry to figure out the
        //length of the side given the radius
    }
    glm::vec2 first = {center.x - side/2, center.y + side/2/tan(cycleAngle/2)}; //first is the angle we start at and the point we'll be rotating around the center to generate our verticies
    if (angle != 0)
    {
        first = rotatePoint(first,center,angle);
    }
    glm::vec2 next = first; //next is the actual point we'll be adding into the polygons
   if (filled)
   {
        glm::vec2 temp;
        polyPoints.push_back({first.x,first.y,z});
        polyColors.push_back(color);
        int indicies = getIndiciesNumber(); //number of indicies minus restart indicies
        polyIndices.push_back(indicies);
        for (int i = 1; i < n; ++i)
        {
            temp = next;
            next = rotatePoint(first, center,cycleAngle*((i%2)*2-1)); //calculate the next point by rotating the previous point
            first = temp; //track the previous point
            polyPoints.push_back({next.x,next.y,z});
            polyColors.push_back(color);
            polyIndices.push_back(indicies + i);
        }
        polyIndices.push_back(restart);
        polygonRequests++;

   }
   else
   {
        for (int i = 0; i < n; ++i)
        {
            next = rotatePoint(first, center,cycleAngle);
            requestLine({first.x,first.y,next.x,next.y},color,z);
            //lines.push_back({{first.x,first.y,next.x,next.y},color},z);
            first = next;
        }
   }

}

void PolyRender::requestPolygon(const std::vector<glm::vec3>& points, const glm::vec4& color)
{
    int size = points.size() - polygonRequests;
    int indices= polyIndices.size();
    for (int i = 0; i < size; ++i)
    {
        polyPoints.push_back(points[i]);
        polyColors.push_back(color);
        polyIndices.push_back(indices +i);
    }
    polygonRequests ++;
    //polygons.push_back({size,color});
}

void PolyRender::renderLines()
{
    glBindVertexArray(VAO);
    int size = lines.size();
    int vertSize = size*3;
    int colorSize = size*4;
    size_t floatSize = sizeof(GLfloat);
    GLfloat* verticies = new GLfloat[size*3];
    GLfloat* colors = new GLfloat[size*4];
    for (int i = 0; i < size; i ++)
    {
       addPointToBuffer(verticies,lines[i].first,i*3);
       //std::cout << verticies[i*3] << " " << verticies[i*3 + 1] << " " << verticies[i*3 + 2] << std::endl;
       addPointToBuffer(colors,lines[i].second,i*4);
    }
//std::cout << glGetError() << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER,lineVBO);
    glBufferData(GL_ARRAY_BUFFER,vertSize*floatSize,verticies,GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(0);
    //glVertexAttribDivisor(0,1);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER,colorSize*floatSize, colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(1);
    //glVertexAttribDivisor(1,1);

    polyRenderer->use();

    glDrawArrays(GL_LINES,0,size);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    lines.clear();

    delete [] verticies;
    delete [] colors;
}

void PolyRender::renderPolygons()
{
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER,polyVBO);
    glBufferData(GL_ARRAY_BUFFER,polyPoints.size()*sizeof(GLfloat)*3,&polyPoints[0],GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(0);
    //glVertexAttribDivisor(0,1);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER,polyColors.size()*sizeof(GLfloat)*4, &polyColors[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(1);
    //glVertexAttribDivisor(1,1);

    polyRenderer->use();
    glDrawElements(GL_TRIANGLE_STRIP,polyIndices.size(),GL_UNSIGNED_INT,&polyIndices[0]);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);


    polyPoints.clear();
    polyColors.clear();
    polyIndices.clear();
    polygonRequests = 0;

   /* delete[] verticies;
    delete[] colors;
    delete[] indicies;*/

}

void PolyRender::renderMesh(float* mesh, int w, int h)
{

}

void PolyRender::render()
{
    if (lines.size() > 0)
    {
        renderLines();
    }
    if (polyColors.size() > 0)
    {
        renderPolygons();
    }
}

