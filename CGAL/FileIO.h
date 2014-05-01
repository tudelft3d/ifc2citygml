#ifndef FILE_IO_GRIVY
#define FILE_IO_GRIVY

#include "stdafx.h"
#include "CommonGeomFunctions.h"
#include "Minkowski.h"
#include "ManifoldFix.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

/* ------------------- <OFFxReader Class> -------------------------*/
//	class OFFxReader

/* ------------------- Data handling functions --------------------*/
//  offxPair					offxLine2pair		(std::string line);
//	bool						sstream2nef			(std::stringstream& offSStream, Nef_polyhedron& nefPolyhe, bool normal=true, bool regular=true);

/* ------------------- <OFFxReader Class> -------------------------*/
class OFFxReader {
private:
	std::ifstream c_OFF_ifstream;
	std::ifstream c_OFFx_ifstream;
	std::string filename_0ext;
	int currOffLnIdx;
	int currNefIdx;
	int nbrOfLinesRead;

	Polyhedron					currPolyhe;
	Nef_polyhedron				currNefPolyhe;
	std::vector<std::string>	currInfo;// not used
	inexcludeMap				blacklist;
	inexcludeMap				whitelist;
	std::set<int>				activeWhite;
	bool						attemptFix;
	bool						splitFirst;				// Should be true, but might give wierd results with SplitNefs requireing manifolds
	std::vector<Nef_polyhedron> currNefVec;
	Kernel::FT					volume;
public:
	OFFxReader (std::string fn_0ext):
		  filename_0ext(fn_0ext)
		, c_OFF_ifstream (fn_0ext + ".OFF")
		, c_OFFx_ifstream(fn_0ext + ".OFFx")
		, currOffLnIdx(0)
		, currNefIdx(0)
		, nbrOfLinesRead(0)
		, attemptFix(true)
		, splitFirst(true)
	{}
	virtual ~OFFxReader() {}
	void finalize() {}

	bool is_open();		// Check if ifstreams are open
	bool good();		// Check if ifstreams are open and good
	void close();		// Check if ifstreams are open if so they are closed.

	// Read next polyhedra+info from OFFx and OFF
	// Updates curr variables
	// Fails at EoF, read error or failed nef creation. Use good() to check for EoF
	bool next();
	void skip();
	void skip(int skipAmount){for (int i=0;i<skipAmount;i++) skip();}
	void eraseCurrent();	// Nullify current data
	void addToBlacklist(int position, std::string blackStr);
	void addToWhitelist(int position, std::string whiteStr);

	// Gets
	Polyhedron					get_currentPolyhe()			{return currPolyhe;}
	Nef_polyhedron				get_currentNef()			{return currNefPolyhe;}
	std::vector<std::string>	get_currentInfo()			{return currInfo;}
	int							get_currentNefIdx()			{return currNefIdx;}
	int							get_nbrOfLinesRead()		{return nbrOfLinesRead;}
	Kernel::FT					get_volume()				{return volume;}
	// Sets
	void						set_attemptFix(bool fix)	{attemptFix=fix;}
	void						set_splitFirst(bool split)	{splitFirst=split;}

};


/* ------------------- Data handling functions --------------------*/

// Formats OFFx-line in a tokenized string vector and (beginLn,endLn) int vector for reading OFF
offxPair offxLine2pair(std::string line);

// Creates a Nef_polyhedron from off-stringstream
// Regularization & Normalizing border are optional
// Returns true if succesfull
bool sstream2nef(std::stringstream& offSStream, Nef_polyhedron& nefPolyhe,bool quite = true, bool normal=true, bool regular=true);

#endif