#pragma once

namespace Redi {

    struct FVertex
    {
        Urho3D::Vector3 position{ Urho3D::Vector3::ZERO };
        Urho3D::Vector3 normal{ Urho3D::Vector3::ZERO };
        Urho3D::Vector2 uv{ Urho3D::Vector2::ZERO };
        FVertex(Urho3D::Vector3 pos)
        {
            position = pos;
        }
    };

    struct FFace
    {
        int idx{-1};
        ea::vector<FVertex> vertices{};
    };
    
}