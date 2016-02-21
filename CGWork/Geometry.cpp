#include "Geometry.h"
#include <math.h>
#include <unordered_map>
#include <set>

Point3D::Point3D()
	: x(0.0), y(0.0), z(0.0), color(RGB(255, 255, 255))
{
}

Point3D::Point3D(double x_, double y_, double z_, COLORREF color_)
	: Point3D(x_, y_, z_, color_, true)
{
}

Point3D::Point3D(double x_, double y_, double z_)
	: Point3D(x_, y_, z_, RGB(255, 255, 255), false)
{
}

Point3D::Point3D(double x_, double y_, double z_, COLORREF color_, bool valid)
	: x(x_), y(y_), z(z_), color(color_), colorValid(valid)
{
}

Point3D::Point3D(const Point3D& other)
	: Point3D(other.x, other.y, other.z, other.color, other.colorValid)
{
}

Point3D::Point3D(const HomogeneousPoint& h)
	: Point3D(h.x / h.w, h.y / h.w, h.z / h.w, h.color, h.colorValid)
{
}

Point3D Point3D::operator+(const Point3D& other) const
{
	return Point3D(x + other.x, y + other.y, z + other.z, color, colorValid);
}

Point3D& Point3D::operator+=(const Point3D& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Point3D& Point3D::operator-=(const Point3D& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}


Point3D Point3D::operator-(const Point3D& other) const
{
	return Point3D(x - other.x, y - other.y, z - other.z, color, colorValid);
}

Point3D Point3D::operator-() const
{
	return Point3D(-x, -y, -z, colorValid);
}

double Point3D::operator*(const Point3D& other) const
{
	return x*other.x + y*other.y + z*other.z;
}

Point3D Point3D::operator*(double s) const
{
	return Point3D(x*s, y*s, z*s, color, colorValid);
}

Point3D Point3D::operator/(double s) const
{
	return Point3D(x / s, y / s, z / s, color, colorValid);
}

Point3D Point3D::Cross(const Point3D& other) const
{
	return Point3D(y*other.z - z*other.y,
		z*other.x - x*other.z,
		x*other.y - y*other.x,
		color, colorValid);
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

double& Point3D::operator[](size_t i)
{
	switch (i)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		throw GeometryException();
		break;
	}
}

const double& Point3D::operator[](size_t i) const
{
	switch (i)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		throw GeometryException();
		break;
	}
}


Point3D operator*(double s, const Point3D& p)
{
	if (p.colorValid)
	{
		return Point3D(p.x*s, p.y*s, p.z*s, p.color);
	}
	else
	{
		return Point3D(p.x*s, p.y*s, p.z*s);
	}
}

const Point3D Point3D::Zero;

HomogeneousPoint::HomogeneousPoint()
	: HomogeneousPoint(0, 0, 0, 1, RGB(255,255,255), false)
{
}

HomogeneousPoint::HomogeneousPoint(double x_, double y_, double z_, double w_, COLORREF color_, bool valid)
	: x(x_), y(y_), z(z_), w(w_), color(color_), colorValid(valid)
{
}

HomogeneousPoint::HomogeneousPoint(double x_, double y_, double z_, double w_, COLORREF color_)
	: HomogeneousPoint(x_, y_, z_, w_, color_, true)
{
}

HomogeneousPoint::HomogeneousPoint(double x_, double y_, double z_, double w_)
	: HomogeneousPoint(x_, y_, z_, w_, RGB(255, 255, 255), false)
{
}

HomogeneousPoint::HomogeneousPoint(const HomogeneousPoint& other)
	: HomogeneousPoint(other.x, other.y, other.z, other.w, other.color, other.colorValid)
{
}

HomogeneousPoint::HomogeneousPoint(const Point3D& p)
	: HomogeneousPoint(p.x, p.y, p.z, 1, p.color, p.colorValid)
{
}

double& HomogeneousPoint::operator[](size_t i)
{
	switch (i)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		throw GeometryException();
		break;
	}
}

const double& HomogeneousPoint::operator[](size_t i) const
{
	switch (i)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		throw GeometryException();
		break;
	}
}

