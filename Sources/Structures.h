#pragma once
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/BoundingBox.h>

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

    enum EFaceDirection : unsigned
    {
        FD_NONE, FD_FORWARD, FD_BACK, FD_LEFT, FD_RIGHT, FD_UP, FD_DOWN
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
        Urho3D::Vector3 normal{Urho3D::Vector3::ZERO};
        Urho3D::BoundingBox boundingBox{0.f,0.f};
        static FFace CreateFace(unsigned vFaceIndex, const ea::vector<FVertex>& verts, const Urho3D::Vector3& norm, const Urho3D::BoundingBox& bb)
        {
            FFace vFace;
            vFace.idx = vFaceIndex;
            vFace.vertices = verts;
            vFace.normal = norm;
            vFace.boundingBox = bb;
            return vFace;
        }
    };

    struct FFaceRay
    {
        unsigned array_index;
        float distance;
    };

    struct FNormalRect
    {
        ENormalDirection direction;
        Urho3D::Vector3 min;
        Urho3D::Vector3 max;
    };

    struct FIntersect
    {
        bool isIntersect{false};
        Urho3D::Vector2 Coord{Urho3D::Vector2::ZERO};
    
    };
}
