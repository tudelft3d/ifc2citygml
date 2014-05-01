#include "FileIO.h"

/* ------------------- OFFxReader Class ---------------------------*/ 

// Check if ifstreams are open
bool OFFxReader::is_open() {
	return (c_OFF_ifstream.is_open() && c_OFFx_ifstream.is_open());
}

// Check if ifstreams are open and good
bool OFFxReader::good() {
	return (c_OFF_ifstream.good() && c_OFFx_ifstream.good());
}

// Check if ifstreams are open if so they are closed.
void OFFxReader::close() {
	if (c_OFFx_ifstream.is_open())	c_OFFx_ifstream.close();
	if (c_OFF_ifstream.is_open())	c_OFF_ifstream.close();
}

void OFFxReader::skip() {
	std::string cOFFxLine;										// Init offx read string
	while(std::getline(c_OFFx_ifstream,cOFFxLine)) {			// Read OFFx line
		nbrOfLinesRead++;
		if (!cOFFxLine.empty()) {								// Check if not empty
			offxPair currOffxPair = offxLine2pair(cOFFxLine);	// Convert line to pair of string & int vectors

			int beginOffLn = (currOffxPair.second)[0];			// Get index to OFF starting line
			int endOffLn = (currOffxPair.second)[1];			// Get index to OFF end line
			if (beginOffLn != currOffLnIdx) std::cerr << "ERROR: Off Line indexes out of sync. Curr: "<<currOffLnIdx<<"OFFx: "<<beginOffLn<< std::endl;
			
			for (;currOffLnIdx < endOffLn; currOffLnIdx++) {	// Read #(endLn-beginLn) lines from OFF
				std::string cOFFxLine;							// Init OFF read string
				if (std::getline(c_OFF_ifstream,cOFFxLine)) {	// Read line		
				} else std::cerr << "ERROR: Something wrong while reading file: " << filename_0ext << ".OFF" << " at line number: " << currOffLnIdx <<std::endl;
			}
			break;														// Break if read one line from OFFx succesfully 
		} else std::cerr << "WARNING: Empty line in file: " << filename_0ext << ".OFFx" <<std::endl;	
	}
}


