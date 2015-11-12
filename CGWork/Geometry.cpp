#include "Geometry.h"
#include <math.h>

Point3D::Point3D()
	: x(0.0), y(0.0), z(0.0)
{
}

Point3D::Point3D(double x_, double y_, double z_)
	: x(x_), y(y_), z(z_)
{
}

Point3D::Point3D(const Point3D& other)
	: Point3D(other.x, other.y, other.z)
{
}

Point3D Point3D::operator + (const Point3D& other) const
{
	return Point3D(x + other.x, y + other.y, z + other.z);
}

Point3D Point3D::operator-(const Point3D& other) const
{
	return Point3D(x - other.x, y - other.y, z - other.z);
}

Point3D Point3D::operator-() const
{
	return Point3D(-x, -y, -z);
}

double Point3D::operator*(const Point3D& other) const
{
	return x*other.x + y*other.y + z*other.z;
}

Point3D Point3D::operator*(double s) const
{
	return Point3D(x*s, y*s, z*s);
}

Point3D Point3D::operator/(double s) const
{
	return Point3D(x/s, y/s, z/s);
}

Point3D Point3D::Cross(const Point3D& other) const
{
	return Point3D(y*other.z - z*other.y,
		z*other.x - x*other.z,
		x*other.y - y*other.x);
}

double Point3D::Norm() const
{
	return sqrt(SquareNorm());
}

double Point3D::SquareNorm() const
{
	return (*this)*(*this);
}

Point3D Point3D::Normalized() const
{
	return (*this) / Norm();
}

Point3D operator*(double s, const Point3D& p)
{
	return Point3D(p.x*s, p.y*s, p.z*s);
}