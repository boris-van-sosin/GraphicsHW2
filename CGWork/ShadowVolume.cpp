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

double WorldDiagonal(const BoundingBox& bbox)
{
	return sqrt((bbox.maxX - bbox.minX)*(bbox.maxX - bbox.minX) +
		(bbox.maxY - bbox.minY)*(bbox.maxY - bbox.minY) +
		(bbox.maxZ - bbox.minZ)*(bbox.maxX - bbox.minZ));
}

ShadowVolume::ShadowVolume()
	: _width(1), _height(1), _stencil(NULL), _stencil2(NULL), _shadowLength(1)
{
}

ShadowVolume::ShadowVolume(size_t w, size_t h, const LightSource& ls, const BoundingBox& wbox)
	: ShadowVolume(w, h, ls, WorldDiagonal(wbox))
{}

ShadowVolume::ShadowVolume(size_t w, size_t h, const LightSource& ls)
	: ShadowVolume(w, h, ls, 0.1)
{}

ShadowVolume::ShadowVolume(size_t w, size_t h, const LightSource& ls, double shadowLength)
	: _width(w), _height(h), _lightSource(ls), _shadowLength(shadowLength)
{
	if ((w | h) == 0)
	{
		_width = _height = 1;
	}
	//_stencil = new std::set<ShadowEvent>[_height * _width];
	//_stencil2 = new std::map<size_t, ShadowLimits>[_height * _width];
}

ShadowVolume::ShadowVolume(const ShadowVolume& other)
	: _width(other._width), _height(other._height), _lightSource(other._lightSource), _shadowLength(other._shadowLength)
{
	//_stencil = new std::set<ShadowEvent>[_height * _width];
	//_stencil2 = new std::map<size_t, ShadowLimits>[_height * _width];
	for (int i = 0; i < _height*_width; ++i)
	{
		//_stencil[i] = other._stencil[i];
		//_stencil2[i] = other._stencil2[i];
	}
}

ShadowVolume::~ShadowVolume()
{
	//delete[] _stencil;
	//delete[] _stencil2;
}