// Read next polyhedra+info from OFFx and OFF
// Updates curr variables
// Fails at EoF, read error or failed nef creation. Use good() to check for EoF
bool OFFxReader::next() {
	Polyhedron		polyhe;										// Init polyhe
	Nef_polyhedron	nefPolyhe;									// Init nef
	offxPair currOffxPair;									// OFFx output for 1 polyhedron
	std::stringstream currOffSStream;						// OFF  output for 1 polyhedron
	std::string cOFFxLine;									// Init offx read string

	if (currNefVec.size()==0) {
		eraseCurrent();											// Erase current
		
		while(std::getline(c_OFFx_ifstream,cOFFxLine)) {		// Read OFFx line
			nbrOfLinesRead++;
			if (!cOFFxLine.empty()) {							// Check if not empty
				currOffxPair = offxLine2pair(cOFFxLine);		// Convert line to pair of string & int vectors

				int beginOffLn = (currOffxPair.second)[0];		// Get index to OFF starting line
				int endOffLn = (currOffxPair.second)[1];		// Get index to OFF end line
				if (beginOffLn != currOffLnIdx) std::cerr << "ERROR: Off Line indexes out of sync. Curr: "<<currOffLnIdx<<"OFFx: "<<beginOffLn<< std::endl;
			
				if(blacklist.size() < currOffxPair.first.size()) blacklist.resize(currOffxPair.first.size());
				if(whitelist.size() < currOffxPair.first.size()) whitelist.resize(currOffxPair.first.size());

				bool skip = false;								
				if (endOffLn-beginOffLn <=2) {					// Skip if empty OFF
					skip=true;
				}else {											// Skip to next line when on black list or not on whitelist
					for (int i=0;i<(int)currOffxPair.first.size();i++) {
						if ((i<(int)blacklist.size() && blacklist[i].find(currOffxPair.first[i]) != blacklist[i].end()) ||
							(activeWhite.find(i)!= activeWhite.end() && whitelist[i].find(currOffxPair.first[i]) == whitelist[i].end())) {
							skip=true;	break;
						}
					}
				}

				for (;currOffLnIdx < endOffLn; currOffLnIdx++) {			// Read #(endLn-beginLn) lines from OFF
					std::string cOFFxLine;									// Init OFF read string
					if (std::getline(c_OFF_ifstream,cOFFxLine) && !skip) {	// Read line		
						currOffSStream << cOFFxLine <<std::endl;			// Put whole line in stringstream
					} else if (!skip) std::cerr << "ERROR: Something wrong while reading file: " << filename_0ext << ".OFF" << " at line number: " << currOffLnIdx <<std::endl;
				}
				if (skip) {
					std::cerr << "Skipped "<<nbrOfLinesRead<<" "<<currOffxPair.first[0]<<" due to black or white list." << std::endl;
					currOffxPair.first.clear();								// Clear temp data
					continue;
				}
				break;														// Break if read one line from OFFx succesfully 
			} else std::cerr << "WARNING: Empty line in file: " << filename_0ext << ".OFFx" <<std::endl;	
		}
		if (currOffxPair.first.empty()) {											// Probably EoF reached
			// close??
			return false;
		}

		currInfo = currOffxPair.first;												// Store nef info
		sstream2nef(currOffSStream, nefPolyhe);										// Create polyhe & nef from stringstream
		if (nefPolyhe.number_of_volumes()<2 ||!nefPolyhe.is_valid()) return next();									// Skip empty input
	} else {
		currPolyhe.clear();			// Erase current polyhe
		currNefPolyhe.clear();		// Erase current NEF, but leave info
	}

	std::set<std::string> alwaysFat;
	alwaysFat.insert("Window");
	alwaysFat.insert("Door");	

	if (currInfo[0] != "Install")	{										// BI's do not need to be manifolds yet

		if (nefPolyhe.number_of_volumes() <2) {								// Dilate non solid geoms
			std::cerr << "WARNING: Geometry is not solid, will make it fat. -> " << cOFFxLine  << std::endl;
			Nef_polyhedron robot;
			makeRobot(FIX_MANIFOLD_SIZE, robot);		
			nefPolyhe.regularization();
			nefPolyhe = CGAL::minkowski_sum_3(nefPolyhe,robot);
		}

		if (splitFirst) {
			if (currNefVec.size()==0 && nefPolyhe.number_of_volumes() >2)	// split if there is a new nefpolyhe with multiple volumes (2=1vol)
				currNefVec = splitNefs (nefPolyhe,false);
			if (currNefVec.size()!=0) {										// do not replace nefs with 0 or 1 volume
				nefPolyhe  = currNefVec.back();
				currNefVec.pop_back();
			}
		}
		if (alwaysFat.find(currInfo[0])!=alwaysFat.end()) {				// Make sure openings do not desolve into the wall REPLACE BY DORPEL THING << whut is this?
			//Point_3 cog2 = comp_cog(nefPolyhe);
			make2Manifold(nefPolyhe,FIX_MANIFOLD_SIZE,false);
			nefPolyhe.convert_to_polyhedron(polyhe);
			Point_3 cog = comp_cog(polyhe);
			//std::cout <<cog<<std::endl<<cog2<<std::endl;
			Transformation toO(CGAL::TRANSLATION, Vector_3(cog,CGAL::ORIGIN)); 
			Transformation scale(CGAL::SCALING, 1+FIX_MANIFOLD_SIZE);
			Transformation fromO(CGAL::TRANSLATION, Vector_3(CGAL::ORIGIN,cog)); 
			nefPolyhe.transform(fromO*scale*toO);
		}
		if ( !nef2polyhe(nefPolyhe,polyhe) ) {			
			if (attemptFix) {
				std::cerr << "WARNING: Geometry is not simple, will fix it.. -> " << cOFFxLine  << std::endl;
				double	tapeSize = FIX_MANIFOLD_SIZE;
			
				//if (nefPolyhe.number_of_volumes() >=2) {
					//Nef_polyhedron robot;
					//makeRobot(tapeSize, robot);		
					//nefPolyhe.regularization();
					//nefPolyhe = CGAL::minkowski_sum_3(nefPolyhe,robot); // would require more mnifolds fixing and splitting etc
				//} else 
					
				//	nefPolyhe = convexFix(nefPolyhe, tapeSize);			// too rigorous

				make2Manifold(nefPolyhe, tapeSize,false);
			}
			if (!attemptFix || !nef2polyhe(nefPolyhe,polyhe)) {		//convert to polyhe
				std::cerr << "ERROR: Creating Nef_polyhedron "<< currNefIdx<<"/"<<nbrOfLinesRead<<" failed: " << cOFFxLine  << std::endl;
				return next();															// Return failure
			}
		}
	}
	std::string sem = currInfo[0];//boost::algorithm::join(currOffxPair.first, " ");
	for (Polyhedron::Facet_iterator fIt = polyhe.facets_begin(); fIt != polyhe.facets_end(); ++fIt)
		fIt->semanticBLA = sem;			
	currPolyhe	  = polyhe;												// Store polyhe
	currNefPolyhe = nefPolyhe;											// Store NEF
	//std::cout << comp_volume(nefPolyhe) <<std::endl; //does not work
	volume = comp_volume(polyhe);										// 0 for BI's
	++currNefIdx;														// Increment curr nef index
	return true;														// Return succes
}

