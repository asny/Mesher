//
//  Created by Asger Nyman Christiansen on 25/04/2017.
//  Copyright © 2017 Asger Nyman Christiansen. All rights reserved.
//

#pragma once

#include "Mesh.h"

class Morph
{
    glm::vec3 direction;
    mesh::Mesh *model;
    std::shared_ptr<mesh::Attribute<mesh::VertexID, float>> weights = std::make_shared<mesh::Attribute<mesh::VertexID, float>>();
    const float RADIUS = 1.;
    
    void update_weights(const glm::vec3& origin, const mesh::VertexID* vertex)
    {
        if(!weights->contains(vertex))
        {
            glm::vec3 position = model->position()->at(vertex);
            auto dist = glm::distance(position, origin);
            if(dist < RADIUS)
            {
                weights->at(vertex) = RADIUS - dist;
                
                for (auto face : vertex->faces())
                {
                    update_weights(origin, face->v1());
                    update_weights(origin, face->v2());
                    update_weights(origin, face->v3());
                }
            }
        }
    }
    
public:
    
    void start(mesh::Mesh* _model, const mesh::VertexID* vertex)
    {
        model = _model;
        direction = model->normal(vertex);
        update_weights(model->position()->at(vertex), vertex);
    }
    
    void update(float power)
    {
        for(auto vertex = model->vertices_begin(); vertex != model->vertices_end(); vertex = vertex->next())
        {
            float weight = weights->at(vertex);
            glm::vec3 position = model->position()->at(vertex);
            model->position()->at(vertex) = position + weight * power * 0.01f * direction;
        }
    }
    
    void end()
    {
        model = nullptr;
        weights->clear();
    }
    
    bool is_morphing()
    {
        return weights->size() != 0;
    }
    
};
