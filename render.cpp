#include <sstream>
#include <fstream>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "SDL.h"

#include "geometry.h"
#include "render.h"
#include "SDLhelper.h"

void drawRectangle(RenderProgram& program, const glm::vec3& color, const glm::vec4& rect, double angle)
{
    glm::vec2 center = {rect.x+rect.z/2,rect.y+rect.a/2};
    glm::vec2 topL = rotatePoint({rect.x,rect.y},center,angle);
    glm::vec2 topR = rotatePoint({rect.x+rect.z,rect.y},center,angle);
    glm::vec2 botL = rotatePoint({rect.x,rect.y+rect.a},center,angle);
    glm::vec2 botR = rotatePoint({rect.x+rect.z,rect.y+rect.a},center,angle);
       drawLine(program,color,{{topL.x,topL.y,topR.x,topR.y},
                                {topL.x, topL.y, botL.x, botL.y},
                                {topR.x,topR.y,botR.x,botR.y},
                                {botL.x, botL.y, botR.x, botR.y}});
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

void drawNGon(RenderProgram& program, const glm::vec3& color, const glm::vec2& center, double radius, int n, double angle)
{
    if (n > 2)
    {
        //glm::vec2 firstVertex = {center.x -radius*cos(angle),center.y + radius*sin(angle)};
        {
            int vertNum = (n*3); //basically the number of points
            float buffer[vertNum];
            double subAngle = 2*M_PI/n;
            double angleFar = std::fmod((M_PI/2 - subAngle/2),subAngle) + angle; //angle from the x-axis of the furthest left point.
            glm::vec2 firstVertex = {center.x - cos(angleFar)*radius, center.y + sin(angleFar)*radius}; //we subtract horizontally because we want the furthest left
            addPointToBuffer(buffer,firstVertex,0);
            addPointToBuffer(buffer,rotatePoint(firstVertex,center,-subAngle),3);
          for (int i = 2; i < n; ++i)
            {
                int modI = 3*i;
                //addPointToBuffer(buffer,rotatePoint({buffer[modI - 6],buffer[modI - 5]},center,((i%2*-2)+1)*subAngle),modI);
                addPointToBuffer(buffer,rotatePoint({buffer[modI - 3],buffer[modI - 2]},center,-subAngle),modI);
            }
            glBindVertexArray(RenderProgram::VAO);
    glBindBuffer(GL_ARRAY_BUFFER,RenderProgram::VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(buffer),buffer,GL_STATIC_DRAW);
     glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );
    glEnableVertexAttribArray(0);
        program.setVec3fv("color",color);
        program.use();
        glDrawArrays(GL_POLYGON,0,n);
        }

    }
}
void drawCircle(RenderProgram& program, glm::vec3 color,double x, double y, double radius)
{
    GLfloat data[360*3];
    double convert = 2*M_PI/360;
    for (int i = 0; i < 360; i ++)
    {
        addPointToBuffer(data,{x + cos(i*convert)*radius, y + sin(i*convert)*radius},i*3);
    }
    glBindBuffer(GL_ARRAY_BUFFER,RenderProgram::VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(data),data,GL_STATIC_DRAW);
     glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
    glEnableVertexAttribArray(0);
    program.setVec3fv("color",color);
    program.use();
    glDrawArrays(GL_LINE_LOOP,0,360);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void drawLine(RenderProgram& program, glm::vec3 color, const std::vector<glm::vec4>& points)
{
    int size = points.size();
    GLfloat buffer_data[size*6];
    for (int i = 0;i < size; i ++)
    {
        addPointToBuffer(buffer_data,{points[i].x,points[i].y},i*6);
        addPointToBuffer(buffer_data,{points[i].z,points[i].a},i*6+3);
    }

glBindBuffer(GL_ARRAY_BUFFER,RenderProgram::VBO);
long length = sizeof(buffer_data);
glBufferData(GL_ARRAY_BUFFER,length,buffer_data,GL_STATIC_DRAW);
//std::cout << glGetError() << std::endl;
 glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
glEnableVertexAttribArray(0);
    program.setVec3fv("color",color);
    program.use();
    glDrawArrays(GL_LINES,0,size*2);
    glBindBuffer(GL_ARRAY_BUFFER,0);


}
int loadShaders(const GLchar* source, GLenum shaderType )
{
    std::ifstream input;
    input.open(source);
    int shader = -1;
    if (input.is_open())
    {
        std::stringstream stream;
        stream << input.rdbuf();
         shader = glCreateShader(shaderType);
         std::string str = stream.str();
        const char* code = str.c_str();
        glShaderSource(shader,1, &code, NULL);
        glCompileShader(shader);
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "Error loading shader: " << infoLog << std::endl;
        }
    }
    else
    {
        std::cout << "Can't find shader! Source: " << source << std::endl;
    }
    return shader;

}

