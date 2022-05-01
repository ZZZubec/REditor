#include "Figure.h"

#include <Urho3D/IO/Log.h>

#include "Math/Ray.h"

using namespace Redi;

Figure::Figure(EFigureType stype)
{
    type_ = stype;
    faces.clear();
    face_id = 0;
}

Vector3 Figure::GetFaceNormal(const ea::vector<FVertex>& vertices) const
{
    float vx1 = vertices[0].position.x_ - vertices[1].position.x_;
    float vy1 = vertices[0].position.y_ - vertices[1].position.y_;
    float vz1 = vertices[0].position.z_ - vertices[1].position.z_;
    float vx2 = vertices[1].position.x_ - vertices[2].position.x_;
    float vy2 = vertices[1].position.y_ - vertices[2].position.y_;
    float vz2 = vertices[1].position.z_ - vertices[2].position.z_;

    //Тогда векторное произведение этих векторов и будет нормаль:
    Vector3 normal;
    normal.x_ = vy1 * vz2 - vz1 * vy2;
    normal.y_ = vz1 * vx2 - vx1 * vz2;
    normal.z_ = vx1 * vy2 - vy1 * vx2;
    normal.Normalize();

    return normal;
}

void Figure::AddFace(const FVertex& v1, const FVertex& v2, const FVertex& v3, const FVertex& v4)
{
    face_id ++;
    const ea::vector<FVertex> vertices = {v1, v2, v3, v4};
    const Vector3 normal = GetFaceNormal(vertices);
    const BoundingBox bb = CalculateMinMax(vertices);
    const FFace face = FFace::CreateFace((int)face_id, vertices, normal, bb);
    faces.push_back(face);
}

void Figure::render(Urho3D::DebugRenderer* debug_renderer)
{
    for(FFace face : faces)
    {
        if(selected_faces.contains(face.idx))
        {
            debug_renderer->AddPolygon(face.vertices[0].position, face.vertices[1].position, face.vertices[2].position, face.vertices[3].position, Color(1.0f, 0.f, 0.f, 0.5f), false);
        }
        else
        {
            debug_renderer->AddPolygon(face.vertices[0].position, face.vertices[1].position, face.vertices[2].position, face.vertices[3].position, Color(0.4f, 0.4f, 0.4f, 1.0f), true);
        }
    }
}

Vector2 Figure::Cross(float a1, float b1, float c1, float a2, float b2, float c2)
{
    Vector2 pCross{Vector2::ZERO};
    pCross.x_ = (b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1);
    pCross.y_ = (a2 * c1 - a1 * c2) / (a1 * b2 - a2 * b1);

    return pCross;
}

FIntersect Figure::IntersectLine(const Vector2& pAB1, const Vector2& pAB2, const Vector3& pCD1, const Vector3& pCD2)
{
    FIntersect result;

    float a1 = pAB2.y_ - pAB1.y_;
    float b1 = pAB1.x_ - pAB2.x_;
    float c1 = -pAB1.x_ * pAB2.y_ + pAB1.y_ * pAB2.x_;

    float a2 = pCD2.y_ - pCD1.y_;
    float b2 = pCD1.x_ - pCD2.x_;
    float c2 = -pCD1.x_ * pCD2.y_ + pCD1.y_ * pCD2.x_;

    // Прямые параллельны
    if ((a1 * b2 - a2 * b1) == 0)
    {
        //"Прямые параллельны";
        if (a1 * b2 == b1 * a2 && a1 * c2 == a2 * c1 && b1 * c2 == c1 * b2)
        {
            //"Прямые совпадают";
        }

        return result;
    }

    //  --------------  Прямые пересекаются ----------------------

    // Точка пересечения прямых
    result.Coord = Cross(a1, b1, c1, a2, b2, c2);
    result.isIntersect = true;

    // Прямые перпендикулярны
    if ((a1 * a2 + b1 * b2) == 0)
    {
        //"Прямые перпендикулярны";

        return result;
    }

    return result;
}

float Figure::GetAngleBetweenPoints(const Vector3& Position1, const Vector3& ForwardVector, const Vector3& Position2)
{
    Vector3 Direction = Position2 - Position1;
    Direction.z_ = 0;
    Direction.Normalize();
    float Angle = ToDegrees(Acos(Direction.DotProduct(ForwardVector)));
    return Angle;
	
}

BoundingBox Figure::CalculateMinMax(const ea::vector<FVertex>& vertices) const
{
    Vector3 Min(vertices[0].position.x_, vertices[0].position.y_, vertices[0].position.z_);
    Vector3 Max(vertices[0].position.x_, vertices[0].position.y_, vertices[0].position.z_);
    for(unsigned i=0; i<vertices.size(); i++)
    {
        if(Min.x_ > vertices[i].position.x_)
        {
            Min.x_ = vertices[i].position.x_;
        }
        if(Min.y_ > vertices[i].position.y_)
        {
            Min.y_ = vertices[i].position.y_;
        }
        if(Min.z_ > vertices[i].position.z_)
        {
            Min.z_ = vertices[i].position.z_;
        }

        if(Max.x_ < vertices[i].position.x_)
        {
            Max.x_ = vertices[i].position.x_;
        }
        if(Max.y_ < vertices[i].position.y_)
        {
            Max.y_ = vertices[i].position.y_;
        }
        if(Max.z_ < vertices[i].position.z_)
        {
            Max.z_ = vertices[i].position.z_;
        }
    }

    const BoundingBox bb(Min, Max);
    return bb;
}

float Figure::GetDistance(const FFace& face, const Vector3& origin) const
{
    float distance = 0.f;
    for(unsigned i=0; i<face.vertices.size(); ++i)
    {
        distance += origin.DistanceToPoint(face.vertices[i].position);
    }
    return distance/(float)face.vertices.size();
}

bool Figure::TraceLine(const Vector3& CameraPosition, const Vector3& CameraDirection, const float maxDistance)
{
    Vector3 dotp = Vector3::UP;
    float v = dotp.DotProduct(CameraDirection);

    Vector3 normal(0,0,0);
    if(Abs(v) >= 0.5f)
    {
        //up or down
        if(v < 0)
        {
            normal = Vector3::UP;
        }
        else
        {
            normal = Vector3::DOWN;
        }
    }
    else
    {
        normal = Vector3::FORWARD;
    }
    //URHO3D_LOGINFO("{}", v);

    selected_faces.clear();

    Ray cameraRay(CameraPosition, CameraDirection);

    ea::vector<FFaceRay> shotFaces;

    for(unsigned i=0; i<faces.size(); ++i)
    {
        float v = cameraRay.HitDistance(faces[i].boundingBox);
        if(v > 0.1f && v < maxDistance)
        {
            //URHO3D_LOGINFO("face:{}, v:{}", faces[i].idx, v);
            const FFaceRay fRay = {i, GetDistance(faces[i], CameraPosition)};
            shotFaces.push_back(fRay);
        }
    }
    
    ea::sort(shotFaces.begin(), shotFaces.end(), FFaceRayCompare);

    if(shotFaces.size() > 1)
    {
        shotFaces.resize(1);
        selected_faces.push_back(faces[shotFaces[0].array_index].idx);
    }

    if(selected_faces.size() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
