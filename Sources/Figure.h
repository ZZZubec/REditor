#pragma once
#include "Structures.h"
#include "EASTL/vector.h"
#include "Graphics/DebugRenderer.h"

namespace Redi
{
    
    using namespace Urho3D;
    
class Figure
{
public:
    Figure(EFigureType stype);
    ~Figure();

private:
    unsigned face_id;
    EFigureType type_;

    ea::vector<unsigned> selected_faces;

public:
    ea::vector<FFace> faces;

    Vector3 GetFaceNormal(const ea::vector<FVertex>& vertices) const;
    
    void AddFace(const FVertex& v1, const FVertex& v2, const FVertex& v3, const FVertex& v4);

    void render(DebugRenderer* debug_renderer);
    Vector2 Cross(float a1, float b1, float c1, float a2, float b2, float c2);
    FIntersect IntersectLine(const Vector2& pAB1, const Vector2& pAB2, const Vector3& pCD1, const Vector3& pCD2);
    float GetAngleBetweenPoints(const Vector3& Position1, const Vector3& ForwardVector, const Vector3& Position2);
    BoundingBox CalculateMinMax(const ea::vector<FVertex>& vertices) const;

    float GetDistance(const FFace& face, const Vector3& origin) const;
    bool TraceLine(const Vector3& CameraPosition, const Vector3& CameraDirection, const float maxDistance);
};

inline bool FFaceRayCompare(const FFaceRay& lhs, const FFaceRay& rhs)
{
    return lhs.distance < rhs.distance;
}
    
}