int RenderProgram::screenWidth = 0;
int RenderProgram::screenHeight = 0;
RenderProgram RenderProgram::basicProgram;
RenderProgram RenderProgram::lineProgram;
RenderProgram RenderProgram::paintProgram;
ViewRange RenderProgram::baseRange;
ViewRange RenderProgram::currentRange;
RenderProgram::RenderProgram(std::string vertexPath, std::string fragmentPath)
{
    init(vertexPath,fragmentPath);
}
void RenderProgram::init(std::string vertexPath, std::string fragmentPath)
{
    GLuint fragment = -1, vertex= -1;
    program = glCreateProgram();
    vertex = loadShaders(vertexPath.c_str(), GL_VERTEX_SHADER );
    fragment = loadShaders(fragmentPath.c_str(),GL_FRAGMENT_SHADER);
    glAttachShader(program,vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(fragment);
    glDeleteShader(vertex);
}
GLuint RenderProgram::VBO, RenderProgram::VAO;
void RenderProgram::init(int screenWidth, int screenHeight)
{
    RenderProgram::screenWidth = screenWidth;
    RenderProgram::screenHeight = screenHeight;



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
        {-10.0f,10.0f}     //magic numbers. Can be anything
    };
    currentRange = baseRange;

glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);
    glClearColor(1,1,1,1);

    RenderProgram::lineProgram.init("../../resources/shaders/vertex/simpleVertex.h","../../resources/shaders/fragment/simpleFragment.h");
    glm::mat4 mat = getOrtho();
    lineProgram.setMatrix4fv("projection",glm::value_ptr(mat));
    RenderProgram::basicProgram.init("../../resources/shaders/vertex/vertexShader.h","../../resources/shaders/fragment/fragmentShader.h");
    basicProgram.setMatrix4fv("projection",glm::value_ptr(mat));
    RenderProgram::paintProgram.init("../../resources/shaders/vertex/vertexShader.h","../../resources/shaders/fragment/paintShader.h");
    paintProgram.setMatrix4fv("projection",glm::value_ptr(mat));
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);


}

glm::vec2 RenderProgram::toAbsolute(const glm::vec2& point)
{
    double ratio = ViewRange::getXRange(currentRange)/ViewRange::getXRange(baseRange);
    return {point.x*ViewRange::getXRange(currentRange)/ViewRange::getXRange(baseRange),point.y*ViewRange::getYRange(currentRange)/ViewRange::getYRange(baseRange)};
}
glm::vec4 RenderProgram::toAbsolute(const glm::vec4& rect)
{
    return glm::vec4(toAbsolute({rect.x,rect.y}),toAbsolute({rect.z,rect.a}));
}

const glm::vec2& RenderProgram::getXRange()
{
    return currentRange.xRange;
}
const glm::vec2& RenderProgram::getYRange()
{
    return currentRange.yRange;
}
const glm::vec2& RenderProgram::getZRange()
{
    return currentRange.zRange;
}
void RenderProgram::setXRange(float x1, float x2)
{
    currentRange.xRange.x = x1;
    currentRange.xRange.y = x2;
}
void RenderProgram::setYRange(float y1, float y2)
{
    currentRange.yRange.x = y1;
    currentRange.yRange.y = y2;
}
void RenderProgram::setZRange(float z1, float z2)
{
    currentRange.zRange.x = z1;
    currentRange.zRange.y = z2;
}

void RenderProgram::resetRange()
{
    currentRange = baseRange;
}

glm::mat4 RenderProgram::getOrtho()
{
    return (glm::ortho(currentRange.xRange.x, currentRange.xRange.y, currentRange.yRange.y, currentRange.yRange.x, currentRange.zRange.x, currentRange.zRange.y));
}

glm::vec2 RenderProgram::getScreenDimen()
{
    return {screenWidth,screenHeight};
}

