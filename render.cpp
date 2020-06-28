#include <sstream>
#include <fstream>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "SDL.h"

#include "render.h"

double pointDistance(const glm::vec2& v1, const glm::vec2& v2)
{
    return sqrt(pow(v1.x - v2.x,2) + pow(v1.y - v2.y,2));
}

glm::vec2 findMidpoint(const glm::vec4& v1)
{
    return {(v1.x + v1.z)/2, (v1.y + v1.a)/2};
}

bool vecIntersect(const glm::vec4& vec1,const glm::vec4& vec2)
{
    if (vec1.z < 0 || vec1.a < 0 || vec2.z < 0 || vec2.a < 0) //if a dimension is negative, we will have to rearrange where the corners are
    {
        glm::vec4 rect1 = vec1, rect2 = vec2;
        if (vec1.z < 0)
        {
            rect1.x += vec1.z;
            rect1.z *= -1;
        }
        if (vec1.a < 0)
        {
            rect1.y += vec1.a;
            rect1.a *= -1;
        }
        if (vec2.z < 0)
        {
            rect2.x += vec2.z;
            rect2.z *= -1;
        }
        if (vec2.a < 0)
        {
            rect2.y += vec2.a;
            rect2.a *= -1;
        }
        return vecIntersect(rect1,rect2);
    }
    return (vec1.x <= vec2.x + vec2.z && vec1.x + vec1.z>= vec2.x && vec1.y <= vec2.y + vec2.a && vec1.y + vec1.a >= vec2.y);
}

glm::vec4 vecIntersectRegion(const glm::vec4& vec1, const glm::vec4& vec2) //returns the region of two colliding rects
{
    glm::vec4 answer= {0,0,0,0};

    if (vec1.y + vec1.a >= vec2.y && vec1.y <= vec2.y + vec2.a)
    {
        answer.y = std::max(vec1.y, vec2.y);
        answer.a = std::min(vec2.y + vec2.a, vec1.y + vec1.a) - answer.y;
    }
    if (vec1.x + vec1.z >= vec2.x && vec1.x <= vec2.x + vec2.z)
    {
        answer.x = std::max(vec1.x, vec2.x);
        answer.z = std::min(vec2.x + vec2.z, vec1.x + vec1.z) - answer.x;
    }
    return answer;
}


double vecDistance(const glm::vec4& rect1, const glm::vec4& rect2)
{
    glm::vec2 comp = {0,0}; //the horizontal and vertical components of the distance
    glm::vec4 region = vecIntersectRegion(rect1,rect2);
    if (region.z == 0) //there is no horizontal overlap. This means that the rect are left and right of each other but with no intersection
    {
        double right = std::min(rect1.x + rect1.z, rect2.x + rect2.z);
        double left =  std::max(rect1.x, rect2.x);
        comp.x = std::max(right,left) - std::min(right,left);
    }
    if (region.a == 0) //same as before but for vertical overlap
    {
        double down = std::min(rect1.y + rect1.a, rect2.y + rect2.a);
        double up =  std::max(rect1.y, rect2.y);
        comp.y = std::max(up,down) - std::min(up,down);
    }
    return sqrt(pow(comp.x,2) + pow(comp.y,2));
}

bool pointInVec(const glm::vec4& vec1, double x, double y, double angle)
{
    glm::vec2 center = {vec1.x + vec1.z/2, vec1.y+vec1.a/2};
    glm::vec2 rotated = rotatePoint({x,y},center,-angle);
    return rotated.x >= vec1.x && rotated.x <= vec1.x + vec1.z && rotated.y >= vec1.y && rotated.y <= vec1.y +vec1.a;
}

double pointVecDistance(const glm::vec4& vec, float x, float y)
{
    return sqrt(pow(std::min(vec.x + vec.z,std::max(vec.x, x)) - x,2) + pow(std::min(vec.y + vec.a,std::max(vec.y, y)) - y,2));
}

glm::vec2 closestPointOnVec(const glm::vec4& vec, const glm::vec2& point) //returns the point on vec that is the closest distance to point. Returns point if point is in vec
{
    return {std::min(vec.x + vec.z,std::max(vec.x, point.x)),std::min(vec.y + vec.a,std::max(vec.y, point.y))};
}