// Nullify current data
void OFFxReader::eraseCurrent() {
	currPolyhe.clear();			// Erase current polyhe
	currNefPolyhe.clear();		// Erase current NEF
	currInfo.clear();			// Erase current info
}
	
void OFFxReader::addToBlacklist(int position, std::string blackStr) {
	if((int)blacklist.size() < position+1) blacklist.resize(position+1);
	blacklist[position].insert(blackStr);
}

void OFFxReader::addToWhitelist(int position, std::string whiteStr) {
	if((int)whitelist.size() < position+1) whitelist.resize(position+1);
	whitelist[position].insert(whiteStr);
	activeWhite.insert(position);
}


/* ------------------- Data handling functions ---------------------------*/

// Formats OFFx-line in a tokenized string vector and (beginLn,endLn) int vector for reading OFF
offxPair offxLine2pair(std::string line) {
	std::vector<std::string> tokenizedLine;						// vector of tokenized current line
	boost::split(tokenizedLine, line, boost::is_any_of(" "));	// tokenize line on " " // #include <boost/algorithm/string.hpp>

	std::vector<int> beginEndVector;							// Start and last line of polyhe in OFF file
	int endLn (stoi(tokenizedLine.back()));						// get last as int
	tokenizedLine.pop_back();									// pop last
	beginEndVector.push_back(stoi(tokenizedLine.back()));		// get new last as int + add (beginLn) to vector
	beginEndVector.push_back(endLn);							// add (endLn) to vector
	tokenizedLine.pop_back();									// pop new last

	return offxPair (tokenizedLine,beginEndVector);				// return paired string & int vector
}

// Creates a Nef_polyhedron from off-stringstream
// Regularization & Normalizing border are optional
// Returns true if succesfull
bool sstream2nef(std::stringstream& offSStream, Nef_polyhedron& nefPolyhe,bool quite, bool normal, bool regular) {
	Polyhedron polyhe;											// Init polyhe
	bool success = true;										// Init succes boolean

	CGAL::OFF_to_nef_3(offSStream, nefPolyhe);				// Create nef from stringstream
	if (regular) nefPolyhe.regularization();				// Regularize nef
	if (nefPolyhe.is_simple()) {							// Check if simple
		if (nefPolyhe.is_valid()) {							// Check is valid
			return true;
		} else if (!quite) std::cerr << "WARNING: Polyhedron is not valid (stream2nef)" << std::endl;
	} else if (!quite) std::cerr << "WARNING: Polyhedron is not simple (stream2nef)" << std::endl;
	return false;												// Return true if succes
}





