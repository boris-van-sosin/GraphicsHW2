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

MatrixHomogeneous ToMatrixHomogeneous(const Matrix3D& m)
{
	HomogeneousPoint cols[] = {
		HomogeneousPoint(m * Point3D(1, 0, 0)),
		HomogeneousPoint(m * Point3D(0, 1, 0)),
		HomogeneousPoint(m * Point3D(0, 0, 1)),
		HomogeneousPoint::ZerosWithW1
	};
	cols[0][3] = cols[1][3] = cols[2][3] = 0;
	return MatrixHomogeneous(cols).Transposed();
}

Polygon3D operator*(const Matrix3D& m, const Polygon3D& poly)
{
	const MatrixHomogeneous mh = ToMatrixHomogeneous(m);
	std::vector<HomogeneousPoint> points;
	points.reserve(poly.points.size());
	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		points.push_back(mh * (*i));
	}
	if (poly.colorValid)
	{
		return Polygon3D(points, poly.color);
	}
	else
	{
		return Polygon3D(points);
	}
}

Polygon3D operator*(const MatrixHomogeneous& m, const Polygon3D& poly)
{
	std::vector<HomogeneousPoint> points;
	points.reserve(poly.points.size());
	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		points.push_back(m * (*i));
	}
	if (poly.colorValid)
	{
		return Polygon3D(points, poly.color);
	}
	else
	{
		return Polygon3D(points);
	}
}

PolygonalObject operator*(const Matrix3D& m, const PolygonalObject& obj)
{
	std::vector<Polygon3D> polygons;
	polygons.reserve(obj.polygons.size());
	for (std::vector<Polygon3D>::const_iterator i = obj.polygons.begin(); i != obj.polygons.end(); ++i)
	{
		polygons.push_back(m * (*i));
	}
	if (obj.colorValid)
	{
		if (obj.opacityValid)
			return PolygonalObject(polygons, obj.color, obj.opacity);
		else
			return PolygonalObject(polygons, obj.color);
	}
	else
	{
		if (obj.opacityValid)
			return PolygonalObject(polygons, obj.opacity);
		else
			return PolygonalObject(polygons);
	}
}

PolygonalObject operator*(const MatrixHomogeneous& m, const PolygonalObject& obj)
{
	std::vector<Polygon3D> polygons;
	polygons.reserve(obj.polygons.size());
	for (std::vector<Polygon3D>::const_iterator i = obj.polygons.begin(); i != obj.polygons.end(); ++i)
	{
		polygons.push_back(m * (*i));
	}
	if (obj.colorValid)
	{
		if (obj.opacityValid)
			return PolygonalObject(polygons, obj.color, obj.opacity);
		else
			return PolygonalObject(polygons, obj.color);
	}
	else
	{
		if (obj.opacityValid)
			return PolygonalObject(polygons, obj.opacity);
		else
			return PolygonalObject(polygons);
	}
}

BoundingBox operator*(const Matrix3D& m, const BoundingBox& bbox)
{
	const MatrixHomogeneous mh = ToMatrixHomogeneous(m);
	HomogeneousPoint corners[] = {
		mh * HomogeneousPoint(bbox.minX, bbox.minY, bbox.minZ),
		mh * HomogeneousPoint(bbox.maxX, bbox.maxY, bbox.maxZ)
	};
	return BoundingBox::OfLineSegmnet(LineSegment(corners[0], corners[1]));
}

BoundingBox operator*(const MatrixHomogeneous& m, const BoundingBox& bbox)
{
	HomogeneousPoint corners[] = {
		HomogeneousPoint(m * HomogeneousPoint(bbox.minX, bbox.minY, bbox.minZ)),
		HomogeneousPoint(m * HomogeneousPoint(bbox.maxX, bbox.maxY, bbox.maxZ))
	};
	return BoundingBox::OfLineSegmnet(LineSegment(corners[0], corners[1]));
}

LineSegment operator*(const Matrix3D& m, const LineSegment& l)
{
	const MatrixHomogeneous mh = ToMatrixHomogeneous(m);
	return LineSegment(mh * l.p0, mh * l.p1);
}

LineSegment operator*(const MatrixHomogeneous& m, const LineSegment& l)
{
	return LineSegment(m * HomogeneousPoint(l.p0), m * HomogeneousPoint(l.p1));
}

Normals::PolygonNormalData operator*(const MatrixHomogeneous&m, const Normals::PolygonNormalData& nd)
{
	Normals::PolygonNormalData res(m * nd.PolygonNormal);
	res.VertexNormals.reserve(nd.VertexNormals.size());
	for (auto i = nd.VertexNormals.begin(); i != nd.VertexNormals.end(); ++i)
	{
		res.VertexNormals.push_back(m * (*i));
	}
	return res;
}

LineSegment TransformNormal(const MatrixHomogeneous& m, const LineSegment& l, double scalingFactor)
{
	const Point3D originTr(m * l.p0);
	const Point3D endpointTr(m * l.p1);
	const Vector3D directionDisp = (endpointTr - originTr).Normalized() * scalingFactor;
	return LineSegment(HomogeneousPoint(originTr), HomogeneousPoint(originTr + directionDisp));
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

CoordinateSystem::CoordinateSystem()
	: _origin(0, 0, 0), _xPoint(1, 0, 0), _yPoint(0, 1, 0), _zPoint(0, 0, 1)
{
}

Point3D CoordinateSystem::Origin() const
{
	return _origin;
}

Vector3D CoordinateSystem::X() const
{
	return _xPoint - _origin;
}

Vector3D CoordinateSystem::Y() const
{
	return _yPoint - _origin;
}

Vector3D CoordinateSystem::Z() const
{
	return _zPoint - _origin;
}

void CoordinateSystem::ApplyMatrix(const Matrix3D& m)
{
	_origin = m * _origin;
}

void CoordinateSystem::ApplyMatrix(const MatrixHomogeneous& m)
{
	_origin = Point3D(m * HomogeneousPoint(_origin));
}
