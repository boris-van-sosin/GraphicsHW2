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

Point3D::Point3D(const HomogeneousPoint& h)
	: Point3D(h.x / h.w, h.y / h.w, h.z / h.w)
{
}

Point3D Point3D::operator+(const Point3D& other) const
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

const Point3D Point3D::Zero;

HomogeneousPoint::HomogeneousPoint()
	: HomogeneousPoint(0, 0, 0, 1)
{
}

HomogeneousPoint::HomogeneousPoint(double x_, double y_, double z_, double w_)
	: HomogeneousPoint(x, y, z, w)
{
}

HomogeneousPoint::HomogeneousPoint(const HomogeneousPoint& other)
	: HomogeneousPoint(other.x, other.y, other.z, 1)
{
}

HomogeneousPoint::HomogeneousPoint(const Point3D& p)
	: HomogeneousPoint(p.x, p.y, p.z, 1)
{
}

double HomogeneousPoint::operator*(const HomogeneousPoint& other) const
{
	return x*other.x + y*other.y + z*other.z + w*other.w;
}

LineSegment::LineSegment(const Point3D& p0_, const Point3D& p1_)
	: p0(p0_), p1(p1_)
{
}

Polygon::Polygon(const std::vector<Point3D>& points_)
	: points(points_)
{
}

std::vector<LineSegment> Polygon::Edges() const
{
	std::vector<LineSegment> res;
	res.reserve(points.size());
	for (int i = 0; i < points.size(); ++i)
	{
		res.push_back(LineSegment(points[i], points[(i + 1) % points.size()]));
	}
	return res;
}

Vector3D Polygon::Normal() const
{
	for (int i = 0; i < points.size(); ++i)
	{
		const Point3D& p0 = points[i];
		const Point3D& p1 = points[(i + 1) % points.size()];
		Vector3D n = p0.Cross(p1); // with the point order
		if (n.SquareNorm() > GEOMETRIC_COMPUTATION_ESPILON)
		{
			return n;
		}
	}
	// polygon is a "flat" line
	return Point3D::Zero;
}

PolygonalObject::PolygonalObject(const std::vector<PolygonalObject>& polygons_)
	: polygons(polygons_)
{
}
