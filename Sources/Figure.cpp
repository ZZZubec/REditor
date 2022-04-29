#include "Figure.h"

Redi::Figure::Figure(EFigureType stype)
{
    type_ = stype;
    faces.clear();
    face_id = 0;
}

void Redi::Figure::AddFace(const FVertex& v1, const FVertex& v2, const FVertex& v3, const FVertex& v4)
{
    face_id ++;
    FFace face = {(int)face_id, {v1, v2, v3, v4}};
    faces.push_back(face);
}
