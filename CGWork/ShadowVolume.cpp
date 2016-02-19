#include "ShadowVolume.h"

#include <unordered_set>
#include <map>
#include <unordered_map>
#include "Drawing.h"

#define SV_COMPUTATION_EPSILON 1e-3

struct HashAndCompareSVPoint
{
	size_t operator()(const Point3D& p) const
	{
		return (size_t)(p.Norm() * 10000);
	}
	bool operator()(const Point3D& p0, const Point3D& p1) const
	{
		return (fabs(p0.x - p1.x) < SV_COMPUTATION_EPSILON) &&
			(fabs(p0.y - p1.y) < SV_COMPUTATION_EPSILON) &&
			(fabs(p0.z - p1.z) < SV_COMPUTATION_EPSILON);
	}
};

struct HashAndCompareSVEdge
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
	HashAndCompareSVPoint _pointHash;
};

ShadowVolume::ShadowVolume()
	: _width(1), _height(1), _stencil(NULL)
{
}

ShadowVolume::ShadowVolume(size_t w, size_t h, const LightSource& ls)
	: _width(w), _height(h), _lightSource(ls)
{
	if (w | h == 0)
	{
		_width = _height = 1;
	}
	_stencil = new std::set<ShadowEvent>[_height * _width];
}

ShadowVolume::ShadowVolume(const ShadowVolume& other)
	: _width(other._width), _height(other._height), _lightSource(other._lightSource)
{
	_stencil = new std::set<ShadowEvent>[_height * _width];
	for (int i = 0; i < _height*_width; ++i)
	{
		_stencil[i] = other._stencil[i];
	}
}

ShadowVolume::~ShadowVolume()
{
	delete[] _stencil;
}

ShadowVolume& ShadowVolume::operator=(const ShadowVolume& other)
{
	if (this == &other)
		return *this;

	_lightSource = other._lightSource;

	SetSize(other._width, other._height);
	for (int i = 0; i < _height*_width; ++i)
	{
		_stencil[i] = other._stencil[i];
	}
}

size_t ShadowVolume::GetHeight() const
{
	return _height;
}

size_t ShadowVolume::GetWidth() const
{
	return _width;
}

void ShadowVolume::SetSize(size_t w, size_t h)
{
	if (w | h == 0)
	{
		return;
	}

	if (h == _height && w == _width)
	{
		Clear();
		return;
	}

	_height = h;
	_width = w;
	delete[] _stencil;
	_stencil = new std::set<ShadowEvent>[_height*_width];
}

void ShadowVolume::Clear()
{
	for (size_t i = 0; i < _height*_width; ++i)
	{
		_stencil[i].clear();
	}
	_currPolyId = 0;
}

void ShadowVolume::SetPixel(int x, int y, double z, COLORREF id)
{
	if (x < 0 || y < 0 || x >= _width || y >= _height)
	{
		return;
	}
	_stencil[y*_width + x].insert(ShadowEvent(_currShadowMode, z, id));
}

void ShadowVolume::SetLightSource(const LightSource& ls)
{
	_lightSource = ls;
}

ShadowVolume::ShadowEvent::ShadowEvent(ShadowEventType t, double z, COLORREF id)
	: _type(t), _z(z), _id(id)
{
}

ShadowVolume::ShadowEvent::~ShadowEvent()
{
}

bool ShadowVolume::ShadowEvent::operator < (const ShadowEvent& other) const
{
	if (_z != other._z)
		return _z < other._z;
	else
		return _type < other._type;
}

bool ShadowVolume::IsPixelLit(size_t x, size_t y, double z) const
{
	int shadowNesting = 0;
	if (y * _width + x >= _width * _height)
		return true;

	const std::set<ShadowEvent>& currPixel = _stencil[y * _width + x];
	bool evenCrossings = true;
	for (auto i = currPixel.begin(); i != currPixel.end(); ++i)
	{
		if ((i->_type == ShadowEnter && i->_z >= z) || (i->_type == ShadowExit && i->_z > z))
			break;

		if (i->_type == ShadowEnter)
			++shadowNesting;
		else
			--shadowNesting;

		evenCrossings = !evenCrossings;
	}
	return evenCrossings;
	if (shadowNesting > 0)
		return false;
	return true;
	return shadowNesting <= 0;
}