double HomogeneousPoint::operator*(const HomogeneousPoint& other) const
{
	return x*other.x + y*other.y + z*other.z + w*other.w;
}

const HomogeneousPoint HomogeneousPoint::Zeros(0, 0, 0, 0);
const HomogeneousPoint HomogeneousPoint::ZerosWithW1(0, 0, 0, 1);

LineSegment::LineSegment(const HomogeneousPoint& p0_, const HomogeneousPoint& p1_)
	: p0(p0_), p1(p1_)
{
}

Vector3D LineSegment::DirectionVector() const
{
	return (Point3D(p1) - Point3D(p0)).Normalized();
}

Polygon3D::Polygon3D()
	: Polygon3D(std::vector<HomogeneousPoint>(), std::vector<Vector3D>(), RGB(255, 25, 255), false)
{
}

Polygon3D::Polygon3D(const std::vector<HomogeneousPoint>& points_, const std::vector<Vector3D>& tmpN, COLORREF color_, bool valid)
	: points(points_), color(color_), colorValid(valid), tmpNormals(tmpN)
{
}

Polygon3D::Polygon3D(const std::vector<HomogeneousPoint>& points_)
	: Polygon3D(points_, std::vector<Vector3D>(), RGB(255, 25, 255), false)
{
}

Polygon3D::Polygon3D(const std::vector<HomogeneousPoint>& points_, COLORREF color_)
	: Polygon3D(points_, std::vector<Vector3D>(), color_, true)
{
}

Polygon3D::Polygon3D(const std::vector<HomogeneousPoint>& points_, const std::vector<Vector3D>& tmpN)
	: Polygon3D(points_, tmpN, RGB(255, 25, 255), false)
{
}

Polygon3D::Polygon3D(const std::vector<HomogeneousPoint>& points_, const std::vector<Vector3D>& tmpN, COLORREF color_)
	: Polygon3D(points_, tmpN, color_, true)
{
}

std::vector<LineSegment> Polygon3D::Edges() const
{
	std::vector<LineSegment> res;
	res.reserve(points.size());
	for (size_t i = 0; i < points.size(); ++i)
	{
		res.push_back(LineSegment(points[i], points[(i + 1) % points.size()]));
	}
	return res;
}

Vector3D Polygon3D::Normal() const
{
	for (size_t i = 0; i < points.size(); ++i)
	{
		const Point3D& p0 = Point3D(points[i]);
		const Point3D& p1 = Point3D( points[(i + 1) % points.size()]);
		const Point3D& p2 = Point3D(points[(i + 2) % points.size()]);
		Vector3D n = (p1 - p0).Normalized().Cross((p2 - p1).Normalized()); // with the point order
		if (n.SquareNorm() > GEOMETRIC_COMPUTATION_EPSILON)
		{
			return n.Normalized();
		}
	}
	// polygon is a "flat" line
	return Point3D::Zero;
}

std::pair<double, HomogeneousPoint> Polygon3D::AreaAndCentroid() const
{
	if (points.empty())
	{
		return std::pair<double, HomogeneousPoint>(0, HomogeneousPoint::Zeros);
	}
	Vector3D v = Vector3D::Zero;
	Point3D p = Point3D::Zero;
	const Vector3D n = Normal();
	const Point3D first = Point3D(points.front());
	for (auto i = points.begin(); i != points.end(); ++i)
	{
		std::vector<HomogeneousPoint>::const_iterator j = (i + 1 != points.end() ? (i + 1) : points.begin());
		const Point3D ip(*i);
		const Point3D jp(*j);
		v += ip.Cross(jp);

		p += (first + ip + jp) * ((ip - first).Cross(jp - first) * n);
	}
	double area = fabs((v *n) / 2.0);
	return std::pair<double, HomogeneousPoint>(area, HomogeneousPoint(p / (area * 6)));
}

PolygonalObject::PolygonalObject(const std::vector<Polygon3D>& polygons_, COLORREF color_, bool valid, double opacity_, bool opacityValid_)
	: polygons(polygons_), color(color_), colorValid(valid), opacity(opacity_), opacityValid(opacityValid_)
{
}

