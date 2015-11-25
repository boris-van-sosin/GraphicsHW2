#include "GeometricTransformations.h"

Matrix3D ToMatrix3D(const Vector3D& r0, const Vector3D& r1, const Vector3D& r2)
{
	Vector3D rows[3] = { r0, r1, r2 };
	return Matrix3D(rows);
}

MatrixHomogeneous ToMatrixHomogeneous(const HomogeneousPoint& r0, const HomogeneousPoint& r1, const HomogeneousPoint& r2, const HomogeneousPoint& r3)
{
	HomogeneousPoint rows[4] = { r0, r1, r2, r3 };
	return MatrixHomogeneous(rows);
}

Polygon3D operator*(const Matrix3D& m, const Polygon3D& poly)
{
	std::vector<Point3D> points;
	points.reserve(poly.points.size());
	for (std::vector<Point3D>::const_iterator i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		points.push_back(m * (*i));
	}
	return Polygon3D(points);
}

Polygon3D operator*(const MatrixHomogeneous& m, const Polygon3D& poly)
{
	std::vector<Point3D> points;
	points.reserve(poly.points.size());
	for (std::vector<Point3D>::const_iterator i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		HomogeneousPoint p(*i);
		points.push_back(Point3D(m * p));
	}
	return Polygon3D(points);
}

PolygonalObject operator*(const Matrix3D& m, const PolygonalObject& obj)
{
	std::vector<Polygon3D> polygons;
	polygons.reserve(obj.polygons.size());
	for (std::vector<Polygon3D>::const_iterator i = obj.polygons.begin(); i != obj.polygons.end(); ++i)
	{
		polygons.push_back(m * (*i));
	}
	return PolygonalObject(polygons);
}

PolygonalObject operator*(const MatrixHomogeneous& m, const PolygonalObject& obj)
{
	std::vector<Polygon3D> polygons;
	polygons.reserve(obj.polygons.size());
	for (std::vector<Polygon3D>::const_iterator i = obj.polygons.begin(); i != obj.polygons.end(); ++i)
	{
		polygons.push_back(m * (*i));
	}
	return PolygonalObject(polygons);
}

namespace Matrices
{
	MatrixHomogeneous Scale(double x, double y, double z)
	{
		HomogeneousPoint rows[] = {
			HomogeneousPoint(x, 0, 0, 0),
			HomogeneousPoint(0, y, 0, 0),
			HomogeneousPoint(0, 0, z, 0),
			HomogeneousPoint(0, 0, 0, 1) };
		return MatrixHomogeneous(rows);
	}

	MatrixHomogeneous Scale(double s)
	{
		return Scale(s, s, s);
	}

	MatrixHomogeneous Rotate(Axis axis, double angle)
	{
		HomogeneousPoint rows[] = {
			HomogeneousPoint(1, 0, 0, 0),
			HomogeneousPoint(0, 1, 0, 0),
			HomogeneousPoint(0, 0, 1, 0),
			HomogeneousPoint(0, 0, 0, 1) };
		switch (axis)
		{
		case AXIS_X:
			rows[2].z = rows[1].y = cos(angle);
			rows[1].z = -(rows[2].y = sin(angle));
			break;
		case AXIS_Y:
			rows[2].z = rows[0].x = cos(angle);
			rows[0].z = -(rows[2].x = sin(angle));
			break;
		case AXIS_Z:
			rows[1].y = rows[0].x = cos(angle);
			rows[0].y = -(rows[1].x = sin(angle));
			break;
		default:
			return ZerosMatrixHomogeneous;
		}
		return MatrixHomogeneous(rows);
	}

	MatrixHomogeneous Translate(double x, double y, double z)
	{
		HomogeneousPoint rows[] = {
			HomogeneousPoint(1, 0, 0, x),
			HomogeneousPoint(0, 1, 0, y),
			HomogeneousPoint(0, 0, 1, z),
			HomogeneousPoint(0, 0, 0, 1) };
		return MatrixHomogeneous(rows);
	}

	MatrixHomogeneous Flip(Axis axis)
	{
		HomogeneousPoint rows[] = {
			HomogeneousPoint(1, 0, 0, 0),
			HomogeneousPoint(0, 1, 0, 0),
			HomogeneousPoint(0, 0, 1, 0),
			HomogeneousPoint(0, 0, 0, 1) };
		switch (axis)
		{
		case AXIS_X:
			rows[0].x = -rows[0].x;
			break;
		case AXIS_Y:
			rows[1].y = -rows[1].y;
			break;
		case AXIS_Z:
			rows[2].z = -rows[2].z;
			break;
		default:
			return ZerosMatrixHomogeneous;
		}
		return MatrixHomogeneous(rows);
	}
}
