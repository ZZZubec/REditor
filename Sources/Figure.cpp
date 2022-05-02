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
    const float size = 0.1f;
    for(const FFace* face = faces.begin(); face != faces.end(); ++face)
    {
        if(selected_faces.contains(face->idx))
        {
            debug_renderer->AddPolygon(face->vertices[0].position, face->vertices[1].position, face->vertices[2].position, face->vertices[3].position, Color(1.0f, 0.f, 0.f, 0.5f), false);
        }
        else
        {
            debug_renderer->AddPolygon(face->vertices[0].position, face->vertices[1].position, face->vertices[2].position, face->vertices[3].position, Color(0.4f, 0.4f, 0.4f, 1.0f), true);
        }
        debug_renderer->AddLine(face->vertices[0].position, face->vertices[1].position, Color(0.2f, 0.2f, 0.2f, 0.5f), true);
        debug_renderer->AddLine(face->vertices[1].position, face->vertices[2].position, Color(0.2f, 0.2f, 0.2f, 0.5f), true);
        debug_renderer->AddLine(face->vertices[2].position, face->vertices[3].position, Color(0.2f, 0.2f, 0.2f, 0.5f), true);
        debug_renderer->AddLine(face->vertices[3].position, face->vertices[0].position, Color(0.2f, 0.2f, 0.2f, 0.5f), true);
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

bool Figure::TraceLine(const Vector3& CameraPosition, const Vector3& CameraDirection, const float maxDistance, Vector3& hitPos)
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
        FFace face = faces[shotFaces[0].array_index];
        hitPos = Vector3(cameraRay.origin_ + cameraRay.direction_ * shotFaces[0].distance);
        //hitPos -= Vector3(cameraRay.direction_ * Abs(Vector3(hitPos - face.boundingBox.Center()).Length()));
        selected_faces.push_back(face.idx);
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

Redi::FFace* Figure::GetSelectedFace()
{
    if(selected_faces.size() > 0)
    {
        for (FFace* face = faces.begin(); face != faces.end(); ++face)
        {
            if(face->idx == selected_faces[0])
            {
                return face;
            }
        }
    }
}

Redi::EFaceDirection Figure::GetFaceDirection(FFace* face)
{
    if(face->normal.Equals(Vector3::UP))
    {
        return Redi::FD_UP;
    }
    else if(face->normal.Equals(Vector3::DOWN))
    {
        return Redi::FD_DOWN;
    }
    else if(face->normal.Equals(Vector3::LEFT))
    {
        return Redi::FD_LEFT;
    }
    else if(face->normal.Equals(Vector3::RIGHT))
    {
        return Redi::FD_RIGHT;
    }
    else if(face->normal.Equals(Vector3::FORWARD))
    {
        return Redi::FD_FORWARD;
    }
    else if(face->normal.Equals(Vector3::BACK))
    {
        return Redi::FD_BACK;
    }
    return Redi::FD_NONE;
}

Redi::EFaceDirection Figure::InvertFaceDirection(EFaceDirection eDirection)
{
    switch (eDirection)
    {
    default:
        return FD_NONE;
    case FD_UP:
        return FD_DOWN;
    case FD_DOWN:
        return FD_UP;
    case FD_LEFT:
        return FD_RIGHT;
    case FD_RIGHT:
        return FD_LEFT;
    case FD_FORWARD:
        return FD_BACK;
    case FD_BACK:
        return FD_FORWARD;
    }
}

Urho3D::Vector3 Figure::GetVector3(EFaceDirection eDirection)
{
    switch (eDirection)
    {
    default:
        return Vector3::ZERO;
    case FD_UP:
        return Vector3::UP;
    case FD_DOWN:
        return Vector3::DOWN;
    case FD_LEFT:
        return Vector3::LEFT;
    case FD_RIGHT:
        return Vector3::RIGHT;
    case FD_FORWARD:
        return Vector3::FORWARD;
    case FD_BACK:
        return Vector3::BACK;
    }
}

void Figure::MoveFace(unsigned idx, const Vector3& offset)
{
    for(const FFace* oldFace = faces.begin(); oldFace != faces.end(); ++oldFace)
    {
        if(oldFace->idx == idx)
        {
            ea::vector<FVertex> n_vertices;
            for(unsigned j=0; j<oldFace->vertices.size(); ++j)
            {
                const FVertex vertex{Vector3(oldFace->vertices[j].position + offset), oldFace->vertices[j].normal, oldFace->vertices[j].uv};
                n_vertices.push_back(vertex);
            }

            if(selected_faces.contains(oldFace->idx))
            {
                selected_faces.clear();
            }

            const BoundingBox bb = CalculateMinMax(n_vertices);
            Redi::FFace nFace = FFace::CreateFace(oldFace->idx, n_vertices, Vector3(oldFace->normal), bb);

            faces.erase(oldFace);
            faces.push_back(nFace);
            break;
        }
    }
}