void RenderProgram::use()
{
        glm::mat4 mat = RenderProgram::getOrtho();
       setMatrix4fv("projection",glm::value_ptr(mat));
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

RenderCamera* RenderCamera::currentCamera = nullptr;

void RenderCamera::init(int w, int h)
{
    rect.z = w;
    rect.a = h;
}

const glm::vec4& RenderCamera::getRect() const
{
    return rect;
}

void RenderCamera::recenter(const glm::vec2& point)
{
    rect.x = point.x - rect.z/2;
    rect.y = point.y - rect.a/2;
}


glm::vec4 RenderCamera::toScreen(const glm::vec4& change) const
{
    glm::vec2 point = toScreen({change.x,change.y});
    return {point.x,point.y,change.z, change.a};
}

glm::vec2 RenderCamera::toScreen(const glm::vec2& point) const
{

    return {(point.x - rect.x), (point.y - rect.y)};
}

glm::vec4 RenderCamera::toWorld(const glm::vec4& change) const
{

    glm::vec2 point = toWorld({change.x,change.y});
    return {point.x,point.y , change.z, change.a};
}

glm::vec2 RenderCamera::toWorld(const glm::vec2& point) const
{
     glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    return {(point.x/screenDimen.x*rect.z + rect.x), (point.y/screenDimen.y*rect.a + rect.y)};
}

glm::vec4 RenderCamera::toAbsolute(const glm::vec4& rect) const
{
    return glm::vec4(toAbsolute({rect.x,rect.y}),toAbsolute({rect.z,rect.a}));
}
glm::vec2 RenderCamera::toAbsolute(const glm::vec2& point) const
{
     //glm::vec2 screenDimen = RenderProgram::getScreenDimen();
     //double horiz = (RenderProgram::getXRange().y - RenderProgram::getXRange().x);
  //   std::cout << horiz << " " << rect.z << std::endl;
     //double vert = ViewRange::getYRange(RenderProgram::getYRange());
    //return {point.x*horiz/screenDimen.x,point.y*rect.a/screenDimen.y};
    return RenderProgram::toAbsolute(point);
}


const int Sprite::floats = 26;
const size_t Sprite::floatSize = sizeof(float);
   void Sprite::load(std::string source)
    {

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1,&VBO);
        glGenBuffers(1,&modVBO);

        glBindVertexArray(VAO);

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
            transparent = true;
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
        loadVertices();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
    }

Sprite::Sprite( std::string source)
    {
        init(source);
    }
    Sprite::~Sprite()
    {

        glDeleteTextures(1,&texture);
    }
void Sprite::setTint(glm::vec3 color)
{
    tint = color;
}
void Sprite::init(std::string source)
    {
        load(source);
    }
void Sprite::loadVertices()
{
           //glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(values),values, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
        defaultVerticies = true;

}
void Sprite::loadVertices(const std::vector<float>& vert)
{
        int size = vert.size();
        float verticies[size];
        for (int i = 0; i < size; i ++)
        {
            verticies[i] = vert[i];
        }
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticies),verticies, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
        defaultVerticies = false;

}
void Sprite::reset()
{
    glBindVertexArray(0);
glBindBuffer(GL_ARRAY_BUFFER,0);
/*modified.clear();
    for (int i = 0; i < 16; i ++)
    {
        modified.push_back(values[i]);
    }
modIndices.clear();
    for (int i = 0; i < 6; i ++)
    {
        modIndices.push_back(indices[i]);
    }*/
    tint = {1,1,1};
}

template<class T>
void Sprite::loadBuffer(unsigned int& buffers,int location, T arr[], int size, int dataSize, int divisor)
{
glGenBuffers(1,&buffers);
glBindBuffer(GL_ARRAY_BUFFER, buffers);
glBufferData(GL_ARRAY_BUFFER, sizeof(*arr)*size, arr, GL_STATIC_DRAW);
glEnableVertexAttribArray(location);
glVertexAttribPointer(location,dataSize,GL_FLOAT,GL_FALSE,0,0);
glVertexAttribDivisor(location,divisor);

//glDeleteBuffers(1,&buffers);
}

