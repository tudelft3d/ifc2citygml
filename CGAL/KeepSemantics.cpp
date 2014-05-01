#include "KeepSemantics.h"
#ifdef WIN32
#include <windows.h>
#endif
CGAL::Timer T;

int main(int argc, char *argv[]) {

	////////////////////////////////////// Settings //////////////////////////////////////
#ifdef WIN32
	SetConsoleTitle("ifCity Converter");
#endif

	std::string filename_0ext = "";
	if (argc <= 1) {
		std::cout << "filename: ";
		std::getline(std::cin,filename_0ext);
	} else {
		filename_0ext = argv[1];
	}
	
#ifdef WIN32
	SetConsoleTitle(("ifCity Converter: "+filename_0ext).c_str());
#endif

	int offsetMM = OFFSET_MAGNITUDE;
	double offsetMagnitude;
	std::string offsetSTR;

	bool useOldExterior = false;
	std::ifstream shrtctstrm(fn2offFN(filename_0ext+"_ext2.OFF"));
	
	if (argc <= 2) {
		if (shrtctstrm.is_open()) {
			std::string overwriteANS = "";
			std::cout << "Reuse old exterior (not safe for overlapping solids) ? y/n ";
			std::getline(std::cin,overwriteANS);
			useOldExterior = overwriteANS=="y";
		}

		// Get closing offset magnitude	
		int tempD;
		if (!useOldExterior) {
			std::cout << "Closing offset magnitude? (~150mm): ";
			if (std::cin >> tempD)
				offsetMM = tempD;
			std::cin.ignore();std::cin.clear();
		}
	} else 
		offsetMM = std::stoi(argv[2]);
	offsetMagnitude = offsetMM/1000.0;
	std::stringstream tempss;
	tempss << offsetMM <<"mm";
	offsetSTR = tempss.str();

#ifdef WIN32
	SetConsoleTitle(("ifCity Converter: "+filename_0ext+" - "+offsetSTR).c_str());
#endif

	bool useFacetMink = false;
	if (argc <= 3) {
		std::string minkANS = "";
		std::cout << "Use Minkowski Sum of Facets instead of normal Minkowski Sum (SLOW) ? y/n ";
		std::getline(std::cin,minkANS);
		useFacetMink = minkANS=="y";
	} else useFacetMink = argv[3]=="y";

	bool createRooms = false;
	if (argc <= 4) {
		std::string croomANS = "";
		std::cout << "Create LoD4 Rooms ? y/n ";
		std::getline(std::cin,croomANS);
		createRooms = croomANS=="y";
	} else createRooms = argv[4]=="y";

	bool createALLLODS = false;
	if (argc <= 5) {
		std::string alllodANS = "";
		std::cout << "Create All LoDs (offset should be >1M)? y/n ";
		std::getline(std::cin,alllodANS);
		createALLLODS = alllodANS=="y";
	} else createALLLODS = argv[5]=="y";

	bool loopmagnitudes = false;
	if (argc <= 6) {
		std::string loopANS = "";
		std::cout << "Loop offset magnitude ? y/n ";
		std::getline(std::cin,loopANS);
		loopmagnitudes = loopANS=="y";
	} else loopmagnitudes = argv[6]=="y";

	////////////////////////////////////// Loop //////////////////////////////////////
	std::ofstream stsFile(filename_0ext+".log");
	bool firstLoop = true;

	for (;firstLoop || (loopmagnitudes && offsetMM<=1000); offsetMM+=10)
	{
		firstLoop = false;
	if (loopmagnitudes) {
		offsetMagnitude = offsetMM/1000.0;
		tempss.clear();
		tempss.str(std::string());
		tempss << offsetMM <<"mm";
		offsetSTR = tempss.str();
	}
#ifdef WIN32
	SetConsoleTitle(("ifCity Converter: "+filename_0ext+" - "+offsetSTR).c_str());
#endif
	////////////////////////////////////// Vars n stuff //////////////////////////////////////

	CGAL::Timer totalTime;
	T.start();
	totalTime.start();

	Nef_polyhedron fusedNefPolyhe;
	Nef_polyhedron fusedInstallNef;
	Nef_polyhedron fusedLoD4Rooms;
	bool has_BuildParts	= false;
	bool has_BuildInstalls	= false;
	bool has_LoD4Rooms		= false;
	PolVector polyheVec;
	PolVector biPolVec;
	PolVector bpPolVec;
	PolVector lod4PolVec;
	std::vector<Nef_polyhedron> bpVector;

	Kernel::FT summedVolume=0;
	NefNaryUnion naryNef;
	NefNaryUnion naryBuildInstall;
	NefNaryUnion naryLoD4Rooms;

	OFFxReader oRead(filename_0ext);
	//oRead.addToBlacklist(0,"Site");
	//oRead.addToBlacklist(0,"Install");
	oRead.addToWhitelist(0,"Wall");
	oRead.addToWhitelist(0,"Roof");
	oRead.addToWhitelist(0,"Ground");
	oRead.addToWhitelist(0,"Door");
	oRead.addToWhitelist(0,"Window");
	oRead.addToWhitelist(0,"Closure");
	oRead.addToWhitelist(0,"Anything");
	oRead.addToWhitelist(0,"Install");

	////oRead.set_attemptFix(false);


	////////////////////////////////////// Reading //////////////////////////////////////

	if (oRead.is_open()){
		//oRead.skip(392);
		while(oRead.next()) {
			std::cout << "Read ";
			if (oRead.get_currentInfo()[0] != "Install") {					// if not a building installation
				if (!useOldExterior || !shrtctstrm.is_open())				// Skip merging if using a pre-made exterior
					naryNef.add_polyhedron(oRead.get_currentNef());			// Get nef for merging
				polyheVec.push_back(oRead.get_currentPolyhe());				// Store polyhe
				summedVolume += oRead.get_volume();	

				if (createRooms && oRead.get_currentInfo()[0] == "Closure") {				// If IFC-Space -> Room
					has_LoD4Rooms = true;
					naryLoD4Rooms.add_polyhedron(oRead.get_currentNef());
				}
			} else {														// If building installation	
				has_BuildInstalls = true;
				naryBuildInstall.add_polyhedron(oRead.get_currentNef());
			}
			std::cout << oRead.get_currentNefIdx()<<"/"<<oRead.get_nbrOfLinesRead() <<" "<<oRead.get_currentInfo()[0]<<" Vol: "<<oRead.get_volume()<<std::endl;
		}
		oRead.close();
	} else {
		std::cerr << "ERROR: Could not open files. " << filename_0ext <<std::endl;
		return 1;
	}
	// Merge nefs if needed
	if (!useOldExterior || !shrtctstrm.is_open()) {
		fusedNefPolyhe = naryNef.get_union();
		fusedNefPolyhe.regularization();
	}
	// Merge Building installations
	if (has_BuildInstalls) {
		fusedInstallNef = naryBuildInstall.get_union();
		fusedInstallNef.regularization();
	}
	if (createRooms && has_LoD4Rooms) {
		fusedLoD4Rooms = naryLoD4Rooms.get_union();
		fusedLoD4Rooms.regularization();
	}

	T.stop();	// BS. time thing
	std::cout << "Done reading. Time: " << T.time() << std::endl;
	T.reset();T.start();

	////////////////////////////////////// Geometric Conversion //////////////////////////////////////

	Polyhedron exteriorPolyhe;
	Nef_polyhedron roundedNef;
	// Use pre-made exterior, for testing purposes only?
	if (shrtctstrm.is_open() && useOldExterior) { 
		fusedNefPolyhe.clear();
		CGAL::OFF_to_nef_3(shrtctstrm,fusedNefPolyhe);
		exteriorPolyhe.clear();
		nef2polyhe(fusedNefPolyhe,exteriorPolyhe);
	}
	else { // Apply closing operation and round coords
		if (offsetMM > 0) {									// no closing needed when offset is <= 0
			if (!closing_Minkowski(fusedNefPolyhe,exteriorPolyhe,offsetMagnitude,bpVector,useFacetMink)) return 1;		
		} else {
			std::cout << "Extracting manifold exterior..."<< std::endl;	
			if (MANIFOLD_FIRST_EXTERIOR) make2Manifold(fusedNefPolyhe,-1.0,false);	//meh
			fusedNefPolyhe = makeExteriorManifold(fusedNefPolyhe,bpVector);
			if (!nef2polyhe(fusedNefPolyhe,exteriorPolyhe)) return 1;	
		}
		roundCoordsSafely(exteriorPolyhe,bpVector);							// maybe do degen first? nah
		polyhe2nef(exteriorPolyhe,roundedNef);								// create nef to cut BP's and BI's
		write_polyhe2off(exteriorPolyhe, filename_0ext+"_ext2");	// write .off to be used as pre-made exterior
	}

	std::cout << summedVolume<< " - " << comp_volume(exteriorPolyhe)<< std::endl;

	T.stop();
	std::cout << "Done exerior. Time: " << T.time() << std::endl;
	T.reset();T.start();

	std::cout << "Fixing degen..."<< std::endl;
	fix_degenerate(exteriorPolyhe);

	T.stop();
	std::cout << "Done degen. Time: " << T.time() << std::endl;
	T.reset();T.start();

	////////////////////////////////////// Semantics Shizzle //////////////////////////////////////
	
	std::cout << "\rMapping semantics... "<< std::endl;
	set_semantic_AABB_C2V(exteriorPolyhe,polyheVec);

	write_polyhe2ColoredOFF(exteriorPolyhe, filename_0ext+"_ext_col");

	double totalArea = 0;	// Calculates and sets area for each triangle needed in later functions
	for (Polyhedron::Facet_iterator fIt = exteriorPolyhe.facets_begin(); fIt != exteriorPolyhe.facets_end(); ++fIt) {
		fIt->area = sqrt(CGAL::to_double(comp_facetSquaredArea(fIt)));
		totalArea += fIt->area;
	}
	if (totalArea==0) std::cerr <<"ERROR: Total area equals zero."<<std::endl;

	Kernel::FT msError		= 0;	// Determines error due to minkowski closing
	double minkTotalArea = 0;		// Total area of artefacts
	for (Polyhedron::Facet_const_iterator fcIt = exteriorPolyhe.facets_begin(); fcIt != exteriorPolyhe.facets_end(); ++fcIt) {
		msError		  += fcIt->leastSqDistance*fcIt->area;
		minkTotalArea += fcIt->isMinkFacet * fcIt->area;
	}

	double rmsError = sqrt(CGAL::to_double(msError/totalArea));
	std::cout << "Area weighted RMS error: " << rmsError << std::endl;

	apply_semanticRequirements(exteriorPolyhe);		// These require area to be set
	remove_singularSemantics(exteriorPolyhe);	

	T.stop();
	std::cout << "Done semantics. Time: " << T.time() << std::endl;
	T.reset();T.start();

	////////////////////////////////////// Writing //////////////////////////////////////

	write_polyhe2ColoredOFF(exteriorPolyhe, filename_0ext+"_ext_col2");

	T.reset();T.start();
	Polyhedron exteriorPolyheRed(exteriorPolyhe);
	std::cout << "Reducing polygons..."<< std::endl;
	int numVertBefore = exteriorPolyheRed.size_of_vertices();
	int numEdgBefore = exteriorPolyheRed.size_of_halfedges()/2;

	mergeCoplanar(exteriorPolyheRed);
	mergeColinear(exteriorPolyheRed);
	int numVertAfter = exteriorPolyheRed.size_of_vertices();
	int numEdgAfter = exteriorPolyheRed.size_of_halfedges()/2;

	T.stop();
	std::cout << "Done Coplanar. Time: " << T.time() << std::endl;

	write_polyhe2ColoredOFF(exteriorPolyheRed, filename_0ext+"_ext_col_CLEAN");	

	CityGMLWriter triGMLwrtr(filename_0ext+"_TRI_"+offsetSTR, exteriorPolyhe);
	triGMLwrtr.write_CityGML();
	triGMLwrtr.write_CityGMLSolids();

	CityGMLWriter cleanNOBPGMLwrtr(filename_0ext+"_POLY_"+offsetSTR, exteriorPolyheRed, false, bpPolVec, false, biPolVec, false, lod4PolVec);
	cleanNOBPGMLwrtr.write_CityGML();
	cleanNOBPGMLwrtr.write_CityGMLSolids();
	
	////////////////////////////////////// BuildingParts //////////////////////////////////////

	std::vector<Nef_polyhedron> roundedbpVector;
	if (!bpVector.empty()) {
		unsigned int bpCount = 1;
		for (std::vector<Nef_polyhedron>::iterator bpIt=bpVector.begin();bpIt!=bpVector.end();++bpIt,++bpCount) {
			std::cout <<"Processing BuildingParts"<< bpCount<<"/"<<bpVector.size()<<"..."<< std::endl;
			*bpIt = *bpIt-roundedNef - fusedNefPolyhe;
			bpIt->regularization();
			if (bpIt->number_of_volumes()<2) continue;

			Polyhedron bpPolyhe;													// Manifold, 2Polyhe, writingPrep
			if (!make2Manifold(*bpIt) ||
				!nef2polyhe(*bpIt,bpPolyhe) ||
				!roundCoordsSafely(bpPolyhe)) {
				std::cerr << "WARNING: Skipped BuildingPart" <<std::endl;
				continue; 
			}
			
			Nef_polyhedron roundedBP;												// Store as nefs to be used to cut BI's
			polyhe2nef(exteriorPolyhe,roundedBP);
			roundedbpVector.push_back(roundedBP);

			fix_degenerate(bpPolyhe);												// Remove degeneracies (as far as possible)
			for (Polyhedron::Facet_iterator fIt = bpPolyhe.facets_begin(); fIt != bpPolyhe.facets_end(); ++fIt)
				fIt->area = sqrt(CGAL::to_double(comp_facetSquaredArea(fIt)));
			set_semantic_AABB_C2V(bpPolyhe,polyheVec);
			apply_semanticRequirements(bpPolyhe);
			remove_singularSemantics(bpPolyhe);
			std::cout << "Reducing BuildingPart polygons..."<< std::endl;
			mergeCoplanar(bpPolyhe);												// Reduce # polygons
			mergeColinear(bpPolyhe);
			if (bpPolyhe.is_closed()) {	//  && comp_volume(bpPolyhe)>0.0001
					bpPolVec.push_back(bpPolyhe);									// Store bp
			}
		}		
	}
	has_BuildParts = !bpPolVec.empty();
	if (has_BuildParts) {
		CityGMLWriter cleanBPGMLwrtr(filename_0ext+"_POLY_"+offsetSTR, exteriorPolyheRed, has_BuildParts, bpPolVec, false, biPolVec, false, lod4PolVec);
		cleanBPGMLwrtr.write_CityGML();
		cleanBPGMLwrtr.write_CityGMLSolids();
	}
	////////////////////////////////////// BuildingInstallations //////////////////////////////////////

	if (has_BuildInstalls) {
		std::cout << "Subtracting Building and BuildingParts from BuildingInstallations..."<< std::endl;
		fusedInstallNef -= roundedNef;										// Cut exterior from BuildingInstallations (also cut bp's)
		fusedInstallNef -= fusedNefPolyhe;									
		for (std::vector<Nef_polyhedron>::iterator bpIt=roundedbpVector.begin();bpIt!=roundedbpVector.end();++bpIt) // Also cut building parts
			fusedInstallNef -= *bpIt;
		for (std::vector<Nef_polyhedron>::iterator bpIt=bpVector.begin();bpIt!=bpVector.end();++bpIt)
			fusedInstallNef -= *bpIt;
		fusedInstallNef.regularization();	

		if (!fusedInstallNef.is_empty()) {
			std::cout << "Splitting BuildingInstallations..."<< std::endl;
			std::vector<Nef_polyhedron> biNefVec = splitNefs (fusedInstallNef);			// Seperate bi Polyhedra

			unsigned int biCount = 1;
			for (std::vector<Nef_polyhedron>::iterator biIt=biNefVec.begin();biIt!=biNefVec.end();++biIt,++biCount) {
				if (biIt->number_of_volumes()<2) continue;
				std::cout <<"Processing BuildingInstallation"<< biCount<<"/"<<biNefVec.size()<<"..."<< std::endl;

				Polyhedron biPolyhe;													// Manifold, 2Polyhe, writingPrep
				if (!make2Manifold(*biIt) ||
					!nef2polyhe(*biIt,biPolyhe) ||
					!roundCoordsSafely(biPolyhe)) {
					std::cerr << "WARNING: Skipped BuildingInstallation" <<std::endl;
					continue; 
				}
				fix_degenerate(biPolyhe);												// Remove degeneracies (as far as possible)
				for (Polyhedron::Facet_iterator fIt = biPolyhe.facets_begin(); fIt != biPolyhe.facets_end(); ++fIt)
					fIt->semanticBLA = "Install";										// Add semantics

				std::cout << "Reducing bi polygons..."<< std::endl;
				mergeCoplanar(biPolyhe);												// Reduce # polygons
				mergeColinear(biPolyhe);
				if (biPolyhe.is_closed())//  && comp_volume(biPolyhe)>0.0001
					biPolVec.push_back(biPolyhe);										// Store bi

			}
		}
	}
	has_BuildInstalls = !biPolVec.empty();
	if (has_BuildInstalls) {
		CityGMLWriter cleanBIGMLwrtr(filename_0ext+"_POLY_"+offsetSTR, exteriorPolyheRed, has_BuildParts, bpPolVec, has_BuildInstalls, biPolVec, false, lod4PolVec);
		cleanBIGMLwrtr.write_CityGML();
		cleanBIGMLwrtr.write_CityGMLSolids();
	}

	////////////////////////////////////// LoD4 Rooms //////////////////////////////////////
	if (createRooms) {
		if (has_LoD4Rooms) {															// Spaces are always interior(is checked) and exterior can only be bigger than original so never intersects
																						// ..  can only be bigger than original so never intersects.
			std::cout << "Splitting Rooms..."<< std::endl;
			std::vector<Nef_polyhedron> roomNefVec = splitNefs (fusedLoD4Rooms);		// Seperate room Polyhedra

			unsigned int roomCount = 1;
			for (std::vector<Nef_polyhedron>::iterator roomIt=roomNefVec.begin();roomIt!=roomNefVec.end();++roomIt,++roomCount) {			
				if (roomIt->number_of_volumes()<2) continue;
				std::cout <<"Processing Room"<< roomCount<<"/"<<roomNefVec.size()<<"..."<< std::endl;

				Polyhedron roomPolyhe;													
				if (!make2Manifold(*roomIt) ||											// Manifold, 2Polyhe, writingPrep
					!nef2polyhe(*roomIt,roomPolyhe) ||
					!roundCoordsSafely(roomPolyhe)) {
					std::cerr << "WARNING: Skipped Room" <<std::endl;
					continue; 
				}
				set_semantic_InteriorLoD4(roomPolyhe);										
				fix_degenerate(roomPolyhe);												// Remove degeneracies (as far as possible)

				std::cout << "Reducing LoD4 Room polygons..."<< std::endl;
				mergeCoplanar(roomPolyhe);												// Reduce # polygons
				mergeColinear(roomPolyhe);

				if (roomPolyhe.is_closed())
					lod4PolVec.push_back(roomPolyhe);									// Store Room
			}		
		}	
		has_LoD4Rooms = !lod4PolVec.empty();
		if (has_LoD4Rooms) {
			CityGMLWriter cleanGMLwrtr(filename_0ext+"_POLY_"+offsetSTR, exteriorPolyheRed, has_BuildParts, bpPolVec, has_BuildInstalls, biPolVec, has_LoD4Rooms, lod4PolVec);
			cleanGMLwrtr.write_CityGML();
			cleanGMLwrtr.write_CityGMLSolids();
		}
	}

	if (createALLLODS) {
	////////////////////////////////////// LoD2 //////////////////////////////////////

		std::vector<Nef_polyhedron> lod2NefVecs;
		std::transform( exteriorPolyhe.facets_begin(), exteriorPolyhe.facets_end(),exteriorPolyhe.planes_begin(),Plane_equation());
		for (Polyhedron::Facet_const_iterator fcIt = exteriorPolyhe.facets_begin(); fcIt != exteriorPolyhe.facets_end(); ++fcIt){ // Loop through all facets
			//Create cutting Nefs for alls ceilings, floors, doors and windows
			if (fcIt->semanticBLA == "Ceiling" || fcIt->semanticBLA == "Floor" || fcIt->semanticBLA == "Door" || fcIt->semanticBLA == "Window" ||
				(fcIt->semanticBLA == "Wall" && is_colinear(fcIt->plane().orthogonal_vector(),Vector_3(Point_3(0,0,1),CGAL::ORIGIN)))) { 
				pointVector pv = comp_facetPoints(fcIt);
				pointVector hullPoints = pv;
				for(pointVector::iterator pvIt=pv.begin(); pvIt!=pv.end(); pvIt++) {
					hullPoints.push_back(Point_3(pvIt->x(),pvIt->y(),pvIt->z()+10000));
					hullPoints.push_back(Point_3(pvIt->x(),pvIt->y(),pvIt->z()-10000));
				}
				Polyhedron polyHull;	
				CGAL::convex_hull_3(hullPoints.begin(),hullPoints.end(),polyHull);		// Create convexhull
				lod2NefVecs.push_back(Nef_polyhedron(polyHull));
			}
		}
		for(std::vector<Nef_polyhedron>::iterator nef2It=lod2NefVecs.begin(); nef2It!=lod2NefVecs.end(); nef2It++)
			roundedNef -= *nef2It;
		roundedNef.regularization();
		Polyhedron lod2Polyhe;
		if (roundedNef.number_of_volumes()<2 ||
			!make2Manifold(roundedNef) ||
			!nef2polyhe(roundedNef,lod2Polyhe) ||
			!roundCoordsSafely(lod2Polyhe)) {
			std::cerr << "WARNING: Skipped LoD2" <<std::endl;
		} else {
			fix_degenerate(lod2Polyhe);												// Remove degeneracies (as far as possible)

			// Deteremine semantics
			std::transform( lod2Polyhe.facets_begin(), lod2Polyhe.facets_end(),lod2Polyhe.planes_begin(),Plane_equation());
			for (Polyhedron::Facet_iterator fIt = lod2Polyhe.facets_begin(); fIt != lod2Polyhe.facets_end(); ++fIt) {
				Vector_3 ortVec = fIt->plane().orthogonal_vector();
				Vector_3 ortVec2(ortVec.x(),ortVec.y(),0);
			
				if (!(ortVec.x()==0&&ortVec.y()==0) && is_colinear(ortVec,ortVec2,5 * CGAL_PI/180))
					fIt->semanticBLA = "Wall";
				else if (ortVec.z()>0)
					fIt->semanticBLA = "Roof";
				else 
					fIt->semanticBLA = "Ground";					
			}

			std::cout << "Reducing LoD2 polygons..."<< std::endl;
			mergeCoplanar(lod2Polyhe);												// Reduce # polygons
			mergeColinear(lod2Polyhe);

			CityGMLWriter lod2GMLwrtr(filename_0ext+"_LoD2", lod2Polyhe);
			lod2GMLwrtr.write_CityGML(2);
			lod2GMLwrtr.write_CityGMLSolids(2);
		}

		////////////////////////////////////// LoD1 //////////////////////////////////////

		Polyhedron lod1Polyhe  = createBBox (lod2Polyhe);
		mergeCoplanar(lod1Polyhe);
		CityGMLWriter lod1GMLwrtr(filename_0ext+"_LoD1", lod1Polyhe);
		lod1GMLwrtr.write_CityGMLSolids(1);

		////////////////////////////////////// LoD0 //////////////////////////////////////

		// requires terrain / site
	} // if (createALLLODS)

	////////////////////////////////////// Done //////////////////////////////////////
	totalTime.stop();
	std::cout << std::endl << "Conversion completed. Time: " << totalTime.time() << std::endl;
	
	std::cout << "Area weighted RMS error: " << rmsError << std::endl;
	std::cout << "Artefact area perc: " << minkTotalArea/totalArea*100 <<"%"<< std::endl;
	std::cout << "Vert before - afer - reduction: "<< numVertBefore<<" - "<<numVertAfter<<" - "<<100-numVertAfter*1.0/numVertBefore*100 <<"%"<< std::endl;
	std::cout << "Edges before - afer - reduction: "<< numEdgBefore<<" - "<<numEdgAfter<<" - "<<100-numEdgAfter*1.0/numEdgBefore*100 <<"%"<< std::endl;

	
	stsFile << offsetMM <<"\t"<< minkTotalArea/totalArea*100 <<"\t"<< totalTime.time() <<"\t"<< rmsError  <<"\t"<< 100.0-numVertAfter*1.0/numVertBefore*100 <<"\t"<<100-numEdgAfter*1.0/numEdgBefore*100 <<std::endl;
	}
	stsFile.close();
	std::cin.get();
	return 0;
}

