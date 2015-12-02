#ifndef _DRAWING_H
#define _DRAWING_H

#include "Geometry.h"
#include "GeometricTransformations.h"

struct ClippingPlane
{
	ClippingPlane(double x_, double y_, double z_, double c_);
	ClippingPlane(const Point3D& p, const Vector3D& n);

	double Apply(const Point3D& p) const;
	double Apply(const HomogeneousPoint& p) const;
	double Apply(double x_, double y_, double z_) const;

	double x, y, z, c;
};

MatrixHomogeneous ScaleAndCenter(const BoundingBox& boundingCube);
std::pair<MatrixHomogeneous, double> PerspectiveWarpMatrix(const BoundingBox& boundingCube);
MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox& boundingCube);

#endif