PolygonalObject::PolygonalObject()
	: PolygonalObject(std::vector<Polygon3D>(), RGB(255, 255, 255), false, 1.0, false)
{
}

PolygonalObject::PolygonalObject(const std::vector<Polygon3D>& polygons_, COLORREF color_)
	: PolygonalObject(polygons_, color_, true, 1.0, false)
{
}

PolygonalObject::PolygonalObject(const std::vector<Polygon3D>& polygons_, double opacity_)
	: PolygonalObject(polygons_, RGB(255, 255, 255), false, opacity_, true)
{
}

PolygonalObject::PolygonalObject(const std::vector<Polygon3D>& polygons_, COLORREF color_, double opacity_)
	: PolygonalObject(polygons_, color_, true, opacity_, true)
{
}


PolygonalObject::PolygonalObject(const std::vector<Polygon3D>& polygons_)
	: PolygonalObject(polygons_, RGB(255, 255, 255), false, 1.0, false)
{
}

BoundingBox::BoundingBox(double minX_, double maxX_, double minY_, double maxY_, double minZ_, double maxZ_)
	: minX(minX_), maxX(maxX_), minY(minY_), maxY(maxY_), minZ(minZ_), maxZ(maxZ_)
{}

BoundingBox::BoundingBox(const BoundingBox& other)
	: BoundingBox(other.minX, other.maxX, other.minY, other.maxY, other.minZ, other.maxZ)
{}

BoundingBox::BoundingBox(const BoundingBox& b1, const BoundingBox& b2)
	: BoundingBox(
	fmin(b1.minX, b2.minX),
	fmax(b1.maxX, b2.maxX),
	fmin(b1.minY, b2.minY),
	fmax(b1.maxY, b2.maxY),
	fmin(b1.minZ, b2.minZ),
	fmax(b1.maxZ, b2.maxZ))
{}

BoundingBox BoundingBox::OfLineSegmnet(const LineSegment& line)
{
	const Point3D p0(line.p0);
	const Point3D p1(line.p1);
	return BoundingBox(fmin(p0.x, p1.x), fmax(p0.x, p1.x), fmin(p0.y, p1.y), fmax(p0.y, p1.y), fmin(p0.z, p1.z), fmax(p0.z, p1.z));
}

BoundingBox BoundingBox::OfLineSegmnets(const std::vector<LineSegment>& lines)
{
	if (lines.empty())
	{
		throw EmptyBoundingBoxException();
	}

	BoundingBox first = OfLineSegmnet(lines.front());
	double minX = first.minX,
		maxX = first.maxX,
		minY = first.minY,
		maxY = first.maxY,
		minZ = first.minZ,
		maxZ = first.maxZ;
	for (std::vector<LineSegment>::const_iterator i = (lines.begin() + 1); i != lines.end(); ++i)
	{
		BoundingBox curr = OfLineSegmnet(*i);
		minX = fmin(minX, curr.minX);
		maxX = fmax(maxX, curr.maxX);
		minY = fmin(minY, curr.minY);
		maxY = fmax(maxY, curr.maxY);
		minZ = fmin(minZ, curr.minZ);
		maxZ = fmax(maxZ, curr.maxZ);
	}

	return BoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
}

BoundingBox BoundingBox::OfPolygon(const Polygon3D& poly)
{
	return OfLineSegmnets(poly.Edges());
}

BoundingBox BoundingBox::OfPolygons(const std::vector<Polygon3D>& polys)
{
	if (polys.empty())
	{
		throw EmptyBoundingBoxException();
	}

	BoundingBox first = OfPolygon(polys.front());
	double minX = first.minX,
		maxX = first.maxX,
		minY = first.minY,
		maxY = first.maxY,
		minZ = first.minZ,
		maxZ = first.maxZ;
	for (std::vector<Polygon3D>::const_iterator i = (polys.begin() + 1); i != polys.end(); ++i)
	{
		BoundingBox curr = OfPolygon(*i);
		minX = fmin(minX, curr.minX);
		maxX = fmax(maxX, curr.maxX);
		minY = fmin(minY, curr.minY);
		maxY = fmax(maxY, curr.maxY);
		minZ = fmin(minZ, curr.minZ);
		maxZ = fmax(maxZ, curr.maxZ);
	}

	return BoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
}

