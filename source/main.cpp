//
//  Created by Asger Nyman Christiansen on 18/12/2016.
//  Copyright © 2016 Asger Nyman Christiansen. All rights reserved.
//

#include "GLCamera.h"
#include "MeshCreator.h"
#include "materials/GLFlatColorMaterial.h"
#include "materials/GLColorMaterial.h"
#include "materials/GLStandardMaterial.h"
#include "effects/GLDebugEffect.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"

using namespace std;
using namespace glm;
using namespace gle;
using namespace mesh;

shared_ptr<Mesh> model;

void update()
{
    static float last_time = time();
    float current_time = time();
    float elapsed_time = current_time - last_time;
    
    last_time = current_time;
}

void create_axes(GLScene& root)
{
    auto xaxis = MeshCreator::create_box(false);
    xaxis->transform(glm::translate(mat4(1.), vec3(1., 0., 0.)));
    xaxis->transform(glm::scale(mat4(1.), vec3(5., 0.05, 0.05)));
    root.add_leaf(xaxis, make_shared<GLColorMaterial>(vec3(1., 0., 0.)));
    
    auto yaxis = MeshCreator::create_box(false);
    yaxis->transform(glm::translate(mat4(1.), vec3(0., 1., 0.)));
    yaxis->transform(glm::scale(mat4(1.), vec3(0.05, 5., 0.05)));
    root.add_leaf(yaxis, make_shared<GLColorMaterial>(vec3(0., 1., 0.)));
    
    auto zaxis = MeshCreator::create_box(false);
    zaxis->transform(glm::translate(mat4(1.), vec3(0., 0., 1.)));
    zaxis->transform(glm::scale(mat4(1.), vec3(0.05, 0.05, 5.)));
    root.add_leaf(zaxis, make_shared<GLColorMaterial>(vec3(0., 0., 1.)));
}

void create_scene(GLScene& root)
{
    auto flat_material = make_shared<GLFlatColorMaterial>(vec3(0.5, 0.1, 0.7));
    auto color_material = make_shared<GLColorMaterial>(vec3(0.5, 0.1, 0.7));
    
    model = make_shared<Mesh>();
    MeshCreator::load_from_obj("bunny.obj", *model);
    model->transform(glm::scale(glm::mat4(1.f), glm::vec3(10.)));
    model->transform(glm::translate(glm::mat4(1.f), -model->center()));
    
    root.add_leaf(model, flat_material);
}

int main(int argc, const char * argv[])
{
    // Initialize SDL
    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        throw std::runtime_error("SDL init failed");
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    // Create window
    int window_width = 1200;
    int window_height = 700;
    auto window = SDL_CreateWindow( "Hello world", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE );
    if( window == NULL )
    {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        throw std::runtime_error("SDL init failed");
    }
    
    // Create context
    auto glcontext = SDL_GL_CreateContext(window);
    
    // Create camera
    auto camera = GLCamera(window_width, window_height);
    
    // Create scene
    auto scene = GLScene();
    auto directional_light = make_shared<GLDirectionalLight>();
    scene.add_light(directional_light);
    directional_light->direction = glm::vec3(1., -1., 0.);
    create_scene(scene);
    create_axes(scene);
    
    camera.set_view(vec3(0., 0., 10.), vec3(0., 0., -1.));
    
    // Create debug effect
    auto debug_effect = GLDebugEffect();
    
    // run while the window is open
    bool quit = false;
    while(!quit)
    {
        // process pending events
        SDL_Event e;
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
            {
                quit = true;
            }
            if( e.key.keysym.sym == SDLK_0)
                debug_effect.type = gle::GLDebugEffect::NONE;
            if( e.key.keysym.sym == SDLK_1)
                debug_effect.type = gle::GLDebugEffect::POSITION;
            if( e.key.keysym.sym == SDLK_2)
                debug_effect.type = gle::GLDebugEffect::NORMAL;
            if( e.key.keysym.sym == SDLK_3)
                debug_effect.type = gle::GLDebugEffect::COLOR;
            if( e.type == SDL_MOUSEWHEEL)
            {
                camera.zoom(e.wheel.y);
            }
        }
        
        // update the scene based on the time elapsed since last update
        update();
        
        // draw one frame
        camera.draw(scene);
        camera.apply_post_effect(debug_effect);
        
        SDL_GL_SwapWindow(window);
    }
    
    // Delete context
    SDL_GL_DeleteContext(glcontext);
    
    // Destroy window
    SDL_DestroyWindow( window );
    window = NULL;
    
    // Quit SDL subsystems
    SDL_Quit();
    return 0;
}
