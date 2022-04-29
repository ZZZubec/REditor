#pragma once
#include "Structures.h"
#include "EASTL/vector.h"

namespace Redi
{
    
class Figure
{
public:
    Figure(EFigureType stype);
    ~Figure();

private:
    unsigned face_id;
    EFigureType type_;

public:
    ea::vector<FFace> faces;

    void AddFace(const FVertex& v1, const FVertex& v2, const FVertex& v3, const FVertex& v4);
};
    
}