void Sprite::loadData(GLfloat* data, const SpriteParameter& parameter, int index)
{
    if (data!= nullptr)
    {
      //  GLsizei stride = (floatSize*floats); //space between everything. We are passing a 4x4 matrix, a vec4, a vec3, and a single float
            const SpriteParameter* current = &parameter;
          //  std::cout << parameter.rect.x << std::endl;
            glm::mat4 matt = glm::mat4(1.0f);
            matt = glm::translate(matt,{current->rect.x + (current->rect.z)/2,current->rect.y + (current->rect.a)/2,0}); //scaling messes with the position of the object. If the object is being rendered to a size of 2x2, there is no reason to counteract the scaling.
            matt = glm::rotate(matt, current->radians, glm::vec3(0,0,1));
            matt = glm::scale(matt, {current->rect.z/2, current->rect.a/2,1});
           const float *pSource = (const float*)glm::value_ptr(matt);
            for (int j = 0; j < 16; j++)
            {
                data[j+index] =pSource[j]; //copy matrix
            }
            data[index + 16]= current->effect;
            data[index + 16 + 1] = current->tint.x;
            data[index + 16 + 2] =  current->tint.y;
            data[index + 16 + 3] = current->tint.z;
            data[index + 16 + 4] = current->tint.a;
            data[index + 20 + 1] = current->z;
            data[index + 20 + 2] = current->portion.x;
            data[index + 20 + 3] = current->portion.y;
            data[index + 20 + 4] = current->portion.z;
            data[index + 20 + 5] = current->portion.a;
           // std::cout << data[index + 20+ 1]<<"\n";
           /* int vertAmount = current->indices.size();
            for (int j = 0; j < vertAmount; j ++)
            {
                for (int z = 0; z < 4; z++)
                {
                    vec.push_back(current->vertices[current->indices[j]*4+z]);
                }
            }*/
        }
        else
        {
            throw new std::invalid_argument("null buffer");
        }
    }

void Sprite::draw(RenderProgram& program, GLfloat* data, int instances)
{
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D,texture);
    //loadVertices();
    glBindBuffer(GL_ARRAY_BUFFER,modVBO);
    GLsizei vec4Size = 4*floatSize;
    int stride = floatSize*floats;
    glBufferData(GL_ARRAY_BUFFER,stride*instances,data,GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)0); //3-6 inclusive are the transformation matrix
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(vec4Size));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)(2 * vec4Size));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * vec4Size));
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, stride, (void*)(4*vec4Size)); //effect
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4*vec4Size + floatSize)); //color
    glVertexAttribPointer(9, 1,GL_FLOAT, GL_FALSE, stride, (void*)((floats-5)*floatSize)); //z
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, stride, (void*)((floats-4)*floatSize)); //portion
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);
    glEnableVertexAttribArray(9);
    glEnableVertexAttribArray(10);

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glVertexAttribDivisor(9, 1);
    glVertexAttribDivisor(10, 1);

    program.use();
    glDrawElementsInstanced(GL_TRIANGLES,6,GL_UNSIGNED_INT,indices,instances);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    reset();

}

void Sprite::renderInstanced(RenderProgram& program, const std::vector<SpriteParameter>& parameters)
{
    double stride = floatSize*floats;
    unsigned int size = parameters.size();
    GLfloat* data = new GLfloat[getFloats()*size];
   // std::cout << size << std::endl;
    for (int i = 0; i < size; i ++)
    {
        loadData(data, parameters[i],i*floats);
    }
    draw(program,data, size);
    delete[] data;
}

unsigned int Sprite::getVAO()
{
    return VAO;
}

glm::vec2 Sprite::getDimen()
{
    return {width,height};
}

int Sprite::getFloats()
{
    return floats;
}


const int Sprite9::floats9 = Sprite::floats*9;
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

int Sprite9::getFloats()
{
    return floats9;
}

void Sprite9::loadData(GLfloat* data, const SpriteParameter& parameter, int index)
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
                                   current->radians,NONE,current->tint,current->program,parameter.z,{1.0/3*h,(1.0/3)*g,1.0/3,1.0/3}}, index + (g*3+h)*floats);
            }
        }
}


BaseAnimation::BaseAnimation(std::string source, int speed, int perRow, int rows, const glm::vec4& sub)
{
    init(source,speed,perRow,rows,  sub);
}

int BaseAnimation::getFrames()
{
    return subSection.z*subSection.a;
}

int BaseAnimation::getDuration(int speed)
{
    if (speed == -1)
    {
        speed = fps;
    }
    return 1000/speed*subSection.z*subSection.a;
}

glm::vec2 BaseAnimation::getDimen()
{
    return {frameDimen.x*width,frameDimen.y*height};
}

