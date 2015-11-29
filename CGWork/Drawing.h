#ifndef _DRAWING_H
#define _DRAWING_H

#include "Geometry.h"
#include "GeometricTransformations.h"

MatrixHomogeneous ScaleAndCenter(const BoundingBox boundingCube);
MatrixHomogeneous PerspectiveWarpMatrix(const BoundingBox boundingCube);
MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox boundingCube);

#endif