#ifndef COMMONWRITERS_GRIVY
#define COMMONWRITERS_GRIVY

#include "stdafx.h"
#include "CommonGeomFunctions.h"

std::string fn2offFN(std::string& filename);
void polyheWriteStats ( Polyhedron polyhe, std::string filename_0ext, CGAL::Timer& tmr, int stage, bool lastStage=false);
void nefWriteStats ( Nef_polyhedron nefPolyhe, std::string filename_0ext, CGAL::Timer& tmr, int stage, bool lastStage=false);
void writeLog (std::string filename_0ext, CGAL::Timer& tmr, std::string stagestr, bool lastStage=false);

/* ------------------- Writer functions ---------------------------*/

bool write_polyhe2ColoredOFF(Polyhedron& polyhe, std::string filename_0ext);
// Writes Polyhedron to 'filename_0ext +".OFF"'
bool write_polyhe2off(Polyhedron& polyhe, std::string filename_0ext);

// Writes Nef_polyhedron to 'filename_0ext +".OFF"'
// Converts nef to polyhe then calls write_polyhe2off
// Nef should be simple
bool write_nef2off(Nef_polyhedron& nefPolyhe, std::string filename_0ext);
#endif