glm::vec4 BaseAnimation::getPortion(const AnimationParameter& param)
{
        //int current =  SDL_GetTicks();
        glm::vec4 backup = subSection;
        if (param.subSection.z != 0 || param.subSection.a != 0)
        {
            subSection = param.subSection;
        }
        int perRow = subSection.z; //frame per rows
        int rows = subSection.a; //number of rows
        int framesSince = param.start;
        if (param.start < 0 )
        {
            framesSince = ((param.fps == -1)*fps + (param.fps != -1)*param.fps)*(SDL_GetTicks() + param.start)/1000.0; //frames that have passed
        }

        glm::vec4 answer = {frameDimen.x*(framesSince%(perRow)) + subSection.x,
        (frameDimen.y*((framesSince/perRow)%rows)) + subSection.y,frameDimen.x, frameDimen.y};

        subSection = backup;

        return answer;
}

SpriteParameter BaseAnimation::processParam(const SpriteParameter& sParam,const AnimationParameter& aParam)
{
    /*Returns a SpriteParameter that represents what to render. sParam.portion is interpreted as the portion of the sprite sheet to render*/
    SpriteParameter param = sParam;
    if (aParam.transform && aParam.camera)
    {
        param.rect = ((aParam.camera)->*(aParam.transform))(param.rect);
    }
    //double timeSince = current - ptr->start;
   // int framesSince = ((ptr->fps == -1)*fps + (ptr->fps != -1)*ptr->fps)*timeSince; //frames that have passed

    glm::vec4 portion = getPortion(aParam);
    param.portion = {param.portion.x + portion.x, param.portion.y + portion.y, param.portion.z*portion.z, param.portion.a*portion.a};
    return param;
}

void BaseAnimation::init(std::string source, int speed, int perRow, int rows, const glm::vec4& sub)
{
    Sprite::init(source);
    fps = speed;
    frameDimen.x = 1.0/(perRow);
    frameDimen.y = 1.0/(rows);
    subSection = sub;
    if (sub.z == 0)
    {
        subSection.z = perRow;
    }
    if (sub.a == 0)
    {
        subSection.a = rows;
    }
}
void BaseAnimation::renderInstanced(RenderProgram& program, const std::list<FullAnimationParameter>& parameters)
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
        param.portion = getPortion({SDL_GetTicks(),-1});
        params.push_back({param});
    }
    Sprite::renderInstanced(program, params);
}

std::vector<SpriteWrapper*> SpriteManager::sprites;
std::map<zWrapper,std::list<SpriteParameter>,SpriteManager::ZWrapperComparator> SpriteManager::params;
void SpriteWrapper::init(std::string source)
{
    spr = new Sprite(source);
    SpriteManager::addSprite(*this);
}

void SpriteWrapper::init(Sprite* sprite)
{
    spr = sprite;
    SpriteManager::addSprite(*this);
}

void SpriteWrapper::request(const SpriteParameter& param)
{
    SpriteManager::request(*this,param);
}

void SpriteWrapper::reset()
{
  //  parameters.clear();
}

