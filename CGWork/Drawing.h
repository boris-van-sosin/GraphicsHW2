#ifndef _DRAWING_H
#define _DRAWING_H

#include "Geometry.h"
#include "GeometricTransformations.h"

struct ClippingPlane
{
	ClippingPlane(double x_, double y_, double z_, double c_);
	ClippingPlane(const Point3D& p, const Vector3D& n);
	ClippingPlane(const ClippingPlane& other);

	double Apply(const Point3D& p) const;
	double Apply(const HomogeneousPoint& p) const;
	double Apply(double x_, double y_, double z_) const;

	Point3D Intersection(const Point3D& p0, const Point3D& p1) const;

	double x, y, z, c;
};

struct PerspectiveData
{
	PerspectiveData(const MatrixHomogeneous& ms, const MatrixHomogeneous& mp, const ClippingPlane& n, const ClippingPlane& f);

	MatrixHomogeneous ScaleAndMoveToView, PerspectiveWarp;
	ClippingPlane NearPlane, FarPlane;
};

MatrixHomogeneous ScaleAndCenter(const BoundingBox& boundingCube);
PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube);
MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox& boundingCube);
PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube, double n, double f);

#endif