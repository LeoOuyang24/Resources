#include <iostream>
#include <fstream>

#include <time.h>
#include <SDL.h>

#include "physics.h"
#include "render.h"
#include "SDLHelper.h"
#include "FreeTypeHelper.h"
#include "resourcesMaster.h"
#include "components.h"

int main(int args, char* argsc[])
{
    //delete ptr;
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    srand(time(NULL));

    ResourcesConfig::loadConfig();

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4);

    GLContext::init(screenWidth,screenHeight,true);

    glEnable(GL_MULTISAMPLE);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1);

    SDL_StopTextInput();

    ViewPort::init(screenWidth,screenHeight);

    FontGlobals::init(screenWidth, screenHeight);
    PolyRender::init(screenWidth,screenHeight);
    SDL_Event e;
    bool quit = false;
    glClearColor(1,1,1,1);
    bool eventsEmpty = true;

    /*RenderCamera camera;
    ViewPort::setZRange((0.1,100);
    camera.init({0,0,CAMERA_Z});
    ViewPort::currentCamera = &camera;*/

    Sprite sub("./sprites/TheSeeker.png");
    float lineVerts[36] = {0,0,0,
                            320,320,0,
                            0,640,0,

                            0,0,0,
                            320,320,0,
                            640,0,0,

                            0,640,0,
                            320,320,0,
                            640,640,0,

                            640,0,0,
                            320,320,0
                            ,640,640,0
                            };
    float colors[8] = {1,0,0,1,1,0,0,1};
    //BasicRenderPipeline line("../../resources/shaders/vertex/polygonVertex.h","../../resources/shaders/fragment/simpleFragment.h",{0,0},&lineVerts[0],3,3);
    //BasicRenderPipeline light("../../resources/shaders/vertex/polygonVertexTest.h","../../resources/shaders/fragment/simpleFragment.h",{0,1,0,1},&lineVerts[0],3,12);


    int instances = 100000;

    std::vector<char> bytes;
    bytes.reserve(instances*sizeof(TightTuple<char,short,int,float,bool>));
    int average = 0;
    int frames = 0;

    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);


    std::string date = std::to_string(now->tm_mon + 1) + "_" + std::to_string(now->tm_mday)+"_" + std::to_string(now->tm_year+1900);
    std::ofstream benchmark;
    benchmark.open("data/benchmark_"+ date +"_" + std::to_string(time(0)) + ".txt");

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            eventsEmpty = false;
            KeyManager::update(e);
            MouseManager::update(e);
            if (e.type == SDL_QUIT)
            {
                quit = true;
                std::cout << average/frames << "\n";
            }
        }
        if (eventsEmpty)
        {
            KeyManager::update(e);
            MouseManager::update(e);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        int dimen = 50;

        /*int start = SDL_GetTicks();



        for (int i = 0; i < instances; i++)
        {
            fillBytesVec(bytes,sizeof(TightTuple<char,short,int,float,bool>),'a',short(18),10,10.0f,true);
        }
        int fillBytesTime = SDL_GetTicks() - start;
        bytes.clear();

        start = SDL_GetTicks();
        for (int i = 0; i < instances; i++)
        {
            TightTuple tuple('a',short(18),10,10.0f,true);
            char* cars = reinterpret_cast<char*>(&tuple);
            bytes.insert(bytes.end(),cars, cars + sizeof(tuple));
        }
        int tightTupleTime = SDL_GetTicks() - start;

        std::cout << sizeof(TightTuple<char,short,int,float,bool>) << " " << tightTupleTime << " " << fillBytesTime << "\n";
        bytes.clear();*/

        for (int i = 0; i < instances; ++i)
        {
            //SpriteManager::requestSprite({*ViewPort::basicProgram,&sub},{i%(screenWidth/dimen)*dimen,i/(screenWidth/dimen)*dimen,dimen,dimen},0);
            SpriteManager::request({*ViewPort::basicProgram,&sub},0,true,glm::vec4{i%(screenWidth/dimen)*dimen,i/(screenWidth/dimen)*dimen,dimen,dimen},0,0,0);
        }

        //std::cout << old << " " << test << "\n";

        //int

        ViewPort::update();

        //SpriteManager::trans.renderTest(instances);
        SpriteManager::render();
        PolyRender::render();

        GLContext::update();
        eventsEmpty = true;
        DeltaTime::update();

        //UNCOMMENT TO RECORD DATA
        //benchmark << DeltaTime::deltaTime << "\n";

        average += DeltaTime::deltaTime;
        frames ++;
        //std::cout << DeltaTime::deltaTime << "\n";
        //Font::tnr.requestWrite({std::to_string(DeltaTime::deltaTime),glm::vec4(0,0,100,100),glm::vec4(1,0,0,1),0,1});

        //SDL_Delay(std::max(0,10 - DeltaTime::deltaTime));

    }
    return 0;
}