void SpriteWrapper::render(const std::list<SpriteParameter>& parameters, float zMod)
{
    if (spr)
    {
        auto end = parameters.end();
        int size = parameters.size();
      //GLsizei vec4Size = sizeof(glm::vec4);
        int floats = spr->getFloats();
        GLfloat* data = new GLfloat[size*floats];
        glBindVertexArray(spr->VAO);
        glBindTexture(GL_TEXTURE_2D,spr->texture);
        glBindBuffer(GL_ARRAY_BUFFER,spr->modVBO);
        int index = 0;
        int i = 0;
       // bool deleted = false;
       SpriteParameter current;

        for (auto it = parameters.begin(); it != end; ++it)
        {
            current = *it;
           // current.z += zMod + SpriteManager::zIncrement*(float)i/size;
          //  std::cout << current.z << "\n";
            spr->loadData(data, current, index*floats);
            index ++;
            if (i == size - 1 || ((std::next(it))->program != current.program ) )
            {
                spr->draw(*(current.program),data,(index)*floats/spr->floats); //floats/spr->floats = # of sprite Parameters per sprite Parameter. This is most relevant for Sprite9, where each SpriteParameter passed results in 8 more Sprite Parameters
                index = 0;
                delete[] data;
                if (i != size - 1)
                {
                    data = new GLfloat[(size-i-1)*floats];
                   // deleted = true;
                }
            }
            i ++;
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        spr->reset();
    }
}

Sprite* SpriteWrapper::getSprite()
{
    return spr;
}

glm::vec2 SpriteWrapper::getDimen()
{
    if (!isReady())
    {
        throw std::logic_error("Tried to get dimensions of uninitialized SpriteWrapper!");
    }
    return spr->getDimen();
}

bool SpriteWrapper::isReady()
{
    return spr;
}

SpriteWrapper::~SpriteWrapper()
{
    reset();
    delete spr;
}

BaseAnimation* AnimationWrapper::getAnimation()
{
    return static_cast<BaseAnimation*>(spr);
}

void AnimationWrapper::init(BaseAnimation* a)
{
    SpriteWrapper::init(a);
}
void AnimationWrapper::reset()
{
    auto end = aParameters.end();
    for (auto i = aParameters.begin(); i != end;)
    {
        if (i->second.repeat <= 0)
        {
           i= aParameters.erase(i);
        }
        else
        {
            glm::vec4 portion = static_cast<BaseAnimation*>(spr)->getPortion(i->second);
            if (portion.x + portion.z == 1 && portion.y + portion.a == 1)
            {
                i->second.repeat --;
            }
            ++i;
        }
    }
    SpriteWrapper::reset();
}

void AnimationWrapper::request(const SpriteParameter& param)
{
    request(param,{});
}

void AnimationWrapper::request(const SpriteParameter& sParam, const AnimationParameter& aParam)
{
   SpriteWrapper::request(static_cast<BaseAnimation*>(spr)->processParam(sParam,aParam));
}
AnimationWrapper::~AnimationWrapper()
{
    reset();
}

void SpriteManager::addSprite(SpriteWrapper& spr)
{
    sprites.push_back(&spr);
}

void SpriteManager::request(SpriteWrapper& wrapper, const SpriteParameter& param)
{
    params[{param.z,&wrapper}].push_back(param);
}

void SpriteManager::render()
{
    int i= 0;
    auto end = params.end();
    bool shouldBeEnabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
    if (shouldBeEnabled)
    {
        glDisable(GL_DEPTH_TEST);
    }
    for (auto it = params.begin(); it != end; ++it)
    {
       it->first.second->render(it->second,0);
       ++i;
    }
    if (shouldBeEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    params.clear();
    /*int size = sprites.size();
    for (int i = 0; i < size; i ++)
    {
        sprites[i]->render();
        sprites[i]->reset();
    }*/
}

unsigned int PolyRender::VAO = -1;
unsigned int PolyRender::lineVBO = -1;
unsigned int PolyRender::polyVBO = -1;
unsigned int PolyRender::colorVBO = -1;
unsigned short PolyRender::restart = 65535;
RenderProgram PolyRender::polyRenderer;
std::vector<std::pair<glm::vec3,glm::vec4>> PolyRender::lines;
std::vector<std::pair<int,glm::vec4>> PolyRender::polygons;
std::vector<glm::vec3> PolyRender::polyPoints;
void PolyRender::init(int screenWidth, int screenHeight)
{
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&lineVBO);
    glGenBuffers(1,&colorVBO);
    glGenBuffers(1,&polyVBO);



    polyRenderer.init("../../resources/shaders/vertex/polygonVertex.h","../../resources/shaders/fragment/simpleFragment.h");

    glPrimitiveRestartIndex(restart);
    glEnable(GL_PRIMITIVE_RESTART);

}

void PolyRender::requestLine(const glm::vec4& line, const glm::vec4& color, float z, unsigned int thickness, RenderCamera* camera)
{
    requestGradientLine(line,color,color,z,thickness,camera);
}

void PolyRender::requestGradientLine(const glm::vec4& line, const glm::vec4& color1, const glm::vec4& color2, float z, unsigned int thickness, RenderCamera* camera)
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
        if (camera)
        {
            p1 = camera->toScreen(p1);
            p2 = camera->toScreen(p2);
        }
        lines.push_back(std::pair<glm::vec3,glm::vec4>(glm::vec3(p1,z),color1));
        lines.push_back(std::pair<glm::vec3, glm::vec4>(glm::vec3(p2,z),color2));
    }
}
void PolyRender::requestCircle( const glm::vec4& color,const glm::vec2& center, double radius, bool filled, float z)
{
    requestNGon(360,center,radius,color,0,filled,z,true);
}

