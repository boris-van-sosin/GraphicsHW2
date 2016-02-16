#include "ShadowVolume.h"

#include "Drawing.h"

ShadowVolume::ShadowVolume(size_t w, size_t h, const LightSource& ls)
	: _width(w), _height(h), _lightSource(ls)
{
	_stencil = new std::set<ShadowEvent>[_height * _width];
}

ShadowVolume::~ShadowVolume()
{
	delete[] _stencil;
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

void ShadowVolume::ProcessModel(const PolygonalModel& model, const MatrixHomogeneous& mTotal, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, size_t normalsOffset, bool clip, const ClippingPlane& cp, const PolygonAdjacencyGraph& polygonAdj)
{
	const std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> shadowObj = GenerateShadowVolume(model, mTotal, normals, polygonAdj);

	ModelAttr attr2;
	attr2.removeBackFace = BACKFACE_REMOVE_BACK;
	attr2.Shading = SHADING_NONE;
	DrawingObject img;
	img.shadowVolume = this;
	img.active = DrawingObject::DRAWING_OBJECT_SV;

	_currShadowMode = ShadowEnter;
	DrawObject(img, shadowObj.first, mTotal, attr2, shadowObj.second, 0, false, clip, cp);
	/*for (auto obj = model.begin(); obj != model.end(); ++obj)
	{
		DrawObject(img, *obj, mTotal, attr2, normals, normalsOffset, true, clip, cp);
	}*/

	attr2.removeBackFace = BACKFACE_REMOVE_FRONT;
	_currShadowMode = ShadowExit;
	DrawObject(img, shadowObj.first, mTotal, attr2, shadowObj.second, 0, false, clip, cp);
	/*for (auto obj = model.begin(); obj != model.end(); ++obj)
	{
		DrawObject(img, *obj, mTotal, attr2, normals, normalsOffset, true, clip, cp);
	}*/
}

std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> ShadowVolume::GenerateShadowVolume(const PolygonalModel& model, const MatrixHomogeneous& m, const std::vector<Normals::PolygonNormalData>& normals, const PolygonAdjacencyGraph& polygonAdj) const
{
	std::vector<Polygon3D> shadowPolygons;
	std::vector<Normals::PolygonNormalData> shadowNormals;
	double shadowLength = 100;
	for (auto j = polygonAdj.begin(); j != polygonAdj.end(); ++j)
	{
		const Polygon3D& currPoly = model[j->objIdx].polygons[j->polygonInObjIdx];
		const LineSegment edge(currPoly.points[j->vertexIdx], currPoly.points[(j->vertexIdx + 1) % currPoly.points.size()]);

		bool addEdge = false;

		if (j->polygonIdxs.size() == 1)
		{
			// boundary
			addEdge = true;
		}
		else
		{
			// silhouette
			const LineSegment n0 = TransformNormal(m, normals[j->polygonIdxs[0]].PolygonNormal);
			const LineSegment n1 = TransformNormal(m, normals[j->polygonIdxs[1]].PolygonNormal);
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

			polyPoints.push_back(edge.p0);
			polyPoints.push_back(edge.p1);

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

			polyPoints.push_back(far1);
			polyPoints.push_back(far0);

			shadowPolygons.push_back(Polygon3D(polyPoints));
			const HomogeneousPoint centroid = shadowPolygons.back().AreaAndCentroid().second;
			shadowNormals.push_back(Normals::PolygonNormalData(LineSegment(centroid, HomogeneousPoint(Point3D(centroid) + shadowPolygons.back().Normal()))));
		}
	}

	return std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>>(PolygonalObject(shadowPolygons), shadowNormals);
}