glm::vec2 rotatePoint(const glm::vec2& p, const glm::vec2& rotateAround, double angle)
{
    glm::vec2 point = {p.x - rotateAround.x,p.y-rotateAround.y};//distances between target and pivot point
    return {point.x*cos(angle)-point.y*sin(angle)+rotateAround.x, point.x*sin(angle) + point.y*cos(angle)+rotateAround.y};
}

bool inLine(const glm::vec2& point, const glm::vec2& point1, const glm::vec2& point2) //line 1 and line 2 are the points of the line
{
    if (point1.x == point2.x)
    {
        return point1.x == point.x && point.y >= std::min(point1.y,point2.y) && point.y <= std::max(point1.y,point2.y);
    }
    double slope = (point2.y - point1.y)/(point2.x - point1.x);
    double yInt = point1.y - slope*point1.x;
    return slope*point.x + yInt == point.y && point.x >= std::min(point1.x,point2.x) && point.x <= std::max(point1.x,point2.x);
}

double pointLineDistance(const glm::vec4& line, const glm::vec2& point)
{
    glm::vec2 a = {line.x,line.y};
    glm::vec2 b = {line.z,line.a};
    double abDist = (pointDistance(a,b));
    double bpDist = (pointDistance(b,point));
    double apDist = pointDistance(a,point);
    double bAngle =  acos((pow(apDist,2) - ( pow(abDist,2)+pow(bpDist,2) ))/(-2*abDist*bpDist)); //angle pba
    double aAngle = acos((pow(bpDist,2) - ( pow(abDist,2)+pow(apDist,2) ))/(-2*abDist*apDist)); //angle pab
    if (aAngle > M_PI/2 || bAngle > M_PI/2) //if either angle is obtuse, the shortest distance is the distance between point and one of the endpoints
    {
        return std::min(bpDist,apDist);
    }
    return sin(bAngle)*bpDist;

}

bool lineInLine(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2)
{
    glm::vec2 intersect = {0,0};
    glm::vec2 nonVert = {0,0}; //this equals the slope and yInt of the line that is not vertical, or line b1-b2 if neither are vertical.
    if (a1.x - a2.x == 0 && b1.x - b2.x == 0)
    {
        double highest = std::min(a1.y,a2.y);
        return a1.x == b1.x && abs(highest - std::min(b1.y,b2.y)) >= abs((highest - (a1.y + a2.y - highest)));
    }
    else
    {
        double slope1 = 0;
        double yInt1 = 0;
        if (a1.x - a2.x != 0) //if not vertical line
        {
            slope1 = (a1.y - a2.y)/(a1.x-a2.x);
            yInt1 = a1.y - slope1*a1.x;
            nonVert.x = slope1;
            nonVert.y = yInt1;
        }
        else
        {
            intersect.x = a1.x;
        }
        double slope2 = 0;
        double yInt2 = 0;
        if (b1.x - b2.x != 0)
        {
            slope2 = (b1.y - b2.y)/(b1.x - b2.x);
            yInt2 = b1.y - slope2*b1.x;
            nonVert.x = slope2;
            nonVert.y = yInt2;
        }
        else
        {
            intersect.x = b1.x;
        }
        if (b1.x != b2.x && a1.x != a2.x)
        {
            if (slope1 == slope2)
            {
                if (yInt1 == yInt2)
                {
                    double left = std::min(a1.x, a2.x);
                    double right = std::max(a1.x, a2.x);
                    return (b1.x <= right && b1.x >= left) || (b2.x <= right && b2.x >= left);
                }
                return false;
            }
                intersect.x = (yInt1 - yInt2)/(slope2 - slope1);
        }
    }
    intersect.y = nonVert.x*intersect.x + nonVert.y;
    return intersect.x >= std::min(a1.x, a2.x) && intersect.x <= std::max(a1.x, a2.x) &&
            intersect.x >= std::min(b1.x, b2.x) && intersect.x <= std::max(b1.x,b2.x) &&
            intersect.y >= std::min(a1.y, a2.y) - .001 && intersect.y <= std::max(a1.y, a2.y) + .001 && //rounding errors lol
            intersect.y >= std::min(b1.y, b2.y) -.001 && intersect.y <= std::max(b1.y, b2.y) + .001;
}