glm::vec2 floorVec(const glm::vec2& vec) //shortcut function for requestRect
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
        polygons.push_back({4,color});
        polyPoints.push_back({topLeft.x,topLeft.y,z});
        polyPoints.push_back({topRight.x,topRight.y,z});
        polyPoints.push_back({botLeft.x,botLeft.y,z});
        polyPoints.push_back({botRight.x,botRight.y,z});
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
        for (int i = 1; i < n; ++i)
        {
            temp = next;
            next = rotatePoint(first, center,cycleAngle*((i%2)*2-1));
            first = temp;
            polyPoints.push_back({next.x,next.y,z});
        }
        polygons.push_back({n,color});
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
    int size = points.size();
    for (int i = 0; i < size; ++i)
    {
        polyPoints.push_back(points[i]);
    }
    polygons.push_back({size,color});
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
    glBufferData(GL_ARRAY_BUFFER,vertSize*floatSize,verticies,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(0);
    //glVertexAttribDivisor(0,1);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER,colorSize*floatSize, colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(1);
    //glVertexAttribDivisor(1,1);

    polyRenderer.use();

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
    int size = polygons.size();
    int pointsSize = polyPoints.size();
    long vertSize  = pointsSize*3;
    long colorSize = pointsSize*4;
    long indSize = pointsSize + size;
    GLfloat* verticies = new GLfloat[vertSize];
    GLfloat* colors = new GLfloat[colorSize];
    GLuint* indicies = new GLuint[indSize];
    int index = 0;
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < polygons[i].first; ++j)
        {
            addPointToBuffer(verticies,polyPoints[index + j], (index+j)*3);
            addPointToBuffer(colors,polygons[i].second,(index + j)*4);
            indicies[index + i + j] = index + j;
        }
        index += polygons[i].first;
        indicies[index + i] = restart;
    }
    glBindBuffer(GL_ARRAY_BUFFER,polyVBO);
    glBufferData(GL_ARRAY_BUFFER,vertSize*sizeof(GLfloat),verticies,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(0);
    //glVertexAttribDivisor(0,1);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER,colorSize*sizeof(GLfloat), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(1);
    //glVertexAttribDivisor(1,1);

    polyRenderer.use();
    glDrawElements(GL_TRIANGLE_STRIP,indSize,GL_UNSIGNED_INT,indicies);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);


    polyPoints.clear();
    polygons.clear();

    delete[] verticies;
    delete[] colors;
    delete[] indicies;

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
    if (polygons.size() > 0)
    {
        renderPolygons();
    }
}

int AnimationSequencer::getStateIndex(int frameStart,int* timeSince_)
{
    int timeSince = (SDL_GetTicks() - frameStart)%fullDuration; //normalize our time since our animation started
    int index = infoSize - 1;
    for (int i = 0; i < infoSize; ++i)
    {
      //  std::cout << info[index].x << " " << timeSince << "\n";
        if (timeSince - info[i].x <= 0)
        {
            index = std::max(i,0);
            break;
        }
        timeSince -= info[i].x;
    }
    if (timeSince_)
    {
        *timeSince_ = timeSince;
    }
    return index;
}

AnimationSequencer::AnimationSequencer(const std::vector<glm::vec2>& baseInfo)
{
    info = new glm::vec3[baseInfo.size()];
    infoSize = baseInfo.size();
    for (int i = 0; i < infoSize; ++i)
    {
        info[i] = {baseInfo[i].x,baseInfo[i].y,totalFrames};
        fullDuration += baseInfo[i].x;
        totalFrames += baseInfo[i].y;
    }
}

AnimationParameter AnimationSequencer::process(int frameStart)
{
    //std::cout << (SDL_GetTicks() - frameStart) << "\n";
    int timeSince = 0;
    int index = getStateIndex(frameStart,&timeSince);
    AnimationParameter param;
    param.fps = 1000.0*info[index].y/info[index].x;
    param.start = fmod((timeSince*info[index].y/info[index].x + info[index].z),totalFrames);
    //std::cout << timeSince << " " << param.start << " " << index << "\n";
    return param;
}

int AnimationSequencer::getStateIndex(int frameStart)
{
    if (isDone(frameStart))
    {
        return -1;
    }
    return getStateIndex(frameStart,nullptr);
}

bool AnimationSequencer::isDone(int frameStart)
{
    return SDL_GetTicks() - frameStart >= fullDuration;
}

AnimationSequencer::~AnimationSequencer()
{

}

