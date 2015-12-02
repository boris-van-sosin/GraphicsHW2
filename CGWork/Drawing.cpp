#include "Drawing.h"

ClippingPlane::ClippingPlane(double x_, double y_, double z_, double c_)
	: x(x_), y(y_), z(z_), c(c_)
{
}

ClippingPlane::ClippingPlane(const Point3D& p, const Vector3D& n)
	: x(n.x), y(n.y), z(n.z), c(-(p * n))
{
}

ClippingPlane::ClippingPlane(const ClippingPlane& other)
	: ClippingPlane(other.x, other.y, other.z, other.c)
{
}

double ClippingPlane::Apply(const Point3D& p) const
{
	return Apply(p.x, p.y, p.z);
}

double ClippingPlane::Apply(const HomogeneousPoint& p) const
{
	return Apply(Point3D(p));
}

double ClippingPlane::Apply(double x_, double y_, double z_) const
{
	return x_ * x + y_ * y + z_ * z + c;
}

Point3D ClippingPlane::Intersection(const Point3D& p0, const Point3D& p1) const
{
	Point3D pb;
	if (z != 0)
	{
		pb = Point3D(0, 0, -c/z);
	}
	else if (y != 0)
	{
		pb = Point3D(0, 0, -c/y);
	}
	else if (x != 0)
	{
		pb = Point3D(0, 0, -c/x);
	}

	const Point3D normal(x, y, z);
	const double a = (p1 - p0)*normal;
	if (fabs(a) < GEOMETRIC_COMPUTATION_EPSILON)
	{
		return Point3D(NAN, NAN, NAN);
	}
	double d = ((pb - p0) * normal) / a;
	return (d * (p1 - p0)) + p0;
}

PerspectiveData::PerspectiveData(const MatrixHomogeneous& ms, const MatrixHomogeneous& mp, const ClippingPlane& n, const ClippingPlane& f)
	: ScaleAndMoveToView(ms), PerspectiveWarp(mp), NearPlane(n), FarPlane(f)
{
}

MatrixHomogeneous ScaleAndCenter(const BoundingBox& boundingCube)
{
	const double vright = boundingCube.maxX;
	const double vleft = boundingCube.minX;
	const double vbottom = boundingCube.minY;
	const double vtop = boundingCube.maxY;
	const double vfar = -boundingCube.maxZ;
	const double vnear = -boundingCube.minZ;
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(2 / (vright - vleft), 0, 0, -(vright + vleft) / (vright - vleft)),
		HomogeneousPoint(0, 2 / (vtop - vbottom), 0, -(vtop + vbottom) / (vtop - vbottom)),
		HomogeneousPoint(0, 0, 2 / (vnear - vfar), -(vnear + vfar) / (vfar - vnear)),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows);
}

PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube)
{
	const double depth = boundingCube.maxZ - boundingCube.minZ;
	const double clippingMargin = depth * 0.1;
	const double near2 = 1 - clippingMargin;
	const double far2 = near2 + depth + clippingMargin;
	const double q2 = far2 / (far2 - near2);

	//return MatrixHomogeneous(rows) * Matrices::Translate(0, 0, boundingCube.minZ + near + depth*0.5);
	HomogeneousPoint rows2[] = {
		HomogeneousPoint(2, 0, 0, 0),
		HomogeneousPoint(0, 2, 0, 0),
		HomogeneousPoint(0, 0, q2, 1),
		HomogeneousPoint(0, 0, -q2*near2, 0)
	};

	return PerspectiveData(
		Matrices::Translate(0, 0, 2) * ScaleAndCenter(boundingCube),
		MatrixHomogeneous(rows2),
		ClippingPlane(0, 0, 1, -near2),
		ClippingPlane(0, 0, -1, -far2));
}

MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox& boundingCube)
{
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(1, 0, 0, 0),
		HomogeneousPoint(0, 1, 0, 0),
		HomogeneousPoint(0, 0, 0, 0),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows) * ScaleAndCenter(boundingCube);
}
