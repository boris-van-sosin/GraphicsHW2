/*****************************************************************************
* Skeleton for an interface to a parser to read IRIT data files.	     *
******************************************************************************
* Written by Amit Mano						November 2008*
******************************************************************************/

#ifndef	IRIT_SKEL_H
#define	IRIT_SKEL_H

#include <stdlib.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "attribut.h"
#include "allocate.h"
#include "ip_cnvrt.h"
#include "symb_lib.h"

#include <vector>
#include "Geometry.h"

bool CGSkelProcessIritDataFiles(CString &FileNames, int NumFiles, std::vector<PolygonalObject>& outObjects = std::vector<PolygonalObject>(), int polygonFineness = 20);
void CGSkelDumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
int CGSkelGetObjectColor(IPObjectStruct *PObj, double RGB[3]);
const char *CGSkelGetObjectTexture(IPObjectStruct *PObj);
const char *CGSkelGetObjectPTexture(IPObjectStruct *PObj);
int CGSkelGetObjectTransp(IPObjectStruct *PObj, double *Transp);

bool CGSkelStoreData(IPObjectStruct *PObj);

#endif // IRIT_SKEL_H
