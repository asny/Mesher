//
//  Created by Asger Nyman Christiansen on 18/12/2016.
//  Copyright © 2016 Asger Nyman Christiansen. All rights reserved.
//

#include "GLCamera.h"
#include "MeshCreator.h"
#include "materials/GLFlatColorMaterial.h"
#include "materials/GLColorMaterial.h"
#include "materials/GLSkyboxMaterial.h"
#include "effects/GLDebugEffect.h"
#include "effects/GLAmbientOcclusionEffect.h"
#include "Search.h"
#include "Morph.hpp"

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
    xaxis->transform(glm::scale(mat4(1.), vec3(2., 0.05, 0.05)));
    root.add_leaf(xaxis, make_shared<GLColorMaterial>(vec3(1., 0., 0.)));
    
    auto yaxis = MeshCreator::create_box(false);
    yaxis->transform(glm::translate(mat4(1.), vec3(0., 1., 0.)));
    yaxis->transform(glm::scale(mat4(1.), vec3(0.05, 2., 0.05)));
    root.add_leaf(yaxis, make_shared<GLColorMaterial>(vec3(0., 1., 0.)));
    
    auto zaxis = MeshCreator::create_box(false);
    zaxis->transform(glm::translate(mat4(1.), vec3(0., 0., 1.)));
    zaxis->transform(glm::scale(mat4(1.), vec3(0.05, 0.05, 2.)));
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
    
    root.add_leaf(model, color_material);
    
    // Create box
    auto box = MeshCreator::create_box(true);
    auto box_material = make_shared<GLFlatColorMaterial>(vec3(0.9, 0.9, 0.9));
    root.add_child(make_shared<GLScaleNode>(5.))->add_leaf(box, box_material);
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
    directional_light->direction = normalize(vec3(0.5, -1., -0.5));
    auto directional_light2 = make_shared<GLDirectionalLight>();
    scene.add_light(directional_light2);
    directional_light2->direction = normalize(vec3(-0.5, -1., 0.5));
    create_scene(scene);
    create_axes(scene);
    
    camera.set_view(vec3(2., 2., 2.), vec3(-1., -1., -1.));
    
    // Create debug effect
    auto debug_effect = GLDebugEffect();
    auto ssao = GLAmbientOcclusionEffect();
    
    // run while the window is open
    bool quit = false;
    bool mouse_rotation = false;
    bool ssao_enabled = true;
    Morph morph;
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
            if( e.key.state == SDL_PRESSED && e.key.keysym.sym == SDLK_m)
            {
                ssao_enabled = !ssao_enabled;
            }
            if( e.type == SDL_MOUSEWHEEL)
            {
                auto position = camera.get_position() + static_cast<float>(e.wheel.y) * camera.get_direction();
                camera.set_view(position, camera.get_direction());
            }
            if( e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
            {
                auto view_ray_origin = camera.get_position();
                auto view_ray_direction = camera.get_view_direction_at(e.button.x, e.button.y);
                auto result = Search::closest_face(*model, view_ray_origin, view_ray_direction);
                if(result.face_id)
                {
                    morph.start(model.get(), result.point, result.face_id->v1());
                }
                else {
                    mouse_rotation = true;
                }
            }
            if( e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
            {
                mouse_rotation = false;
                morph.end();
            }
            if(mouse_rotation && e.type == SDL_MOUSEMOTION)
            {
                glm::vec3 direction = camera.get_direction();
                glm::vec3 up_direction = glm::vec3(0., 1., 0.);
                glm::vec3 right_direction = glm::cross(direction, up_direction);
                up_direction = glm::cross(right_direction, direction);
                glm::vec3 camera_position = camera.get_position();
                float zoom = length(camera_position);
                camera_position += 0.1f * (static_cast<float>(e.motion.xrel) * right_direction + static_cast<float>(e.motion.yrel) * up_direction);
                camera_position = zoom * normalize(camera_position);
                camera.set_view(camera_position, glm::normalize(-camera_position));
            }
            if(morph.is_morphing() && e.type == SDL_MOUSEMOTION)
            {
                morph.update(static_cast<float>(e.motion.yrel));
            }
        }
        
        // update the scene based on the time elapsed since last update
        update();
        
        // draw one frame
        camera.draw(scene);
        if(ssao_enabled)
            camera.apply_post_effect(ssao);
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
