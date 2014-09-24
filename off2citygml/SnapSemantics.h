#ifndef SNAP_SEMANTICS_GRIVY
#define SNAP_SEMANTICS_GRIVY

#include "stdafx.h"
#include "CommonGeomFunctions.h"

void set_semantic_InteriorLoD4(Polyhedron& polyhe);
bool indirectlyTouchingFindSem(std::string findSem,std::set<Polyhedron::Facet_handle>& fhSet);
void connectedSemFacets(Polyhedron::Facet_handle fh, std::string sem="-1",bool allExceptThisSem=false, std::set<Polyhedron::Facet_handle>& fhSet = std::set<Polyhedron::Facet_handle>(),bool checkCoPlanar=false);
double coplanarSem(Polyhedron::Facet_handle fh, std::set<Polyhedron::Facet_handle>& fhSet);
void remove_singularSemantics(Polyhedron& exteriorPolyhe);
void apply_semanticRequirements(Polyhedron& exteriorPolyhe);
void set_semantic_AABB_C2V(Polyhedron& exteriorPolyhe,PolVector& polyVec);
#endif