BoundingBox BoundingBox::OfObject(const PolygonalObject& obj)
{
	return OfPolygons(obj.polygons);
}

BoundingBox BoundingBox::OfObjects(const std::vector<PolygonalObject>& objs)
{
	if (objs.empty())
	{
		throw EmptyBoundingBoxException();
	}

	BoundingBox first = OfObject(objs.front());
	double minX = first.minX,
		maxX = first.maxX,
		minY = first.minY,
		maxY = first.maxY,
		minZ = first.minZ,
		maxZ = first.maxZ;
	for (std::vector<PolygonalObject>::const_iterator i = (objs.begin() + 1); i != objs.end(); ++i)
	{
		BoundingBox curr = OfObject(*i);
		minX = fmin(minX, curr.minX);
		maxX = fmax(maxX, curr.maxX);
		minY = fmin(minY, curr.minY);
		maxY = fmax(maxY, curr.maxY);
		minZ = fmin(minZ, curr.minZ);
		maxZ = fmax(maxZ, curr.maxZ);
	}

	return BoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
}

BoundingBox BoundingBox::join(const std::vector<BoundingBox>& boxes)
{
	if (boxes.empty())
	{
		throw EmptyBoundingBoxException();
	}

	double minX = boxes.front().minX, 		
		maxX = boxes.front().maxX,
		minY = boxes.front().minY,
		maxY = boxes.front().maxY,
		minZ = boxes.front().minZ,
		maxZ = boxes.front().maxZ;
	for (std::vector<BoundingBox>::const_iterator i = (boxes.begin()+1); i != boxes.end(); ++i)
	{
		minX = fmin(minX, i->minX);
		maxX = fmax(maxX, i->maxX);
		minY = fmin(minY, i->minY);
		maxY = fmax(maxY, i->maxY);
		minZ = fmin(minZ, i->minZ);
		maxZ = fmax(maxZ, i->maxZ);
	}
	return BoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
}

