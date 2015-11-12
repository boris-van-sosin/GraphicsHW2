#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>

#define GEOMETRIC_COMPUTATION_ESPILON 1e-10

class HomogeneousPoint;

class Point3D
{
public:
	Point3D();
	Point3D(double x_, double y_, double z_);
	Point3D(const Point3D& other);
	explicit Point3D(const HomogeneousPoint& h);

public:
	Point3D operator+(const Point3D& other) const;
	Point3D operator-(const Point3D& other) const;
	Point3D operator-() const;
	double operator*(const Point3D& other) const;
	Point3D operator*(double s) const;
	Point3D operator/(double s) const;
	Point3D Cross(const Point3D& other) const;

	double Norm() const;
	double SquareNorm() const;
	Point3D Normalized() const;

public:
	double x, y, z;
	static const Point3D Zero;
};

Point3D operator*(double s, const Point3D& p);

typedef Point3D Vector3D;

class HomogeneousPoint
{
public:
	HomogeneousPoint();
	HomogeneousPoint(double x_, double y_, double z_, double w_=1.0);
	HomogeneousPoint(const HomogeneousPoint& other);
	explicit HomogeneousPoint(const Point3D& p);

	double operator*(const HomogeneousPoint& other) const;

public:
	double x, y, z, w;
};

class LineSegment
{
public:
	LineSegment(const Point3D& p0_, const Point3D& p1_);

	//int Octant() const;
	/*
	321
	4*0
	567
	*/

	Point3D p0, p1;
};

class Polygon3D
{
public:
	Polygon3D();
	Polygon3D(const std::vector<Point3D>& points_);

	std::vector<LineSegment> Edges() const;
	Vector3D Normal() const;

	std::vector<Point3D> points;
};

class PolygonalObject
{
public:
	PolygonalObject();
	PolygonalObject(const std::vector<Polygon3D>& polygons_);

	std::vector<Polygon3D> polygons;
};

#endif