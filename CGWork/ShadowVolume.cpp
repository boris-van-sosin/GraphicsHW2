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
	

	ModelAttr attr2;
	attr2.removeBackFace = true;
	attr2.Shading = SHADING_NONE;
	DrawingObject img;
	img.shadowVolume = this;
	img.active = DrawingObject::DRAWING_OBJECT_SV;
	for (auto obj = model.begin(); obj != model.end(); ++obj)
		DrawObject(img, *obj, mTotal, attr2, normals, normalsOffset, true, clip, cp);
}

PolygonalObject ShadowVolume::GenerateShadowVolume(const PolygonalModel& model, const MatrixHomogeneous& m, const std::vector<Normals::PolygonNormalData>& normals, const PolygonAdjacencyGraph& polygonAdj) const
{
	std::vector<Polygon3D> shadowPolygons;
	for (auto j = polygonAdj.begin(); j != polygonAdj.end(); ++j)
	{
		const Polygon3D& currPoly = model[j->objIdx].polygons[j->polygonInObjIdx];
		const LineSegment edge(currPoly.points[j->vertexIdx], currPoly.points[(j->vertexIdx + 1) % currPoly.points.size()]);

		if (j->polygonIdxs.size() == 1)
		{
			// boundary
		}
		else
		{
			// silhouette
			const LineSegment n0 = TransformNormal(m, normals[j->polygonIdxs[0]].PolygonNormal);
			const LineSegment n1 = TransformNormal(m, normals[j->polygonIdxs[1]].PolygonNormal);
			const Vector3D optVector(0, 0, 1);
			const Vector3D n0Vec = Vector3D(n0.p1) - Vector3D(n0.p0);
			const Vector3D n1Vec = Vector3D(n1.p1) - Vector3D(n1.p0);
			if ((n0Vec * optVector) * (n1Vec * optVector) < 0)
			{
			}
		}
	}

	return PolygonalObject(shadowPolygons);
}