BoundingBox BoundingBox::BoundingCube() const
{
	const Point3D center((minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2);
	const double maxSize = fmax(maxX - minX, fmax(maxY - minY, maxZ - minZ));
	const double halfMaxSize = maxSize / 2;
	return BoundingBox(center.x - halfMaxSize,
					   center.x + halfMaxSize,
					   center.y - halfMaxSize,
					   center.y + halfMaxSize,
					   center.z - halfMaxSize,
					   center.z + halfMaxSize);
}

PolygonalObject BoundingBox::ToObject(bool solid) const
{
	const HomogeneousPoint corners[] = {
		HomogeneousPoint(minX, minY, minZ),
		HomogeneousPoint(minX, minY, maxZ),
		HomogeneousPoint(minX, maxY, minZ),
		HomogeneousPoint(minX, maxY, maxZ),

		HomogeneousPoint(maxX, minY, minZ),
		HomogeneousPoint(maxX, minY, maxZ),
		HomogeneousPoint(maxX, maxY, minZ),
		HomogeneousPoint(maxX, maxY, maxZ),
	};

	if (solid)
	{
		Polygon3D sides[6]; // bottom, top, left, right, frott, back
		//bottom
		sides[0].points.push_back(corners[0]);
		sides[0].points.push_back(corners[1]);
		sides[0].points.push_back(corners[5]);
		sides[0].points.push_back(corners[4]);

		//top
		sides[1].points.push_back(corners[2]);
		sides[1].points.push_back(corners[3]);
		sides[1].points.push_back(corners[7]);
		sides[1].points.push_back(corners[6]);

		//left
		sides[2].points.push_back(corners[0]);
		sides[2].points.push_back(corners[1]);
		sides[2].points.push_back(corners[3]);
		sides[2].points.push_back(corners[2]);

		//right
		sides[3].points.push_back(corners[4]);
		sides[3].points.push_back(corners[5]);
		sides[3].points.push_back(corners[7]);
		sides[3].points.push_back(corners[6]);

		//front
		sides[4].points.push_back(corners[0]);
		sides[4].points.push_back(corners[2]);
		sides[4].points.push_back(corners[6]);
		sides[4].points.push_back(corners[4]);

		//back
		sides[5].points.push_back(corners[1]);
		sides[5].points.push_back(corners[3]);
		sides[5].points.push_back(corners[7]);
		sides[5].points.push_back(corners[5]);


		return PolygonalObject(std::vector<Polygon3D>(sides, sides + 6));
	}
	else
	{
		Polygon3D sides[4]; // bottom, top, left, right. no need for front and back
		//bottom
		sides[0].points.push_back(corners[0]);
		sides[0].points.push_back(corners[1]);
		sides[0].points.push_back(corners[5]);
		sides[0].points.push_back(corners[4]);

		//top
		sides[1].points.push_back(corners[2]);
		sides[1].points.push_back(corners[3]);
		sides[1].points.push_back(corners[7]);
		sides[1].points.push_back(corners[6]);

		//left
		sides[2].points.push_back(corners[0]);
		sides[2].points.push_back(corners[1]);
		sides[2].points.push_back(corners[3]);
		sides[2].points.push_back(corners[2]);

		//right
		sides[3].points.push_back(corners[4]);
		sides[3].points.push_back(corners[5]);
		sides[3].points.push_back(corners[7]);
		sides[3].points.push_back(corners[6]);

		return PolygonalObject(std::vector<Polygon3D>(sides, sides + 4));
	}
}

std::vector<PolygonalObject> BoundingBox::BoundingBoxObjectsOfSubObjects(const std::vector<PolygonalObject>& objs)
{
	std::vector<PolygonalObject> res;
	res.reserve(objs.size());
	for (auto i = objs.begin(); i != objs.end(); ++i)
	{
		res.push_back(OfObject(*i).ToObject());
	}
	return res;
}

inline double PolygonIntersection::PlaneLineIntersectionParam(const Point3D& pt, const Vector3D& lineDir, const Point3D& planePoint, const Vector3D& n)
{
	return n * (planePoint - pt) / (n * lineDir);
}

inline Point3D PolygonIntersection::PlaneLineIntersection(const Point3D& pt, const Vector3D& lineDir, const Point3D& planePoint, const Vector3D& n)
{
	return pt + PlaneLineIntersectionParam(pt, lineDir, planePoint, n) * lineDir;
}

inline bool IsLeft(const Point3D& pt, const Point3D& lp, const Vector3D v, const Vector3D& pn)
{
	return v.Cross(pt - lp) * pn > 0;
}

inline bool PolygonIntersection::IsPointInPolygon(const Point3D& pt, const Polygon3D& poly)
{
	return IsPointInPolygon(pt, poly, poly.Normal());
}

inline bool PolygonIntersection::IsPointInPolygon(const Point3D& pt, const Polygon3D& poly, const Vector3D& n)
{
	if (poly.points.size() < 3)
		return false;

	const Vector3D up = Point3D(poly.points[1]) - Point3D(poly.points[0]);

	int wn = 0;    // the  winding number counter
	// loop through all edges of the polygon
	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		const Point3D p0(*i);
		const Point3D p1((i + 1) != poly.points.end() ? *(i + 1) : poly.points.front());
		const Vector3D ev = p1 - p0;

		if ((pt - p0) * up >= 0)
		{          // start y <= P.y
			if ((pt - p1) * up < 0)      // an upward crossing
				if (IsLeft(pt, p0, ev, n))  // P left of  edge
					++wn;            // have  a valid up intersect
		}
		else
		{                        // start y > P.y (no test needed)
			if ((pt - p1) * up >= 0)     // a downward crossing
				if (!IsLeft(pt, p0, ev, n))  // P right of  edge
					--wn;            // have  a valid down intersect
		}
	}
	return wn != 0;
}

inline std::pair<bool, double> PolygonIntersection::PolygonRayIntersectionParam(const Point3D& pt, const Vector3D& lineDir, const Polygon3D& poly)
{
	return PolygonRayIntersectionParam(pt, lineDir, poly, poly.Normal());
}

