#ifndef MERGE_COPLANAR_FACETS_GRIVY
#define MERGE_COPLANAR_FACETS_GRIVY

#include "stdafx.h"
#include "CommonGeomFunctions.h"

// Checks if there are two disconnected linestrings which both have the 2 coplanar facets
bool joinCreatesNoHole (Polyhedron::Halfedge_handle& heH);

bool triDoesNotIntersectFacet (Polyhedron::Halfedge_handle& heH, bool isOpposite=false);

void mergeColinear(Polyhedron& p);

//no border
//no degen
void mergeCoplanar(Polyhedron& p,bool step2=false);
#endif