void ShadowVolume::ProcessModel(const PolygonalModel& model, const MatrixHomogeneous& mTotal, const std::vector<Normals::PolygonNormalData>& normals, bool clip, const ClippingPlane& cp, const PolygonAdjacencyGraph& polygonAdj)
{
	std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> shadowObj = GenerateShadowVolume(model, normals, polygonAdj);

	for (auto p = shadowObj.first.polygons.begin(); p != shadowObj.first.polygons.end(); ++p)
	{
		p->color = _currPolyId;
		p->colorValid = true;
		++_currPolyId;
	}

	ModelAttr attr2;
	attr2.removeBackFace = BACKFACE_REMOVE_BACK;
	attr2.Shading = SHADING_NONE;
	attr2.forceColor = false;
	DrawingObject img;
	img.shadowVolume = this;
	img.active = DrawingObject::DRAWING_OBJECT_SV;

	_currShadowMode = ShadowExit;
	DrawObject(img, shadowObj.first, mTotal, attr2, shadowObj.second, 0, true, clip, cp);
	_currShadowMode = ShadowEnter;
	size_t normalsOffset = 0;
	for (auto obj = model.begin(); obj != model.end(); ++obj)
	{
		DrawObject(img, *obj, mTotal, attr2, normals, normalsOffset, true, clip, cp);
		normalsOffset += obj->polygons.size();
	}

	attr2.removeBackFace = BACKFACE_REMOVE_FRONT;
	_currShadowMode = ShadowEnter;
	DrawObject(img, shadowObj.first, mTotal, attr2, shadowObj.second, 0, true, clip, cp);
}

bool ComparePoint3D(const Point3D& p0, const Point3D& p1)
{
	if (p0.x != p1.x)
		return p0.x < p1.x;
	if (p0.y != p1.y)
		return p0.y < p1.y;
	return p0.z < p1.z;
}