inline std::pair<bool, double> PolygonIntersection::PolygonRayIntersectionParam(const Point3D& pt, const Vector3D& lineDir, const Polygon3D& poly, const Vector3D& n)
{
	if (poly.points.size() < 3)
		return std::pair<bool, double>(false, 0);

	if (lineDir * n == 0)
		return std::pair<bool, double>(false, 0);

	const double s = PlaneLineIntersectionParam(pt, lineDir, Point3D(poly.points.front()), n);
	if (s < 0)
		return std::pair<bool, double>(false, s);

	const Point3D pointOnPlane = pt + s * lineDir;
	const bool inPolygon = IsPointInPolygon(pointOnPlane, poly, n);
	return std::pair<bool, double>(inPolygon, s);
}


PolygonAdjacency::PolygonAdjacency(int p, int obj, int polyInObj, int v)
	: objIdx(obj), polygonInObjIdx(polyInObj), vertexIdx(v)
{
	polygonIdxs.reserve(2);
	polygonIdxs.push_back(p);
}

PolygonAdjacency::PolygonAdjacency(const PolygonAdjacency& other)
	: polygonIdxs(other.polygonIdxs), objIdx(other.objIdx), polygonInObjIdx(other.polygonInObjIdx), vertexIdx(other.vertexIdx)
{}

PolygonAdjacency::PolygonAdjacency()
{}


struct HashAndComparePoint3D
{
	size_t operator()(const Point3D& p) const
	{
		return (size_t)(p.Norm() * 10000);
	}
	bool operator()(const Point3D& p0, const Point3D& p1) const
	{
		return (fabs(p0.x - p1.x) < GEOMETRIC_COMPUTATION_EPSILON) &&
			(fabs(p0.y - p1.y) < GEOMETRIC_COMPUTATION_EPSILON) &&
			(fabs(p0.z - p1.z) < GEOMETRIC_COMPUTATION_EPSILON);
	}
};

struct HashAndCompareEdge
{
	size_t operator()(const LineSegment& e) const
	{
		return _pointHash(Point3D(e.p0)) ^ _pointHash(Point3D(e.p1));
	}
	bool operator()(const LineSegment& e0, const LineSegment& e1) const
	{
		return (_pointHash(Point3D(e0.p0), Point3D(e1.p0)) && _pointHash(Point3D(e0.p1), Point3D(e1.p1))) ||
			(_pointHash(Point3D(e0.p0), Point3D(e1.p1)) && _pointHash(Point3D(e0.p1), Point3D(e1.p0)));
	}

private:
	HashAndComparePoint3D _pointHash;
};

Normals::PolygonNormalData::PolygonNormalData(const LineSegment& ls)
	: PolygonNormal(ls)
{
}

Normals::PolygonNormalData::PolygonNormalData(const Normals::PolygonNormalData& other)
	: PolygonNormal(other.PolygonNormal), VertexNormals(other.VertexNormals)
{
}

Normals::PolygonNormalData& Normals::PolygonNormalData::operator = (const Normals::PolygonNormalData& other)
{
	if (this == &other)
		return *this;
	PolygonNormal = other.PolygonNormal;
	VertexNormals = other.VertexNormals;
	return *this;
}

void FixNormal(const PolygonalObject& obj, size_t polygonIdx, const std::pair<double, HomogeneousPoint>& areaAndCentroid, Vector3D& normal)
{
	const Point3D centroid(areaAndCentroid.second);
	bool evenIntersections = true; // zero is an even number
	for (size_t i = 0; i != obj.polygons.size(); ++i)
	{
		if (i == polygonIdx)
			continue;

		const std::pair<bool, double> intersectionRes = PolygonIntersection::PolygonRayIntersectionParam(centroid, normal, obj.polygons[i]);
		if (intersectionRes.first && (intersectionRes.second > 0))
		{
			evenIntersections = !evenIntersections;
		}
	}

	if (!evenIntersections)
	{
		normal = -normal;
	}
}