// OLD FIX CODE changes mostly around line 140

//
//#include "FileIO.h"
//
///* ------------------- OFFxReader Class ---------------------------*/ 
//
//// Check if ifstreams are open
//bool OFFxReader::is_open() {
//	return (c_OFF_ifstream.is_open() && c_OFFx_ifstream.is_open());
//}
//
//// Check if ifstreams are open and good
//bool OFFxReader::good() {
//	return (c_OFF_ifstream.good() && c_OFFx_ifstream.good());
//}
//
//// Check if ifstreams are open if so they are closed.
//void OFFxReader::close() {
//	if (c_OFFx_ifstream.is_open())	c_OFFx_ifstream.close();
//	if (c_OFF_ifstream.is_open())	c_OFF_ifstream.close();
//}
//
//void OFFxReader::skip() {
//	std::string cOFFxLine;										// Init offx read string
//	while(std::getline(c_OFFx_ifstream,cOFFxLine)) {			// Read OFFx line
//		nbrOfLinesRead++;
//		if (!cOFFxLine.empty()) {								// Check if not empty
//			offxPair currOffxPair = offxLine2pair(cOFFxLine);	// Convert line to pair of string & int vectors
//
//			int beginOffLn = (currOffxPair.second)[0];			// Get index to OFF starting line
//			int endOffLn = (currOffxPair.second)[1];			// Get index to OFF end line
//			if (beginOffLn != currOffLnIdx) std::cerr << "ERROR: Off Line indexes out of sync. Curr: "<<currOffLnIdx<<"OFFx: "<<beginOffLn<< std::endl;
//			
//			for (;currOffLnIdx < endOffLn; currOffLnIdx++) {	// Read #(endLn-beginLn) lines from OFF
//				std::string cOFFxLine;							// Init OFF read string
//				if (std::getline(c_OFF_ifstream,cOFFxLine)) {	// Read line		
//				} else std::cerr << "ERROR: Something wrong while reading file: " << filename_0ext << ".OFF" << " at line number: " << currOffLnIdx <<std::endl;
//			}
//			break;														// Break if read one line from OFFx succesfully 
//		} else std::cerr << "WARNING: Empty line in file: " << filename_0ext << ".OFFx" <<std::endl;	
//	}
//}
//
//
//// Read next polyhedra+info from OFFx and OFF
//// Updates curr variables
//// Fails at EoF, read error or failed nef creation. Use good() to check for EoF
//bool OFFxReader::next() {
//	Polyhedron		polyhe;										// Init polyhe
//	Nef_polyhedron	nefPolyhe;									// Init nef
//	offxPair currOffxPair;									// OFFx output for 1 polyhedron
//	std::stringstream currOffSStream;						// OFF  output for 1 polyhedron
//	std::string cOFFxLine;									// Init offx read string
//
//	if (currNefVec.size()==0) {
//		eraseCurrent();											// Erase current
//		
//		while(std::getline(c_OFFx_ifstream,cOFFxLine)) {		// Read OFFx line
//			nbrOfLinesRead++;
//			if (!cOFFxLine.empty()) {							// Check if not empty
//				currOffxPair = offxLine2pair(cOFFxLine);		// Convert line to pair of string & int vectors
//
//				int beginOffLn = (currOffxPair.second)[0];		// Get index to OFF starting line
//				int endOffLn = (currOffxPair.second)[1];		// Get index to OFF end line
//				if (beginOffLn != currOffLnIdx) std::cerr << "ERROR: Off Line indexes out of sync. Curr: "<<currOffLnIdx<<"OFFx: "<<beginOffLn<< std::endl;
//			
//				if(blacklist.size() < currOffxPair.first.size()) blacklist.resize(currOffxPair.first.size());
//				if(whitelist.size() < currOffxPair.first.size()) whitelist.resize(currOffxPair.first.size());
//
//				bool skip = false;								
//				if (endOffLn-beginOffLn <=2) {					// Skip if empty OFF
//					skip=true;
//				}else {											// Skip to next line when on black list or not on whitelist
//					for (int i=0;i<(int)currOffxPair.first.size();i++) {
//						if ((i<(int)blacklist.size() && blacklist[i].find(currOffxPair.first[i]) != blacklist[i].end()) ||
//							(activeWhite.find(i)!= activeWhite.end() && whitelist[i].find(currOffxPair.first[i]) == whitelist[i].end())) {
//							skip=true;	break;
//						}
//					}
//				}
//
//				for (;currOffLnIdx < endOffLn; currOffLnIdx++) {			// Read #(endLn-beginLn) lines from OFF
//					std::string cOFFxLine;									// Init OFF read string
//					if (std::getline(c_OFF_ifstream,cOFFxLine) && !skip) {	// Read line		
//						currOffSStream << cOFFxLine <<std::endl;			// Put whole line in stringstream
//					} else if (!skip) std::cerr << "ERROR: Something wrong while reading file: " << filename_0ext << ".OFF" << " at line number: " << currOffLnIdx <<std::endl;
//				}
//				if (skip) {
//					std::cerr << "Skipped "<<nbrOfLinesRead<<" "<<currOffxPair.first[0]<<" due to black or white list." << std::endl;
//					currOffxPair.first.clear();								// Clear temp data
//					continue;
//				}
//				break;														// Break if read one line from OFFx succesfully 
//			} else std::cerr << "WARNING: Empty line in file: " << filename_0ext << ".OFFx" <<std::endl;	
//		}
//		if (currOffxPair.first.empty()) {											// Probably EoF reached
//			// close??
//			return false;
//		}
//
//		currInfo = currOffxPair.first;												// Store nef info
//		sstream2nef(currOffSStream, nefPolyhe);										// Create polyhe & nef from stringstream
//		if (nefPolyhe.number_of_volumes()<2 ||!nefPolyhe.is_valid()) return next();									// Skip empty input
//	} else {
//		currPolyhe.clear();			// Erase current polyhe
//		currNefPolyhe.clear();		// Erase current NEF, but leave info
//	}
//
//	std::set<std::string> alwaysFat;
//	alwaysFat.insert("Window");
//	alwaysFat.insert("Door");	
//
//	if (currInfo[0] != "Install")	{									// BI's do not need to be manifolds yet
//		if (splitFirst) {
//			if (currNefVec.size()==0)									// else nefPolyhe is empty
//				currNefVec = splitNefs (nefPolyhe,false);
//			nefPolyhe  = currNefVec.back();
//			currNefVec.pop_back();
//		}
//
//		if ( nefPolyhe.is_simple() ) {
//			if (alwaysFat.find(currInfo[0])!=alwaysFat.end()) {			// Make sure openings do not desolve into the wall REPLACE BY DORPEL THING
//			
//				//Point_3 cog2 = comp_cog(nefPolyhe);
//				nefPolyhe.convert_to_polyhedron(polyhe);
//				Point_3 cog = comp_cog(polyhe);
//				//std::cout <<cog<<std::endl<<cog2<<std::endl;
//
//				Transformation toO(CGAL::TRANSLATION, Vector_3(cog,CGAL::ORIGIN)); 
//				Transformation scale(CGAL::SCALING, 1+FIX_MANIFOLD_SIZE);
//				Transformation fromO(CGAL::TRANSLATION, Vector_3(CGAL::ORIGIN,cog)); 
//				nefPolyhe.transform(fromO*scale*toO);
//			}
//			nefPolyhe.convert_to_polyhedron(polyhe);	
//		} else {
//			if (attemptFix) {
//				std::cerr << "WARNING: Geometry is not simple, will make it fat. -> " << cOFFxLine  << std::endl;
//				double	tapeSize = FIX_MANIFOLD_SIZE;
//			
//				if (nefPolyhe.number_of_volumes() >=2) {
//					Nef_polyhedron robot;
//					makeRobot(tapeSize, robot);		
//					nefPolyhe.regularization();
//					nefPolyhe = CGAL::minkowski_sum_3(nefPolyhe,robot);
//				} else 
//					nefPolyhe = convexFix(nefPolyhe, tapeSize);
//
//				make2Manifold(nefPolyhe);
//			}
//			if (!attemptFix || !nef2polyhe(nefPolyhe,polyhe)) {
//				std::cerr << "ERROR: Creating Nef_polyhedron "<< currNefIdx<<"/"<<nbrOfLinesRead<<" failed: " << cOFFxLine  << std::endl;
//				return next();															// Return failure
//			}
//		}
//	}
//	std::string sem = currInfo[0];//boost::algorithm::join(currOffxPair.first, " ");
//	for (Polyhedron::Facet_iterator fIt = polyhe.facets_begin(); fIt != polyhe.facets_end(); ++fIt)
//		fIt->semanticBLA = sem;			
//	currPolyhe	  = polyhe;												// Store polyhe
//	currNefPolyhe = nefPolyhe;											// Store NEF
//	//std::cout << comp_volume(nefPolyhe) <<std::endl; //does not work
//	volume = comp_volume(polyhe);										// 0 for BI's
//	++currNefIdx;														// Increment curr nef index
//	return true;														// Return succes
//}
//
//// Nullify current data
//void OFFxReader::eraseCurrent() {
//	currPolyhe.clear();			// Erase current polyhe
//	currNefPolyhe.clear();		// Erase current NEF
//	currInfo.clear();			// Erase current info
//}
//	
//void OFFxReader::addToBlacklist(int position, std::string blackStr) {
//	if((int)blacklist.size() < position+1) blacklist.resize(position+1);
//	blacklist[position].insert(blackStr);
//}
//
//void OFFxReader::addToWhitelist(int position, std::string whiteStr) {
//	if((int)whitelist.size() < position+1) whitelist.resize(position+1);
//	whitelist[position].insert(whiteStr);
//	activeWhite.insert(position);
//}
//
//
///* ------------------- Data handling functions ---------------------------*/
//
//// Formats OFFx-line in a tokenized string vector and (beginLn,endLn) int vector for reading OFF
//offxPair offxLine2pair(std::string line) {
//	std::vector<std::string> tokenizedLine;						// vector of tokenized current line
//	boost::split(tokenizedLine, line, boost::is_any_of(" "));	// tokenize line on " " // #include <boost/algorithm/string.hpp>
//
//	std::vector<int> beginEndVector;							// Start and last line of polyhe in OFF file
//	int endLn (stoi(tokenizedLine.back()));						// get last as int
//	tokenizedLine.pop_back();									// pop last
//	beginEndVector.push_back(stoi(tokenizedLine.back()));		// get new last as int + add (beginLn) to vector
//	beginEndVector.push_back(endLn);							// add (endLn) to vector
//	tokenizedLine.pop_back();									// pop new last
//
//	return offxPair (tokenizedLine,beginEndVector);				// return paired string & int vector
//}
//
//// Creates a Nef_polyhedron from off-stringstream
//// Regularization & Normalizing border are optional
//// Returns true if succesfull
//bool sstream2nef(std::stringstream& offSStream, Nef_polyhedron& nefPolyhe,bool quite, bool normal, bool regular) {
//	Polyhedron polyhe;											// Init polyhe
//	bool success = true;										// Init succes boolean
//
//	CGAL::OFF_to_nef_3(offSStream, nefPolyhe);				// Create nef from stringstream
//	if (regular) nefPolyhe.regularization();				// Regularize nef
//	if (nefPolyhe.is_simple()) {							// Check if simple
//		if (nefPolyhe.is_valid()) {							// Check is valid
//			return true;
//		} else if (!quite) std::cerr << "WARNING: Polyhedron is not valid (stream2nef)" << std::endl;
//	} else if (!quite) std::cerr << "WARNING: Polyhedron is not simple (stream2nef)" << std::endl;
//	return false;												// Return true if succes
//}