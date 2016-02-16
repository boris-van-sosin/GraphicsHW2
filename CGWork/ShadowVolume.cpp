#include "ShadowVolume.h"

#include "Drawing.h"

ShadowVolume::ShadowVolume()
	: _width(0), _height(0), _stencil(NULL)
{
}

ShadowVolume::ShadowVolume(size_t w, size_t h, const LightSource& ls)
	: _width(w), _height(h), _lightSource(ls)
{
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
}

void ShadowVolume::SetPixel(int x, int y, double z)
{
	if (x < 0 || y < 0 || x >= _width || y >= _height)
	{
		return;
	}
	_stencil[y*_width + x].insert(ShadowEvent(_currShadowMode, z));
}

void ShadowVolume::SetLightSource(const LightSource& ls)
{
	_lightSource = ls;
}

ShadowVolume::ShadowEvent::ShadowEvent(ShadowEventType t, double z)
	: _type(t), _z(z)
{
}

ShadowVolume::ShadowEvent::~ShadowEvent()
{
}

bool ShadowVolume::ShadowEvent::operator < (const ShadowEvent& other) const
{
	return _z < other._z;
}

bool ShadowVolume::IsPixelLit(size_t x, size_t y, double z) const
{
	int shadowNesting = 0;
	if (y * _width + x >= _width * _height)
		return true;

	const std::set<ShadowEvent>& currPixel = _stencil[y * _width + x];
	for (auto i = currPixel.begin(); i != currPixel.end(); ++i)
	{
		if ((i->_type == ShadowEnter && i->_z >= z) || (i->_type == ShadowExit && i->_z > z))
			break;

		if (i->_type == ShadowEnter)
			++shadowNesting;
		else
			--shadowNesting;
	}
	if (shadowNesting > 0)
		return false;
	return true;
	return shadowNesting <= 0;
}

void ShadowVolume::ProcessModel(const PolygonalModel& model, const MatrixHomogeneous& mTotal, const std::vector<Normals::PolygonNormalData>& normals, bool clip, const ClippingPlane& cp, const PolygonAdjacencyGraph& polygonAdj)
{
	const std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> shadowObj = GenerateShadowVolume(model, mTotal, normals, polygonAdj);

	ModelAttr attr2;
	attr2.removeBackFace = BACKFACE_REMOVE_BACK;
	attr2.Shading = SHADING_NONE;
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

std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> ShadowVolume::GenerateShadowVolume(const PolygonalModel& model, const MatrixHomogeneous& m, const std::vector<Normals::PolygonNormalData>& normals, const PolygonAdjacencyGraph& polygonAdj) const
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
			const Vector3D n0Vec = Vector3D(n0.p1) - Vector3D(n0.p0);
			const Vector3D n1Vec = Vector3D(n1.p1) - Vector3D(n1.p0);
			if ((n0Vec * optVector) * (n1Vec * optVector) < 0)
			{
				addEdge = true;
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
		}
	}

	return std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>>(PolygonalObject(shadowPolygons), shadowNormals);
}