void Normals::ComputeNormals(const PolygonalModel& objs, std::vector<Normals::PolygonNormalData>& polygonNormals, NormalList& vertexNormals, PolygonAdjacencyGraph& polygonAdjacency, NormalsGeneration src)
{
	polygonNormals.clear();
	vertexNormals.clear();
	if (objs.empty())
	{
		return;
	}
	std::vector<double> polygonAreas;
	const size_t approxNumPolygons = objs.front().polygons.size() * objs.size();
	polygonAreas.reserve(approxNumPolygons);
	polygonNormals.reserve(approxNumPolygons);
	const size_t approxNumVertices = approxNumPolygons * (objs.front().polygons.empty() ? 1 : objs.front().polygons.front().points.size());
	std::unordered_map<Point3D, Vector3D, HashAndComparePoint3D, HashAndComparePoint3D> vertexMap;
	vertexMap.reserve(approxNumVertices);

	std::unordered_map<LineSegment, PolygonAdjacency, HashAndCompareEdge, HashAndCompareEdge> edgeMap;
	edgeMap.reserve(2 * approxNumVertices);

	int polygonIdx = 0;
	for (auto i = objs.begin(); i != objs.end(); ++i)
	{
		for (auto j = i->polygons.begin(); j != i->polygons.end(); ++j)
		{
			const std::pair<double, HomogeneousPoint> areaAndCentroid = j->AreaAndCentroid();
			Vector3D currPolygonNormal = j->Normal();
			if (src == NORMALS_SMART)
			{
				FixNormal(*i, j - i->polygons.begin(), areaAndCentroid, currPolygonNormal);
			}

			PolygonNormalData d(LineSegment(areaAndCentroid.second, HomogeneousPoint(Point3D(areaAndCentroid.second) + currPolygonNormal)));
			polygonNormals.push_back(d);

			polygonAreas.push_back(areaAndCentroid.first);
			for (auto v = j->points.begin(); v != j->points.end(); ++v)
			{
				if ((src != NORMALS_FILE) || j->tmpNormals.empty())
				{
					if (vertexMap.find(Point3D(*v)) == vertexMap.end())
					{
						vertexMap[Point3D(*v)] = currPolygonNormal * areaAndCentroid.first;
					}
					else
					{
						vertexMap[Point3D(*v)] += currPolygonNormal * areaAndCentroid.first;
					}
				}
				else
				{
					vertexMap[Point3D(*v)] = j->tmpNormals[v - j->points.begin()];
				}

				const LineSegment currEdge(*v, (v + 1 != j->points.end()) ? *(v + 1) : j->points.front());
				if (edgeMap.find(currEdge) == edgeMap.end())
				{
					edgeMap[currEdge] = PolygonAdjacency(polygonIdx, (i - objs.begin()), (j - i->polygons.begin()), v - j->points.begin());
				}
				else
				{
					edgeMap[currEdge].polygonIdxs.push_back(polygonIdx);
				}
			}
			++polygonIdx;
		}
	}

	polygonAdjacency.clear();
	polygonAdjacency.reserve(edgeMap.size());
	for (auto e = edgeMap.begin(); e != edgeMap.end(); ++e)
	{
		polygonAdjacency.push_back(e->second);
	}

	vertexNormals.reserve(vertexMap.size());
	for (auto i = vertexMap.begin(); i != vertexMap.end(); ++i)
	{
		const Vector3D direction = i->second.Normalized();
		vertexNormals.push_back(LineSegment(HomogeneousPoint(i->first), HomogeneousPoint(i->first + direction)));
	}

	polygonIdx = 0;
	for (auto i = objs.begin(); i != objs.end(); ++i)
	{
		for (auto j = i->polygons.begin(); j != i->polygons.end(); ++j)
		{
			for (auto v = j->points.begin(); v != j->points.end(); ++v)
			{
				const Vector3D direction = vertexMap[Point3D(*v)].Normalized();
				polygonNormals[polygonIdx].VertexNormals.push_back(LineSegment(*v, HomogeneousPoint(Point3D(*v) + direction)));
			}
			++polygonIdx;
		}
	}
}

Vector3D Normals::PolygonNormalData::NormalDirection() const
{
	return PolygonNormal.DirectionVector();
}
