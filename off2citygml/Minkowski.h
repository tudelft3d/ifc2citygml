#ifndef MINKOWSKI_GRIVY
#define MINKOWSKI_GRIVY

#include "stdafx.h"
#include "FileIO.h"
#include "CommonGeomFunctions.h"

Nef_polyhedron makeExteriorManifold(Nef_polyhedron extManiNef, std::vector<Nef_polyhedron>& nefVec=std::vector<Nef_polyhedron>(),double fixSize=-1, bool doCut=DO_CUT_FIX);
Nef_polyhedron separateFirstNef(Nef_polyhedron& nefPolyhe, std::vector<Nef_polyhedron>& nefVec);
void shrinkMinkowski(Nef_polyhedron& nefPolyhe, Nef_polyhedron& robot);
bool closing_Minkowski(Nef_polyhedron& fusedNefPolyhe, Polyhedron& exteriorPolyhe, double offsetMagnitude, std::vector<Nef_polyhedron>& bpVector,bool useFacetMink=false);
Nef_polyhedron minkowskiSum_Of_Facets (Nef_polyhedron minkowskiNef, Nef_polyhedron& robot, bool subtract=false);
Nef_polyhedron minkowskiSum_of_triangle(pointVector& facePoints,Nef_polyhedron& robot);
Nef_polyhedron makeRobot (double offsetMagnitude, Nef_polyhedron& uNef=Nef_polyhedron(), int shape=0, Vector_3 faceNormal=Vector_3(), bool rotateRobot=false);
void get_unitBox (Nef_polyhedron& uNef);

#endif