glm::vec2 lineLineIntersect(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2)
{
    glm::vec2 intersect = {0,0};
    glm::vec2 nonVert = {0,0}; //this equals the slope and yInt of the line that is not vertical, or line b1-b2 if neither are vertical.
     if (lineInLine(a1,a2,b1,b2))
     {
         if (a1.x == a2.x && b1.x == b2.x)
         {
             intersect = a1;
         }
         else
         {
             double slope1 = 0;
             if (b1.x == b2.x)
             {
                 slope1 = (a1.y - a2.y)/(a1.x - a2.x);
                 intersect.x = b1.x;
                 intersect.y = slope1*b1.x + a1.y - slope1*a1.x;
             }
             else if (a1.x == a2.x)
             {
                 slope1 = (b1.y - b2.y)/(b1.x - b2.x);
                 intersect.x = a1.x;
                 intersect.y = slope1*a1.x + b1.y - slope1*b1.x;    //if both lines are the same, this should return a1.y
             }
             else //neigther is vertical
             {
                 slope1 = (a1.y - a2.y)/(a1.x - a2.x);
                 double slope2 = (b1.y - b2.y)/(b1.x - b2.x);
                 double yInt1 = a1.y - slope1*a1.x;
                 double yInt2 = b1.y - slope2*b1.x;
                 intersect.x = (yInt1 - yInt2)/(slope2 - slope1);
                 intersect.y = intersect.x*slope1 + yInt1;
             }
         }
     }
    return intersect;
}

bool lineInVec(const glm::vec2& point1,const glm::vec2& point2, const glm::vec4& r1, double angle) //given points p1 and p2, with p1 having the lesser x value, this draws a line between the 2 points and checks to
{                                        //see if that line intersects with any of the sides of r1.
    if (point1.x > point2.x)
    {
        return lineInVec(point2,point1,r1,angle);
    }
    glm::vec2 center = {r1.x + r1.z/2, r1.y + r1.a/2};

    glm::vec2 p1 = rotatePoint(point1,center,-angle);
    glm::vec2 p2 = rotatePoint(point2,center,-angle);

    glm::vec2 topLeft = {r1.x, r1.y};
    glm::vec2 topRight = {r1.x + r1.z, r1.y};
    glm::vec2 botLeft = {r1.x, r1.y + r1.a};
    glm::vec2 botRight = {r1.x + r1.z, r1.y + r1.a};

    return lineInLine(topLeft,topRight,p1,p2) || lineInLine(topRight, botRight, p1, p2) ||
            lineInLine(topLeft, botLeft, p1, p2) || lineInLine(botLeft, botRight, p1, p2);
}

bool vecContains(glm::vec4 r1, glm::vec4 r2)
{
    return (r1.x >= r2.x && r1.x + r1.z <= r2.x + r2.z && r1.y >= r2.y && r1.y + r1.a <= r2.y + r2.a);
}

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

void printRect(const glm::vec4& rect)
{
    std::cout << rect.x << " " << rect.y << " " << rect.z << " " << rect.a << std::endl;
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
glm::vec2 RenderProgram::xRange = {0,0};
glm::vec2 RenderProgram::yRange = {0,0};
glm::vec2 RenderProgram::zRange = {0,0};
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE); //set version
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

    glewExperimental = true;

    GLenum err=glewInit();
      if(err!=GLEW_OK) {
        // Problem: glewInit failed, something is seriously wrong.
        std::cout << "glewInit failed: " << glewGetErrorString(err) << std::endl;
      }


    xRange = {0,screenWidth};
    yRange = {0,screenHeight};
    zRange = {-10,10}; //magic numbers. Can be anything

glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE);
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

const glm::vec2& RenderProgram::getXRange()
{
    return xRange;
}
const glm::vec2& RenderProgram::getYRange()
{
    return yRange;
}
const glm::vec2& RenderProgram::getZRange()
{
    return zRange;
}
void RenderProgram::setXRange(float x1, float x2)
{
    xRange.x = x1;
    xRange.y = x2;
}
void RenderProgram::setYRange(float y1, float y2)
{
    yRange.x = y1;
    yRange.y = y2;
}
void RenderProgram::setZRange(float z1, float z2)
{
    zRange.x = z1;
    zRange.y = z2;
}

