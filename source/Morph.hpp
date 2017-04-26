//
//  Created by Asger Nyman Christiansen on 25/04/2017.
//  Copyright © 2017 Asger Nyman Christiansen. All rights reserved.
//

#pragma once

#include "Mesh.h"

class Morph {
    
    
public:
    
    static void apply(mesh::Mesh& mesh, const mesh::VertexID* vertex, float value)
    {
        glm::vec3 pos = mesh.position()->at(vertex);
        mesh.position()->at(vertex) = pos + mesh.normal(vertex) * value;
    }
    
};
