#pragma once

namespace Redi {

    struct FVertex
    {
        Urho3D::Vector3 position;
        Urho3D::Vector3 normal;
        Urho3D::Vector2 uv;
    };

    struct FFace
    {
        ea::vector<FVertex> vertices;
    };
    
}