std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> ShadowVolume::GenerateShadowVolume(const PolygonalModel& model, const std::vector<Normals::PolygonNormalData>& normals, const PolygonAdjacencyGraph& polygonAdj) const
{
	std::vector<Polygon3D> shadowPolygons;
	std::vector<Normals::PolygonNormalData> shadowNormals;
	double shadowLength = 2;
	for (auto j = polygonAdj.begin(); j != polygonAdj.end(); ++j)
	{
		const Polygon3D& currPoly = model[j->objIdx].polygons[j->polygonInObjIdx];
		const LineSegment edge(currPoly.points[j->vertexIdx], currPoly.points[(j->vertexIdx + 1) % currPoly.points.size()]);

		bool addEdge = false;
		Point3D innerPoint;

		if (j->polygonIdxs.size() == 1)
		{
			// boundary
			addEdge = true;
			innerPoint = Point3D(currPoly.AreaAndCentroid().second);
		}
		else
		{
			// silhouette
			const LineSegment n0 = normals[j->polygonIdxs[0]].PolygonNormal;
			const LineSegment n1 = normals[j->polygonIdxs[1]].PolygonNormal;
			innerPoint = (Point3D(n0.p0) + Point3D(n1.p0)) / 2;
			const Point3D midpoint = (Point3D(edge.p0) + Point3D(edge.p1)) / 2;
			const Vector3D optVector = (_lightSource._type == LightSource::PLANE) ?
										_lightSource.Direction() :
										(midpoint - Point3D(_lightSource._origin));
			const Vector3D n0Vec = n0.DirectionVector();
			const Vector3D n1Vec = n1.DirectionVector();
			if ((n0Vec * optVector) * (n1Vec * optVector) < 0)
			{
				addEdge = true;
				const Vector3D innerDir0 = innerPoint - Point3D(n0.p0);
				const Vector3D innerDir1 = innerPoint - Point3D(n1.p0);
				if (n0Vec * innerDir0 > 0 && n1Vec * innerDir1 > 0)
				{
					innerPoint = midpoint - innerPoint + midpoint;
					//
					//addEdge = false;
				}
			}
		}

		if (addEdge)
		{
			std::vector<HomogeneousPoint> polyPoints;
			polyPoints.reserve(4);

			Vector3D ray0, ray1;
			if (_lightSource._type == LightSource::PLANE)
			{
				ray0 = ray1 = _lightSource.Direction();
			}
			else
			{
				ray0 = (Point3D(edge.p0) - Point3D(_lightSource._origin)).Normalized();
				ray1 = (Point3D(edge.p1) - Point3D(_lightSource._origin)).Normalized();
			}

			const HomogeneousPoint far0((Point3D(edge.p0) + (ray0 * shadowLength)));
			const HomogeneousPoint far1((Point3D(edge.p1) + (ray1 * shadowLength)));

			polyPoints.push_back(edge.p0);
			polyPoints.push_back(edge.p1);
			polyPoints.push_back(far1);
			polyPoints.push_back(far0);
			Polygon3D currShadow(polyPoints);
			if (currShadow.Normal() * (innerPoint - Point3D(edge.p0)) < 0)
			{
				std::reverse(currShadow.points.begin(), currShadow.points.end());
			}

			shadowPolygons.push_back(currShadow);
			const HomogeneousPoint centroid = shadowPolygons.back().AreaAndCentroid().second;
			shadowNormals.push_back(Normals::PolygonNormalData(LineSegment(centroid, HomogeneousPoint(Point3D(centroid) + shadowPolygons.back().Normal()))));

			//
			//if (shadowPolygons.size() >= 23)
			//	break;
		}
	}

	/*const size_t approxNumPolygons = model.front().polygons.size() * model.size();
	const size_t approxNumVertices = approxNumPolygons * (model.front().polygons.empty() ? 1 : model.front().polygons.front().points.size());

	std::unordered_set<Point3D, HashAndCompareSVPoint, HashAndCompareSVPoint> points;
	points.reserve(approxNumVertices);
	std::unordered_map<LineSegment, PolygonAdjacency, HashAndCompareSVEdge, HashAndCompareSVEdge> edgeMap;
	edgeMap.reserve(2 * approxNumVertices);

	size_t polygonIdx = 0;
	for (auto p = shadowPolygons.begin(); p != shadowPolygons.end(); ++p)
	{
		for (auto v = p->points.begin(); v != p->points.end(); ++v)
		{
			std::unordered_set<Point3D, HashAndCompareSVPoint, HashAndCompareSVPoint>::iterator prevV;
			if ((prevV = points.find(Point3D(*v))) == points.end())
				points.insert(Point3D(*v));
			else
			{
				*v = HomogeneousPoint(*prevV);
			}

			const LineSegment currEdge(*v, (v + 1 != p->points.end()) ? *(v + 1) : p->points.front());
			if (edgeMap.find(currEdge) == edgeMap.end())
			{
				edgeMap[currEdge] = PolygonAdjacency(polygonIdx, 0, (p - shadowPolygons.begin()), v - p->points.begin());
			}
			else
			{
				edgeMap[currEdge].polygonIdxs.push_back(polygonIdx);
			}
		}
		++polygonIdx;
	}

	bool fr = true;
	std::map<size_t, size_t> polygonsBoundaryEdges;
	for (auto e = edgeMap.begin(); e != edgeMap.end(); ++e)
	{
		const size_t numIncidentPolygons = e->second.polygonIdxs.size();
		if (numIncidentPolygons == 1)
		{
			if (polygonsBoundaryEdges.find(e->second.polygonInObjIdx) == polygonsBoundaryEdges.end())
			{
				polygonsBoundaryEdges[e->second.polygonInObjIdx] = 1;
			}
			else
			{
				polygonsBoundaryEdges[e->second.polygonInObjIdx] += 1;
			}
		}
		else if (numIncidentPolygons == 2)
		{
			const size_t i0 = e->second.polygonIdxs[0];
			const size_t i1 = e->second.polygonIdxs[1];
			double t = shadowNormals[i0].PolygonNormal.DirectionVector() * shadowNormals[i1].PolygonNormal.DirectionVector();
			const Normals::PolygonNormalData& n0 = shadowNormals[i0];
			const Normals::PolygonNormalData& n1 = shadowNormals[i1];
			Polygon3D& p0 = shadowPolygons[e->second.polygonIdxs[0]];
			Polygon3D& p1 = shadowPolygons[e->second.polygonIdxs[1]];
			if ((t >= 1.1 || t < 0) && fr)
			{
				fr = false;
				//continue;
			}
		}
	}

	size_t numDeleted = 0;
	for (auto p = polygonsBoundaryEdges.begin(); p != polygonsBoundaryEdges.end(); ++p)
	{
		
		if (p->second == 4)
		{
			const size_t currIdxToDelete = p->first - numDeleted;
			shadowPolygons.erase(shadowPolygons.begin() + currIdxToDelete);
			shadowNormals.erase(shadowNormals.begin() + currIdxToDelete);
			++numDeleted;
		}
	}*/

	return std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>>(PolygonalObject(shadowPolygons), shadowNormals);
}