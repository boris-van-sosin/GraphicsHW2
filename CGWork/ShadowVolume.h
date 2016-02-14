#pragma once

#include <set>
#include <vector>
#include "Geometry.h"
#include "GeometricTransformations.h"
#include "DrawingSupport.h"

class ShadowVolume
{
public:
	ShadowVolume();
	~ShadowVolume();

	enum ShadowEventType { ShadowEnter, ShadowExit};

	class ShadowEvent
	{
	public:
		ShadowEventType Type;
		double z;
		bool operator < (const ShadowEvent& other) const;
	};

	size_t GetHeight() const;
	size_t GetWidth() const;
	void SetSize(size_t w, size_t h);
	void Clear();

	bool IsPixelLit(size_t x, size_t y, double z) const;

	void ProcessModel(const PolygonalObject& obj, const MatrixHomogeneous& mTotal, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, size_t normalsOffset, bool fillPolygons, bool clip, const ClippingPlane& cp);

private:
	std::set<ShadowEvent>* Stencil;
	LightSource _lightSource;
};

