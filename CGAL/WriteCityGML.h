#ifndef WRITE_CITYGML_GRIVY
#define WRITE_CITYGML_GRIVY

#include "stdafx.h"
#include "CommonGeomFunctions.h"

#include "SnapSemantics.h"

typedef std::map<std::string,std::vector<std::string>> GMLidMap;

// Overly complex indentation function to test operator overloading
class StringIndents {
private:
	friend std::ostream& operator<<(std::ostream &strm, const StringIndents &indent) {
		return strm << indent.indent; }
	int numberOfTabs;		// Current number of tabs
	std::string indent;		// Indentation string of tabs
public:
	// Constructors
	StringIndents ()		: numberOfTabs(0) {}
	StringIndents (int size): numberOfTabs(0) {set(size);}

	virtual ~StringIndents() {}
	void finalize() {}

	// Update variables
	void increment (int incr) {
		numberOfTabs += incr;
		if (incr >= 0)	for (int j = 0; j < incr; ++j)	indent.push_back('\t');
		else if (numberOfTabs > 0) indent = indent.substr(0, indent.size()+incr);
		else { indent = ""; numberOfTabs=0; }
	}

	// Get set methods
	std::string get() {return indent;}
	std::string iGet(int incr) { increment(incr); return indent; }
	void set(int size) { increment(size-numberOfTabs); }

	// Operators
	
	StringIndents& operator=(const StringIndents &other) {
		numberOfTabs = other.numberOfTabs; indent = other.indent; return *this; }
	StringIndents& operator=(const int &size) { increment(size-numberOfTabs); return *this; }
	StringIndents& operator+ (int incr) { increment (incr);  return *this; }
	StringIndents& operator- (int decr) { increment (-decr); return *this; }
	StringIndents& operator++ () { numberOfTabs++; indent.push_back('\t'); return *this; }
	StringIndents& operator-- () { increment (-1); return *this; }
	StringIndents  operator++ (int) { StringIndents prevInd = *this;
		numberOfTabs++; indent.push_back('\t'); return prevInd; }
	StringIndents  operator-- (int) { StringIndents prevInd = *this;
		increment (-1); return prevInd;	}
};

class CityGMLWriter {
private:
	std::string filename_0ext;
	std::ofstream gml_ofstream;
	PolVector exPolVector;
	bool has_BuildInstalls;
	PolVector biPolVector;
	bool has_BuildParts;
	PolVector bpPolVector;
	bool has_LoD4Rooms;
	PolVector lod4PolVector;

	GMLidMap			gmlIDs;
	GMLidMap::iterator	idIt;

	unsigned int SLDgmlID;

	StringIndents indent;		// Indentation object
public:
	// Constructors
	CityGMLWriter (std::string fn_0ext, Polyhedron& polyhe): 
			filename_0ext(fn_0ext),
			gml_ofstream(fn_0ext +".gml"),	// Open off file
			indent(),
			SLDgmlID(0)
	{
		exPolVector.push_back(polyhe);
	}
	CityGMLWriter (std::string fn_0ext, Polyhedron& polyhe,bool has_bp,PolVector& bpPolVec, bool has_bi, PolVector& biPolVec,bool has_LoD4, PolVector& lod4Vec): 
			filename_0ext(fn_0ext),
			gml_ofstream(fn_0ext +".gml"),	// Open off file
			indent(),
			has_BuildParts(has_bp),
			bpPolVector(bpPolVec),
			has_BuildInstalls(has_bi),
			biPolVector(biPolVec),
			has_LoD4Rooms(has_LoD4),
			lod4PolVector(lod4Vec),
			SLDgmlID(0)
	{
		exPolVector.push_back(polyhe);
	}



	virtual ~CityGMLWriter() {}
	void finalize() {}
		
	void set_filename(std::string fn_0ext) {filename_0ext = fn_0ext;}
	bool open() {gml_ofstream.close(); gml_ofstream.open(filename_0ext+".gml"); return gml_ofstream.is_open();}
	bool open(std::string fn_0ext) {set_filename(fn_0ext); return open();}

	bool write_CityGML(int buildLoD = 3);
	bool write_CityGMLSolids(int buildLoD = 3);
	void writeHeader();
	void writeFooter();

	void writeGeometryHeader(int kindOSolid, int partCount);
	void writeGeometryFooter(int kindOSolid);
	void writeGeometrySLD(PolVector& pVec,int LoD,bool is_buildInstall=false);
	void writeGeometry(PolVector& pVec,int kindOSolid,int LoD);

	void writeSurfaceOpening(std::string idStrt, GMLidMap& solidIDs, sfsVec& openSFSVector, std::map<int,std::vector<int>> openingMap, unsigned int surfCount,int LoD,bool is_interior);
	void writeSurfaceMember(Polyhedron::Facet_handle fh, std::string gmlID="", bool is_interior=false);

	void writeAppearance();	
};

std::string temp2FinalSem (std::string& oldSem);
void groupSemanticFacets(Polyhedron& polyhe,sfsVec& semFacetSetsVector,sfsVec& openSFSVector,std::map<int,std::vector<int>>& openingMap);

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime();

#endif