ShadowVolume& ShadowVolume::operator=(const ShadowVolume& other)
{
	if (this == &other)
		return *this;

	_lightSource = other._lightSource;

	_shadowLength = other._shadowLength;

	SetSize(other._width, other._height);
	for (int i = 0; i < _height*_width; ++i)
	{
		//_stencil[i] = other._stencil[i];
		//_stencil2[i] = other._stencil2[i];
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
	if ((w | h) == 0)
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
	//delete[] _stencil;
	//delete[] _stencil2;
	//_stencil = new std::set<ShadowEvent>[_height*_width];
	//_stencil2 = new std::map<size_t, ShadowLimits>[_height*_width];
}

void ShadowVolume::SetWorldBox(const BoundingBox& wbox)
{
	_shadowLength = WorldDiagonal(wbox);
}

void ShadowVolume::Clear()
{
	for (size_t i = 0; i < _height*_width; ++i)
	{
		//_stencil[i].clear();
		//_stencil2[i].clear();
	}
	_boundingBoxes.clear();
	_shadowSurfaces.clear();
	_shadowSurfaceNormals.clear();
	_currPolyId = 0;
	_currModelId = 0;
}

void ShadowVolume::SetPixel(int x, int y, double z, COLORREF id)
{
	if (x < 0 || y < 0 || x >= _width || y >= _height)
	{
		return;
	}
	//_stencil[y*_width + x].insert(ShadowEvent(_currShadowMode, z, id));
	return;
	std::map<size_t, ShadowLimits>& currPixel = _stencil2[y*_width + x];
	if (currPixel.find(id) == currPixel.end())
	{
		currPixel[id] = ShadowLimits(z, z);
	}
	else
	{
		currPixel[id].Update(z);
	}
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

ShadowVolume::ShadowLimits::ShadowLimits()
	: _minZ(0), _maxZ(0)
{}

ShadowVolume::ShadowLimits::ShadowLimits(double min, double max)
	: _minZ(min), _maxZ(max)
{}

void ShadowVolume::ShadowLimits::Update(double z)
{
	if (z < _minZ)
		_minZ = z;
	if (z > _maxZ)
		_maxZ = z;
}

bool ShadowVolume::IsPixelLit(size_t x, size_t y, double z, const Point3D& pt) const
{
	if (y * _width + x >= _width * _height)
		return true;
	/*
	int shadowNesting = 0;
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
	return shadowNesting <= 0;*/

	//const Vector3D ray = -(_lightSource._type == LightSource::PLANE ? _lightSource.Direction() : (Point3D(_lightSource._origin) - pt).Normalized());
	//const Point3D pt2 = pt;
	const Point3D pt2(x, y, z);
	const Vector3D ray(0, 0, -1);
	int shadowNesting = 0;
	bool shadowCrossings = true;
	for (auto i = _shadowSurfaces.begin(); i != _shadowSurfaces.end(); ++i)
	{
		std::pair<bool, double> bi = PolygonIntersection::PolygonRayIntersectionParam(pt2, ray, *i);
		if (bi.first && bi.second > SV_COMPUTATION_EPSILON)
		{
			if (_shadowSurfaceNormals[i - _shadowSurfaces.begin()].PolygonNormal.DirectionVector().z < 0)
			{
				--shadowNesting;
			}
			else if (_shadowSurfaceNormals[i - _shadowSurfaces.begin()].PolygonNormal.DirectionVector().z > 0)
			{
				++shadowNesting;
			}
			shadowCrossings = !shadowCrossings;
		}
	}
	return shadowCrossings;
	return shadowNesting <= 0;
	/*for (auto it = currPixel2.begin(); it != currPixel2.end(); ++it)
	{
		//if (z < it->second._minZ || z > it->second._maxZ)
		//	continue;

		const size_t currIdx = it->first;
		for (auto subBox = _boundingBoxes[currIdx].begin(); subBox != _boundingBoxes[currIdx].end(); ++subBox)
		{
			bool boxIntersection = false;
			for (auto boxPoly = subBox->_bboxObj.polygons.begin(); boxPoly != subBox->_bboxObj.polygons.end(); ++boxPoly)
			{
				std::pair<bool, double> bi = PolygonIntersection::PolygonRayIntersectionParam(pt, ray, *boxPoly);
				if (bi.first)
				{
					boxIntersection = true;
					break;
				}
			}
			if (boxIntersection)
			{
				for (auto modelPoly = subBox->_polygons.begin(); modelPoly != subBox->_polygons.end(); ++modelPoly)
				{
					std::pair<bool, double> bi = PolygonIntersection::PolygonRayIntersectionParam(pt, ray, **modelPoly);
					if (bi.first && bi.second > SV_COMPUTATION_EPSILON)
					{
						return false;
					}
				}
			}
		}
	}
	return true;*/
}

void ShadowVolume::ProcessModel(const PolygonalModel& model, const MatrixHomogeneous& mTotal, const std::vector<Normals::PolygonNormalData>& normals, bool clip, const ClippingPlane& cp, const PolygonAdjacencyGraph& polygonAdj)
{
	std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> shadowObj = GenerateShadowVolume(model, normals, polygonAdj);
	shadowObj.first = mTotal * shadowObj.first;
	for (auto i = shadowObj.second.begin(); i != shadowObj.second.end(); ++i)
		*i = mTotal * *i;
	//_models.push_back(&model);
	PartitionIntoBBoxes(model);
	_shadowSurfaces.insert(_shadowSurfaces.end(), shadowObj.first.polygons.begin(), shadowObj.first.polygons.end());
	_shadowSurfaceNormals.insert(_shadowSurfaceNormals.end(), shadowObj.second.begin(), shadowObj.second.end());

	for (auto p = shadowObj.first.polygons.begin(); p != shadowObj.first.polygons.end(); ++p)
	{
		p->color = _currPolyId;
		p->colorValid = true;
		++_currPolyId;
	}

	ModelAttr attr2;
	attr2.removeBackFace = BACKFACE_REMOVE_BACK;
	attr2.Shading = SHADING_NONE;
	attr2.forceColor = true;
	attr2.color = _currModelId++;
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
	std::vector<HomogeneousPoint> pointsObserved;
	pointsObserved.reserve(1000);
	for (auto j = polygonAdj.begin(); j != polygonAdj.end(); ++j)
	{
		const Polygon3D& currPoly = model[j->objIdx].polygons[j->polygonInObjIdx];
		LineSegment edge(currPoly.points[j->vertexIdx], currPoly.points[(j->vertexIdx + 1) % currPoly.points.size()]);

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

			HomogeneousPoint far0((Point3D(edge.p0) + (ray0 * _shadowLength)));
			HomogeneousPoint far1((Point3D(edge.p1) + (ray1 * _shadowLength)));

			for (auto prevP = pointsObserved.begin(); prevP != pointsObserved.end(); ++prevP)
			{
				if ((Point3D(edge.p0) - Point3D(*prevP)).Norm() <= SV_COMPUTATION_EPSILON)
				{
					edge.p0 = *prevP;
				}
				if ((Point3D(edge.p1) - Point3D(*prevP)).Norm() <= SV_COMPUTATION_EPSILON)
				{
					edge.p1 = *prevP;
				}
				if ((Point3D(far0) - Point3D(*prevP)).Norm() <= SV_COMPUTATION_EPSILON)
				{
					far0 = *prevP;
				}
				if ((Point3D(far1) - Point3D(*prevP)).Norm() <= SV_COMPUTATION_EPSILON)
				{
					far1 = *prevP;
				}
			}

			pointsObserved.push_back(edge.p0);
			pointsObserved.push_back(edge.p1);
			pointsObserved.push_back(far0);
			pointsObserved.push_back(far1);

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

	
	return std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>>(PolygonalObject(shadowPolygons), shadowNormals);
}

void ShadowVolume::PartitionIntoBBoxes(const PolygonalModel& model)
{
	_boundingBoxes.push_back(std::vector<VolumeMapping>());
	_boundingBoxes.back().push_back(VolumeMapping());
	//
	
	PolygonalObject& currBBox = _boundingBoxes.back()[0]._bboxObj;
	std::vector<const Polygon3D*>& currPolygonList = _boundingBoxes.back()[0]._polygons;
	currBBox = BoundingBox::OfObjects(model).ToObject(true);
	for (auto i = model.begin(); i != model.end(); ++i)
	{
		for (auto j = i->polygons.begin(); j != i->polygons.end(); ++j)
		{
			currPolygonList.push_back(&(*j));
		}
	}
}

