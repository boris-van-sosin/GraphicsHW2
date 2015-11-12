#ifndef IRIT_ADAPTER_H
#define IRIT_ADAPTER_H

#include "Geometry.h"
#include <vector>

struct IPObjectStruct;

namespace IritAdapter
{
	class ConvertError {};
	std::vector<PolygonalObject> Convert(IPObjectStruct* iritObjects);
};

#endif