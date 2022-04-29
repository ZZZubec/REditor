#pragma once
#include <Urho3D/Math/Vector3.h>

using namespace Urho3D;

namespace Redi {

    enum EEditorMode : unsigned
    {
        EM_SELECT, EM_MOVE, EM_EXTRUDE
    };

    enum ENormalDirection : unsigned
    {
        ND_X, ND_Y, ND_Z
    };

    enum EFigureType : unsigned
    {
        FT_TRIANGLE, FT_QUAD
    };

    struct FVertex
    {
        Urho3D::Vector3 position{ Urho3D::Vector3::ZERO };
        Urho3D::Vector3 normal{ Urho3D::Vector3::ZERO };
        Urho3D::Vector2 uv{ Urho3D::Vector2::ZERO };
    };

    struct FFace
    {
        int idx{-1};
        ea::vector<FVertex> vertices{};
    };

    struct FNormalRect
    {
        ENormalDirection direction;
        Urho3D::Vector3 min;
        Urho3D::Vector3 max;
    };
}
