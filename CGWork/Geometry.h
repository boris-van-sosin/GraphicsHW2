#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <atlimage.h>

#define GEOMETRIC_COMPUTATION_EPSILON 1e-10

class HomogeneousPoint;

class GeometryException {};

class Point3D
{
public:
	Point3D();
	Point3D(double x_, double y_, double z_);
	Point3D(double x_, double y_, double z_, COLORREF color_);
	Point3D(const Point3D& other);
	explicit Point3D(const HomogeneousPoint& h);

public:
	Point3D operator+(const Point3D& other) const;
	Point3D operator-(const Point3D& other) const;
	Point3D& operator+=(const Point3D& other);
	Point3D& operator-=(const Point3D& other);
	Point3D operator-() const;
	double operator*(const Point3D& other) const;
	Point3D operator*(double s) const;
	Point3D operator/(double s) const;
	Point3D Cross(const Point3D& other) const;

	double Norm() const;
	double SquareNorm() const;
	Point3D Normalized() const;
	
	double& operator[](size_t i);
	const double& operator[](size_t i) const;

public:
	double x, y, z;
	COLORREF color;
	bool colorValid;
	static const Point3D Zero;

private:
	Point3D(double x_, double y_, double z_, COLORREF color_, bool valid);
};

Point3D operator*(double s, const Point3D& p);

typedef Point3D Vector3D;

class HomogeneousPoint
{
public:
	HomogeneousPoint();
	HomogeneousPoint(double x_, double y_, double z_, double w_ = 1.0);
	HomogeneousPoint(double x_, double y_, double z_, double w_, COLORREF color_);
	HomogeneousPoint(const HomogeneousPoint& other);
	explicit HomogeneousPoint(const Point3D& p);

	double operator*(const HomogeneousPoint& other) const;

	double& operator[](size_t i);
	const double& operator[](size_t i) const;

public:
	double x, y, z, w;
	COLORREF color;
	bool colorValid;
	static const HomogeneousPoint Zeros;
	static const HomogeneousPoint ZerosWithW1;
private:
	HomogeneousPoint(double x_, double y_, double z_, double w_, COLORREF color_, bool valid);
};

class LineSegment
{
public:
	LineSegment(const HomogeneousPoint& p0_, const HomogeneousPoint& p1_);
	Vector3D DirectionVector() const;
	HomogeneousPoint p0, p1;
};

class Polygon3D
{
public:
	Polygon3D();
	Polygon3D(const std::vector<HomogeneousPoint>& points_);
	Polygon3D(const std::vector<HomogeneousPoint>& points_, COLORREF color_);
	Polygon3D(const std::vector<HomogeneousPoint>& points_, const std::vector<Vector3D>& tmpN);
	Polygon3D(const std::vector<HomogeneousPoint>& points_, const std::vector<Vector3D>& tmpN, COLORREF color_);

	std::vector<LineSegment> Edges() const;
	Vector3D Normal() const;
	std::pair<double, HomogeneousPoint> AreaAndCentroid() const;

	std::vector<HomogeneousPoint> points;
	std::vector<Vector3D> tmpNormals;
	COLORREF color;
	bool colorValid;
private:
	Polygon3D(const std::vector<HomogeneousPoint>& points_, const std::vector<Vector3D>& tmpN, COLORREF color_, bool valid);
};

class PolygonalObject
{
public:
	PolygonalObject();
	PolygonalObject(const std::vector<Polygon3D>& polygons_);
	PolygonalObject(const std::vector<Polygon3D>& polygons_, COLORREF color_);
	PolygonalObject(const std::vector<Polygon3D>& polygons_, double opacity_);
	PolygonalObject(const std::vector<Polygon3D>& polygons_, COLORREF color_, double opacity_);

	std::vector<Polygon3D> polygons;
	COLORREF color;
	bool colorValid;
	double opacity;
	bool opacityValid;
private:
	PolygonalObject(const std::vector<Polygon3D>& polygons_, COLORREF color_, bool valid, double opacity_, bool opacityValid_);
};

typedef std::vector<PolygonalObject> PolygonalModel;

class EmptyBoundingBoxException {};

class BoundingBox
{
public:
	BoundingBox(double minX_, double maxX_, double minY_, double maxY_, double minZ_, double maxZ_);
public:
	BoundingBox(const BoundingBox& other);
	BoundingBox(const BoundingBox& b1, const BoundingBox& b2);
	
	BoundingBox BoundingCube() const;
	PolygonalObject ToObject(bool solid = false) const;

public:
	static BoundingBox OfLineSegmnet(const LineSegment& line);
	static BoundingBox OfLineSegmnets(const std::vector<LineSegment>& lines);
	static BoundingBox OfPolygon(const Polygon3D& poly);
	static BoundingBox OfPolygons(const std::vector<Polygon3D>& polys);
	static BoundingBox OfObject(const PolygonalObject& obj);
	static BoundingBox OfObjects(const std::vector<PolygonalObject>& objs);

	static std::vector<PolygonalObject> BoundingBoxObjectsOfSubObjects(const std::vector<PolygonalObject>& objs);

public:
	const double minX, maxX, minY, maxY, minZ, maxZ;

private:
	static BoundingBox join(const std::vector<BoundingBox>& boxes);
};

namespace PolygonIntersection
{
	inline double PlaneLineIntersectionParam(const Point3D& pt, const Vector3D& lineDir, const Point3D& planePoint, const Vector3D& n);
	inline Point3D PlaneLineIntersection(const Point3D& pt, const Vector3D& lineDir, const Point3D& planePoint, const Vector3D& n);
	inline bool IsPointInPolygon(const Point3D& pt, const Polygon3D& poly);
	inline bool IsPointInPolygon(const Point3D& pt, const Polygon3D& poly, const Vector3D& n);
	inline std::pair<bool, double> PolygonRayIntersectionParam(const Point3D& pt, const Vector3D& lineDir, const Polygon3D& poly);
	inline std::pair<bool, double> PolygonRayIntersectionParam(const Point3D& pt, const Vector3D& lineDir, const Polygon3D& poly, const Vector3D& n);
}

class PolygonAdjacency
{
public:
	PolygonAdjacency(int p, int obj, int polyInObj, int v);
	PolygonAdjacency(const PolygonAdjacency& other);
	PolygonAdjacency();

	std::vector<int> polygonIdxs;
	int objIdx, polygonInObjIdx;
	int vertexIdx;
};

typedef std::vector<PolygonAdjacency> PolygonAdjacencyGraph;

namespace Normals
{
	enum NormalsGeneration { NORMALS_VERTEX_ORDER, NORMALS_FILE, NORMALS_SMART };

	typedef std::vector<LineSegment> NormalList;

	struct PolygonNormalData
	{
		PolygonNormalData(const LineSegment& ls);
		PolygonNormalData(const PolygonNormalData& other);
		PolygonNormalData& operator=(const PolygonNormalData& other);

		LineSegment PolygonNormal;
		NormalList VertexNormals;

		Vector3D NormalDirection() const;
	};

	void ComputeNormals(const PolygonalModel& objs, std::vector<PolygonNormalData>& polygonNormals, NormalList& vertexNormals, PolygonAdjacencyGraph& polygonAdjacency, NormalsGeneration src = NORMALS_VERTEX_ORDER);
}

#endif