glm::mat4 RenderProgram::getOrtho()
{
    return (glm::ortho(xRange.x, xRange.y, yRange.y, yRange.x, zRange.x, zRange.y));
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

const int Sprite::floats = 26;
const int Sprite::floatSize = sizeof(float);
   void Sprite::load(std::string source,bool transparent)
    {

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1,&VBO);
        glGenBuffers(1,&modVBO);

        glBindVertexArray(VAO);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D,texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        int channels;

        unsigned char* data = stbi_load(source.c_str(),&width, &height, &channels, 0);

        int rgb = GL_RGB*!transparent + GL_RGBA*transparent;
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

Sprite::Sprite( std::string source, bool transparent)
    {
        init(source,transparent);
    }
    Sprite::~Sprite()
    {

        glDeleteTextures(1,&texture);
    }
void Sprite::setTint(glm::vec3 color)
{
    tint = color;
}
void Sprite::init(std::string source, bool transparent)
    {
        load(source, transparent);
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
            glm::mat4 matt = glm::mat4(1.0f);
            matt = glm::translate(matt,{(int)current->rect.x + (current->rect.z*(current->rect.z != 2))/2,(int)current->rect.y + (current->rect.a*(current->rect.a != 2))/2,0}); //scaling messes with the position of the object. If the object is being rendered to a size of 2x2, there is no reason to counteract the scaling.
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
    delete data;
    }
  /*  else
    {
        std::cout << "Haven't loaded a texture yet!" << std::endl;
    }*/

int Sprite::getFloats()
{
    return floats;
}

unsigned int Sprite::getVAO()
{
    return VAO;
}
const int Sprite9::floats9 = Sprite::floats*9;
Sprite9::Sprite9(std::string source, bool transparent, glm::vec2 W, glm::vec2 H) : Sprite(source, transparent)
{
    widths = W;
    heights = H;
}
void Sprite9::init(std::string source, bool transparent, glm::vec2 W, glm::vec2 H)
{
    Sprite::init(source, transparent);
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

BaseAnimation::BaseAnimation(std::string source, bool transparent, double speed, int perRow, int rows, const glm::vec4& sub)
{
    init(source,transparent,speed,perRow,rows,  sub);
}
void BaseAnimation::init(std::string source, bool transparent, double speed, int perRow, int rows, const glm::vec4& sub)
{
    Sprite::init(source, transparent);
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
void BaseAnimation::renderInstanced(RenderProgram& program, const std::vector<AnimationParameter>& parameters)
{
    int size = parameters.size();
    std::vector<SpriteParameter> param;
    int perRow = subSection.z; //frame per rows
    int rows = subSection.a; //number of rows
    double current =  SDL_GetTicks();
    for (int i = 0; i < size; i ++)
    {
        const AnimationParameter* ptr = &parameters[i];
        double timeSince = current - ptr->start;
        int framesSince = ((ptr->fps == -1)*fps + (ptr->fps != -1)*ptr->fps)*timeSince; //frames that have passed
        param.push_back({ptr->rect,ptr->radians,ptr->effect,ptr->tint,&RenderProgram::basicProgram,ptr->z,{frameDimen.x*(framesSince%(perRow)) + subSection.x,
                                                            (frameDimen.y*((framesSince/perRow)%rows)) + subSection.y,frameDimen.x, frameDimen.y}});
    }
    Sprite::renderInstanced(program, param);
}

std::vector<SpriteWrapper*> SpriteManager::sprites;
void SpriteWrapper::init(std::string source, bool transparent)
{
    spr = new Sprite(source, transparent);
    SpriteManager::addSprite(*this);
}

void SpriteWrapper::init(Sprite* sprite)
{
    spr = sprite;
    SpriteManager::addSprite(*this);
}

void SpriteWrapper::request(SpriteParameter&& param)
{
    parameters.push_back({param});
}

void SpriteWrapper::reset()
{
    parameters.clear();
}

SpriteWrapper::~SpriteWrapper()
{
    reset();
    delete spr;
}

void SpriteWrapper::render()
{
    if (spr)
    {
        int size = parameters.size();
      //GLsizei vec4Size = sizeof(glm::vec4);
        int floats = spr->getFloats();
        GLfloat* data = new GLfloat[size*floats];
        glBindVertexArray(spr->VAO);
        glBindTexture(GL_TEXTURE_2D,spr->texture);
        glBindBuffer(GL_ARRAY_BUFFER,spr->modVBO);
        int index = 0;
       // bool deleted = false;
        for (int i = 0; i < size; i ++)
        {
            const SpriteParameter* current = &(parameters[i]);
            spr->loadData(data, *current, index*floats);
            index ++;
            if (i == size - 1 || (parameters[i+1].program != current->program ) )
            {
                spr->draw(*(current->program),data,(index)*floats/spr->floats); //floats/spr->floats = # of sprite Parameters per sprite Parameter. This is most relevant for Sprite9, where each SpriteParameter passed results in 8 more Sprite Parameters
                index = 0;
                delete[] data;
                if (i != size - 1)
                {
                    data = new GLfloat[(size-i-1)*floats];
                   // deleted = true;
                }
            }
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        spr->reset();
    }
}
void AnimationWrapper::init(BaseAnimation* a)
{
    SpriteWrapper::init(a);
}
void AnimationWrapper::reset()
{
    aParameters.clear();
    SpriteWrapper::reset();
}
void AnimationWrapper::render()
{
    if (aParameters.size() > 0)
    {
          BaseAnimation* ptr = static_cast<BaseAnimation*>(spr);
            ptr->renderInstanced(RenderProgram::basicProgram,aParameters);
    }
}
void AnimationWrapper::request(AnimationParameter&& param)
{
    aParameters.push_back(param);
}
AnimationWrapper::~AnimationWrapper()
{
    reset();
}

void SpriteManager::addSprite(SpriteWrapper& spr)
{
    sprites.push_back(&spr);
}

void SpriteManager::render()
{
    int size = sprites.size();
    for (int i = 0; i < size; i ++)
    {
        sprites[i]->render();
        sprites[i]->reset();
    }
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

void PolyRender::requestLine(const glm::vec4& line, const glm::vec4& color, float z)
{
    lines.push_back(std::pair<glm::vec3,glm::vec4>({line.x,line.y,z},color));
    lines.push_back(std::pair<glm::vec3, glm::vec4>({line.z,line.a,z},color));
}
void PolyRender::requestCircle( const glm::vec4& color,double x, double y, double radius)
{
    double convert = 2*M_PI/360;
    for (int i = 0; i < 360; i ++)
    {
        requestLine({x + cos(i*convert)*radius, y + sin(i*convert)*radius, x + cos((i-1)*convert)*radius, y + sin((i-1)*convert)*radius},color);
    }
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
        topLeft = rotatePoint(topLeft,center,angle);
        topRight = rotatePoint(topRight,center,angle);
        botLeft = rotatePoint(botLeft,center,angle);
        botRight = rotatePoint(botRight, center, angle);
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

void PolyRender::requestNGon(int n, const glm::vec2& center, double side, const glm::vec4& color, double angle, bool filled, float z)
{
    //this function divides a regular NGon into n-2 triangles so we can render them using a triangle strip
    double cycleAngle = 2.0/n*M_PI;
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
    GLfloat verticies[size*3];
    GLfloat colors[size*4];
    for (int i = 0; i < size; i ++)
    {
       addPointToBuffer(verticies,lines[i].first,i*3);
       //std::cout << verticies[i*3] << " " << verticies[i*3 + 1] << " " << verticies[i*3 + 2] << std::endl;
       addPointToBuffer(colors,lines[i].second,i*4);
    }
//std::cout << glGetError() << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER,lineVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verticies),verticies,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(0);
    //glVertexAttribDivisor(0,1);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,(void*)0);
    glEnableVertexAttribArray(1);
    //glVertexAttribDivisor(1,1);

    polyRenderer.use();

    glDrawArrays(GL_LINES,0,size);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    lines.clear();
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

