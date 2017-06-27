//
//  Created by Asger Nyman Christiansen on 18/12/2016.
//  Copyright © 2016 Asger Nyman Christiansen. All rights reserved.
//

#include "MeshCreator.h"
#include "SurfaceMeasures.h"
#include "materials/GLFlatColorMaterial.h"
#include "materials/GLColorMaterial.h"
#include "materials/GLSkyboxMaterial.h"
#include "materials/GLWireframeMaterial.h"
#include "effects/GLDebugEffect.h"
#include "effects/GLAmbientOcclusionEffect.h"
#include "GLEventHandler.h"
#include "Morph.hpp"

#define SDL_MAIN_HANDLED
#include "SDL.h"

using namespace std;
using namespace glm;
using namespace gle;
using namespace mesh;

shared_ptr<Mesh> model;
shared_ptr<bool> wireframe_enabled = make_shared<bool>(true);

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
    auto wireframe_material = make_shared<GLWireframeMaterial>(vec3(0.9, 0.1, 0.1));
    
    model = make_shared<Mesh>();
    MeshCreator::load_from_obj("../Models/bunny.obj", *model);
    auto min_pos = min_coordinates(*model);
    auto max_pos = max_coordinates(*model);
    auto size = max_pos - min_pos;
    auto center = min_pos + 0.5f * size;
    model->transform(glm::translate(glm::mat4(1.f), -center));
    float scale = 5.f/std::max(std::max(size.x, size.y), size.z);
    model->transform(glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, scale)));
    
    root.add_child(make_shared<GLSwitchNode>(wireframe_enabled))->add_leaf(model, wireframe_material);
    root.add_leaf(model, color_material);
    
    // Create box
    auto box = MeshCreator::create_box(true);
    box->transform(glm::scale(glm::mat4(1.f), glm::vec3(5., 5., 5.)));
    box->transform(glm::translate(glm::mat4(1.f), glm::vec3(0., 2.5, 0.)));
    auto box_material = make_shared<GLFlatColorMaterial>(vec3(0.9, 0.9, 0.9));
    root.add_leaf(box, box_material);
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
    directional_light->diffuse_intensity = 0.2f;
    auto directional_light2 = make_shared<GLDirectionalLight>();
    scene.add_light(directional_light2);
    directional_light2->direction = normalize(vec3(-0.5, -1., 0.5));
    directional_light2->diffuse_intensity = 0.2f;
    create_scene(scene);
    create_axes(scene);
    
    camera.set_view(vec3(7., 7., 7.), vec3(-1., -1., -1.));
    
    // Create debug effect
    auto debug_effect = GLDebugEffect();
    auto ssao = GLAmbientOcclusionEffect();
    
    // run while the window is open
    bool quit = false;
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
            GLEventHandler::show_debug(e, debug_effect);
            if( e.key.state == SDL_PRESSED && e.key.keysym.sym == SDLK_m)
            {
                ssao_enabled = !ssao_enabled;
            }
            if( e.key.state == SDL_PRESSED && e.key.keysym.sym == SDLK_w)
            {
                *wireframe_enabled = !*wireframe_enabled;
            }
            
            if(morph.is_morphing())
            {
                if( e.type == SDL_MOUSEMOTION )
                {
                    morph.update(static_cast<float>(e.motion.yrel));
                    scene.invalidate(model);
                }
                if( e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT )
                {
                    morph.end();
                }
            }
            else {
                auto result = GLEventHandler::picking(e, camera, *model);
                if(result.is_a_hit())
                {
                    morph.start(model.get(), result.point, result.face_id->v1());
                }
                else {
                    GLEventHandler::navigate_spherical(e, camera);
                }
            }
        }
        
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
