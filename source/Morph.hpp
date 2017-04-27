//
//  Created by Asger Nyman Christiansen on 25/04/2017.
//  Copyright © 2017 Asger Nyman Christiansen. All rights reserved.
//

#pragma once

#include "Mesh.h"

class Morph
{
    glm::vec3 direction;
    mesh::Mesh* model;
    const mesh::VertexID* vertex = nullptr;
    
public:
    
    void start(mesh::Mesh* _model, const mesh::VertexID* _vertex)
    {
        model = _model;
        vertex = _vertex;
        direction = model->normal(vertex);
    }
    
    void update(float power)
    {
        glm::vec3 position = model->position()->at(vertex);
        model->position()->at(vertex) = position + direction * power * 0.01f;
    }
    
    void end()
    {
        vertex = nullptr;
    }
    
    bool is_morphing()
    {
        return vertex != nullptr;
    }
    
};
