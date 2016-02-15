#include "DrawingSupport.h"

LightSource::LightSource()
	: _origin(Vector3D::Zero), _offset(1, 1, 1), _type(LightSource::POINT)
{
	for (int i = 0; i < 3; i++) {
		_intensity[i] = 0;
	}
}

LightSource::LightSource(const Point3D& or, const Vector3D& dir, LightSourceType t, double p)
	: LightSource(HomogeneousPoint(or), HomogeneousPoint(dir), t, p)
{
}

LightSource::LightSource(const HomogeneousPoint& or, const HomogeneousPoint& dir, LightSourceType t, double p)
	: _origin(or), _offset(Point3D(or) + (Point3D(dir).Normalized())), _type(t)
{
	for (int i = 0; i < 3; i++) {
		_intensity[i] = p;
	}
}

LightSource::LightSource(const LightSource& other)
	: _origin(other._origin), _offset(other._offset), _type(other._type)
{
	for (int i = 0; i < 3; i++) {
		_intensity[i] = other._intensity[i];
	}
}

Vector3D LightSource::Direction() const
{
	if (_type == PLANE)
		return Vector3D(_offset).Normalized();
	return (Vector3D(_origin) - Vector3D(_offset)).Normalized();
}

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
		pb = Point3D(0, 0, -c / z);
	}
	else if (y != 0)
	{
		pb = Point3D(0, 0, -c / y);
	}
	else if (x != 0)
	{
		pb = Point3D(0, 0, -c / x);
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
