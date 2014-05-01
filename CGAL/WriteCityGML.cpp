#include "WriteCityGML.h"

bool CityGMLWriter::write_CityGML(int buildLoD) {
	/*Write the building to the CityGML file*/
	
	if (gml_ofstream.is_open())	{						// Check if open
		writeHeader();
		writeGeometryHeader(0,0);
								writeGeometry(exPolVector	,0,buildLoD); 
		if (has_BuildInstalls)	writeGeometry(biPolVector	,1,3);
		if (has_BuildParts)		writeGeometry(bpPolVector	,2,3);
		if (has_LoD4Rooms)		writeGeometry(lod4PolVector	,3,4);

		
		writeGeometryFooter(0);
		//writeAddress(); 
		//writeAppearance();				// move one up to make it within the building bla

		writeFooter();
		gml_ofstream.close();							// Close file
		return true;
	} else std::cerr << "ERROR: Can't open file: " << filename_0ext << ".gml" <<std::endl;
	return false;
}

bool CityGMLWriter::write_CityGMLSolids(int buildLoD) {
	/*Write the building solids to the CityGML file*/	
	if (open(filename_0ext+"_SLD"))	{					// Open and Check if open
		writeHeader();
		writeGeometryHeader(0,0);
								writeGeometrySLD(exPolVector	,buildLoD); 
		//if (has_BuildInstalls)	writeGeometrySLD(biPolVector	,3,true);
		//if (has_BuildParts)		writeGeometrySLD(bpPolVector	,3);
		//if (has_LoD4Rooms)		writeGeometrySLD(lod4PolVector	,4);
		writeGeometryFooter(0);
		writeFooter();
		gml_ofstream.close();							// Close file
		return true;
	} else std::cerr << "ERROR: Can't open file: " << filename_0ext << ".gml" <<std::endl;
	return false;
}


void CityGMLWriter::writeHeader(){
	gml_ofstream	<<	indent << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"<< std::endl;
	gml_ofstream	<<	indent << "<!-- CityGML Dataset produced with the IfCity-Converter developed by Sjors Donkers (SjorsDonkers@gmail.com) -->"<< std::endl;
	gml_ofstream	<<	indent << "<!-- Created: "<<currentDateTime()<<" -->"<<std::endl;

	gml_ofstream	<<	indent++ << "<CityModel" << std::endl;
	gml_ofstream	<<	indent << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
	gml_ofstream	<<	indent << "xmlns:xAL=\"urn:oasis:names:tc:ciq:xsdschema:xAL:2.0\"" << std::endl;
	gml_ofstream	<<	indent << "xmlns:app=\"http://www.opengis.net/citygml/appearance/2.0\"" << std::endl;
	gml_ofstream	<<	indent << "xmlns:xlink=\"http://www.w3.org/1999/xlink\"" << std::endl;
	gml_ofstream	<<	indent << "xmlns:gml=\"http://www.opengis.net/gml\"" << std::endl;
	gml_ofstream	<<	indent << "xmlns=\"http://www.opengis.net/citygml/2.0\"" << std::endl;
	gml_ofstream	<<	indent << "xmlns:bldg=\"http://www.opengis.net/citygml/building/2.0\"" << std::endl;
	gml_ofstream	<<	indent << "xsi:schemaLocation=\"http://www.opengis.net/citygml/2.0 http://schemas.opengis.net/citygml/2.0/cityGMLBase.xsd" << std::endl;
    gml_ofstream	<<	indent << "\thttp://www.opengis.net/citygml/appearance/2.0 http://schemas.opengis.net/citygml/appearance/2.0/appearance.xsd" << std::endl;
    gml_ofstream	<<	indent << "\thttp://www.opengis.net/citygml/building/2.0 http://schemas.opengis.net/citygml/building/2.0/building.xsd\">" << std::endl;	
	
	gml_ofstream	<<	indent	<<	"<gml:name>3D city model LOD3/4</gml:name>"<<std::endl;

//	gml_ofstream	<<	indent++	<<	"<gml:boundedBy>"<<std::endl;
//	gml_ofstream	<<	indent++	<<	"<gml:Envelope srsDimension=\"3\" srsName=\"urn:ogc:def:crs,crs:EPSG::25832,crs:EPSG::5783\">"<<std::endl;
//	gml_ofstream	<<	indent	<<	"<gml:lowerCorner>-50 -50 -50</gml:lowerCorner>"<<std::endl;
//	gml_ofstream	<<	indent	<<		"<gml:upperCorner>50  50 50</gml:upperCorner>"<<std::endl;
//	gml_ofstream	<<	--indent	<<	"</gml:Envelope>"<<std::endl;
//	gml_ofstream	<<	--indent	<<	"</gml:boundedBy>"<<std::endl;
}
void CityGMLWriter::writeFooter() { 
	gml_ofstream << --indent << "</CityModel>"<<std::endl;
}


void CityGMLWriter::writeGeometryHeader(int kindOSolid, int partCount){
	bool is_build			= kindOSolid==0;
	bool is_buildInstall	= kindOSolid==1;
	bool is_buildPart		= kindOSolid==2;
	bool is_room			= kindOSolid==3;

	if (is_build) {
		gml_ofstream	<<	indent++	<<	"<cityObjectMember>"<<std::endl;
		gml_ofstream	<<	indent++	<<	"<bldg:Building gml:id=\"" << filename_0ext << "\">"<<std::endl;
		gml_ofstream	<<	indent		<<	"<gml:name>" << filename_0ext << "</gml:name>"<<std::endl;
	}else if (is_buildInstall) {
		gml_ofstream << indent++ << "<bldg:outerBuildingInstallation>"<<std::endl;		
		gml_ofstream << indent++ << "<bldg:BuildingInstallation>"<<std::endl;
		gml_ofstream << indent << "<gml:name>"<<"BuildingInstallation "<<partCount<<"</gml:name>"<<std::endl;
	} else if (is_buildPart) {
		gml_ofstream << indent++ << "<bldg:consistsOfBuildingPart>"<<std::endl;	
		gml_ofstream << indent++ << "<bldg:BuildingPart>"<<std::endl;
		gml_ofstream << indent << "<gml:name>"<<"BuildingPart "<<partCount<<"</gml:name>"<<std::endl;
	} else if (is_room) {
		gml_ofstream << indent++ << "<bldg:interiorRoom>"<<std::endl;
		gml_ofstream << indent++ << "<bldg:Room>"<<std::endl;
		gml_ofstream << indent << "<gml:name>"<<"InteriorRoom "<<partCount<<"</gml:name>"<<std::endl;
	}
}

void CityGMLWriter::writeGeometryFooter(int kindOSolid) { 
	bool is_build			= kindOSolid==0;
	bool is_buildInstall	= kindOSolid==1;
	bool is_buildPart		= kindOSolid==2;
	bool is_room			= kindOSolid==3;

	if (is_build) {	
		// Could place appearance here
		gml_ofstream << --indent << "</bldg:Building>"<<std::endl;
		gml_ofstream << --indent << "</cityObjectMember>"<<std::endl;
	} else if (is_buildInstall) {
		gml_ofstream << --indent << "</bldg:BuildingInstallation>"<<std::endl;
		gml_ofstream << --indent << "</bldg:outerBuildingInstallation>"<<std::endl;
	} else if (is_buildPart) {
		gml_ofstream << --indent << "</bldg:BuildingPart>"<<std::endl;
		gml_ofstream << --indent << "</bldg:consistsOfBuildingPart>"<<std::endl;
	} else if (is_room) {
		gml_ofstream << --indent << "</bldg:Room>"<<std::endl;
		gml_ofstream << --indent << "</bldg:interiorRoom>"<<std::endl;
	}
}

void CityGMLWriter::writeGeometrySLD(PolVector& pVec,int LoD,bool is_buildInstall) {
	for (PolVector::iterator pvIt=pVec.begin();pvIt!=pVec.end();++pvIt) {
		// Writing solid
		if (is_buildInstall)
			gml_ofstream << indent++ << "<bldg:lod"<<LoD<<"Geometry>"<<std::endl;
		else
			gml_ofstream << indent++ << "<bldg:lod"<<LoD<<"Solid>"<<std::endl;
		gml_ofstream << indent++ << "<gml:Solid>"<<std::endl;
		gml_ofstream << indent++ << "<gml:exterior>"<<std::endl;
		gml_ofstream << indent++ << "<gml:CompositeSurface>"<<std::endl;
		for (Polyhedron::Facet_iterator fIt=pvIt->facets_begin();fIt!=pvIt->facets_end();++fIt)
			writeSurfaceMember(fIt);		
		gml_ofstream << --indent << "</gml:CompositeSurface>"<<std::endl;
		gml_ofstream << --indent << "</gml:exterior>"<<std::endl;
		gml_ofstream << --indent << "</gml:Solid>"<<std::endl;
		if (is_buildInstall)
			gml_ofstream << --indent << "</bldg:lod"<<LoD<<"Geometry>"<<std::endl;
		else
			gml_ofstream << --indent << "</bldg:lod"<<LoD<<"Solid>"<<std::endl;
	}
}

void CityGMLWriter::writeGeometry(PolVector& pVec,int kindOSolid,int LoD) {
	bool is_build			= kindOSolid==0;
	bool is_buildInstall	= kindOSolid==1;
	bool is_buildPart		= kindOSolid==2;
	bool is_room			= kindOSolid==3;

	bool is_interior = is_room;

	unsigned int partCount = 0;

	for (PolVector::iterator pvIt=pVec.begin();pvIt!=pVec.end();++pvIt,++partCount) {

		// create sfs
		sfsVec semFacetSetsVector;
		sfsVec openSFSVector;
		std::map<int,std::vector<int>> openingMap;
		groupSemanticFacets(*pvIt,semFacetSetsVector,openSFSVector,openingMap);

		// Write header
		if (!is_build) writeGeometryHeader(kindOSolid,partCount);

		// Should start loop over all LoD's here but mehh
		
		sfsVec::iterator sfsVecIt=semFacetSetsVector.begin();
		if (temp2FinalSem(sfsVecIt->first)!="BuildingInstallation") {
			// GMLid's of the current solid
			GMLidMap solidIDs;

			int surfCount = 0;										// Current surface index
			for (;sfsVecIt!=semFacetSetsVector.end();++sfsVecIt,++surfCount) {
				std::string finalSem = temp2FinalSem(sfsVecIt->first);
			
				gml_ofstream << indent++ << "<bldg:boundedBy>"<<std::endl;
				gml_ofstream << indent++ << "<bldg:"+finalSem+">"<<std::endl;
				gml_ofstream << indent << "<gml:name>"<<sfsVecIt->first<<" "<<partCount<<surfCount<<"</gml:name>"<<std::endl;
				gml_ofstream << indent++ << "<bldg:lod"<<LoD<<"MultiSurface>"<<std::endl;		// lod3Geometry for bi? No only solids
				gml_ofstream << indent++ << "<gml:MultiSurface>"<<std::endl;
			
				// Prepare gmlId string
				std::stringstream idStrtss;
				if (is_build) {					idStrtss << filename_0ext<<"_S"<<surfCount;
				} else if (is_buildInstall) {	idStrtss << filename_0ext<<"_BuildingInstallation_"<<partCount<<"_S"<<surfCount;
				} else if (is_buildPart) {		idStrtss << filename_0ext<<"_BuildingPart_"<<partCount<<"_S"<<surfCount;
				} else if (is_room) {			idStrtss << filename_0ext<<"_Room_"<<partCount<<"_S"<<surfCount;
				}
				std::string gmlStrt = idStrtss.str();

				int memIdx = 0;
				for (std::set<Polyhedron::Facet_handle>::iterator setIt=sfsVecIt->second.begin();setIt!=sfsVecIt->second.end();++setIt,++memIdx) {
					// Start new member
					// Create gmlID
					std::stringstream gmlIDss;
					gmlIDss << gmlStrt<<"_m"<<memIdx;			
					std::string gmlID = gmlIDss.str();

					// Store gmlID's
					solidIDs[finalSem].push_back(gmlID);	// current solid gmlID's
					gmlIDs[finalSem].push_back(gmlID);		// all gmlID's

					// Write Surface member to file
					writeSurfaceMember(*setIt,gmlID,is_interior);
				}
				// Close non-opening part of surface
				gml_ofstream << --indent << "</gml:MultiSurface>"<<std::endl;
			
				gml_ofstream << --indent << "</bldg:lod"<<LoD<<"MultiSurface>"<<std::endl;		// lod3Geometry	
				// Process openings for current surface
				writeSurfaceOpening(gmlStrt, solidIDs, openSFSVector, openingMap, surfCount,LoD,is_interior);
				// Close current surface
				gml_ofstream << --indent << "</bldg:"+finalSem+">"<<std::endl;
				gml_ofstream << --indent << "</bldg:boundedBy>"<<std::endl;
			}
			// Writing solid with XLinks
			gml_ofstream << indent++ << "<bldg:lod"<<LoD<<"Solid>"<<std::endl;
			gml_ofstream << indent++ << "<gml:Solid>"<<std::endl;
			gml_ofstream << indent++ << "<gml:exterior>"<<std::endl;
			gml_ofstream << indent++ << "<gml:CompositeSurface>"<<std::endl;

			for (idIt = solidIDs.begin(); idIt != solidIDs.end(); ++idIt) {
				gml_ofstream << indent << "<!-- "<<idIt->first<<" -->"<<std::endl;		// Comment polyType
				for (int v = 0; v != idIt->second.size(); ++v) {
					if (is_interior) {
						gml_ofstream << indent++ << "<gml:surfaceMember>"<<std::endl;
						gml_ofstream << indent++ << "<gml:OrientableSurface orientation=\"-\">"<<std::endl;	
						gml_ofstream << indent << "<gml:baseSurface xlink:href=\"#"<<idIt->second[v]<<"\"/>"<<std::endl;	
						gml_ofstream << --indent << "</gml:OrientableSurface>"<<std::endl;	
						gml_ofstream << --indent << "</gml:surfaceMember>"<<std::endl;
					} else
						gml_ofstream << indent << "<gml:surfaceMember xlink:href=\"#"<<idIt->second[v]<<"\"/>"<<std::endl;	
				}
			}
			gml_ofstream << --indent << "</gml:CompositeSurface>"<<std::endl;
			gml_ofstream << --indent << "</gml:exterior>"<<std::endl;
			gml_ofstream << --indent << "</gml:Solid>"<<std::endl;
			gml_ofstream << --indent << "</bldg:lod"<<LoD<<"Solid>"<<std::endl;
		} else 
			writeGeometrySLD(pVec,LoD,is_buildInstall);	// Write no_SEM_BI solid
	
		// Close buildingInstall or BuildingPart
		if (!is_build) writeGeometryFooter(kindOSolid);
	}
}

void CityGMLWriter::writeSurfaceOpening(std::string idStrt, GMLidMap& solidIDs, sfsVec& openSFSVector, std::map<int,std::vector<int>> openingMap, unsigned int surfCount, int LoD, bool is_interior) { // string, localGMLids, openingmap

	if ( openingMap.find(surfCount) != openingMap.end() ) {			// If surface has openings
		int openCount = 0;										// Current opening index
		for (std::vector<int>::iterator intIt=openingMap[surfCount].begin();intIt!=openingMap[surfCount].end();++intIt,++openCount) {
			// Start new opening
			std::string openFinalSem = temp2FinalSem(openSFSVector[*intIt].first);
			gml_ofstream << indent++ << "<bldg:opening>"<<std::endl;
			gml_ofstream << indent++ << "<bldg:"+openFinalSem+">"<<std::endl;
			gml_ofstream << indent << "<gml:name>"<<openSFSVector[*intIt].first<<"_S"<<surfCount<<"_O"<<openCount<<"</gml:name>"<<std::endl;
			gml_ofstream << indent++ << "<bldg:lod"<<LoD<<"MultiSurface>"<<std::endl;
			gml_ofstream << indent++ << "<gml:MultiSurface>"<<std::endl;				
			// Start new opening member
			int openIdx = 0;
			for (std::set<Polyhedron::Facet_handle>::iterator setIt=openSFSVector[*intIt].second.begin();setIt!=openSFSVector[*intIt].second.end();++setIt,++openIdx) {
				// Create gmlID
				std::stringstream gmlIDss;
				gmlIDss <<idStrt<<"_O"<<openCount<<"_m"<<openIdx;
				std::string gmlID = gmlIDss.str();
				// Store gmlID's
				solidIDs[openFinalSem].push_back(gmlID);
				gmlIDs[openFinalSem].push_back(gmlID);
				// Write Surface member to file
				writeSurfaceMember(*setIt,gmlID,is_interior);
			}
			// Close current opening
			gml_ofstream << --indent << "</gml:MultiSurface>"<<std::endl;
			gml_ofstream << --indent << "</bldg:lod"<<LoD<<"MultiSurface>"<<std::endl;
			gml_ofstream << --indent << "</bldg:"+openFinalSem+">"<<std::endl;
			gml_ofstream << --indent << "</bldg:opening>"<<std::endl;
}	}	}

// Writes facet with gmlID to file
void CityGMLWriter::writeSurfaceMember(Polyhedron::Facet_handle fh, std::string gmlID, bool is_interior) {
	gml_ofstream << indent++ << "<gml:surfaceMember>"<<std::endl;
	if (gmlID=="") {
		std::stringstream gmlIDss;
		gmlIDss << filename_0ext<<"_Solid_"<<SLDgmlID;			
		gmlID = gmlIDss.str();
		gmlIDs["BuildingInstallation"].push_back(gmlID);
		++SLDgmlID;
	}

	gml_ofstream << indent++ << "<gml:Polygon gml:id=\""<< gmlID <<"\">"<<std::endl;
    gml_ofstream << indent++ << "<gml:exterior>"<<std::endl;
    gml_ofstream << indent++ << "<gml:LinearRing>"<<std::endl;
    gml_ofstream << indent++ << "<gml:posList>"<<std::endl;
	// Get posList
		pointVector pointV = comp_facetPoints(fh);					// Reverse List if interior so facets should face inward
		if (is_interior) std::reverse(pointV.begin(),pointV.end());

		std::string posList; std::stringstream ss;							// Putting coordinates in a String
		
		for (pointVector::iterator pvIt=pointV.begin();pvIt!=pointV.end();++pvIt) {
			ss.str(std::string());ss.clear();ss.precision(OUTPUT_DECIMALS); ss << indent << *pvIt << ""<<std::endl;
			posList += ss.str();
		}		// Duplicate first coords at the end
		ss.str(std::string());ss.clear();ss.precision(OUTPUT_DECIMALS); ss <<indent << CGAL::to_double(pointV.begin()->x()) <<" "<< CGAL::to_double(pointV.begin()->y()) <<" "<< CGAL::to_double(pointV.begin()->z()) << ""<<std::endl;
		posList += ss.str();
	// Got posList
	gml_ofstream << posList;
	gml_ofstream << --indent << "</gml:posList>"<<std::endl;
	gml_ofstream << --indent << "</gml:LinearRing>"<<std::endl;
	gml_ofstream << --indent << "</gml:exterior>"<<std::endl;
	gml_ofstream << --indent << "</gml:Polygon>"<<std::endl;
	gml_ofstream << --indent << "</gml:surfaceMember>"<<std::endl;
}

void CityGMLWriter::writeAppearance() {
	std::string transp, color;
	for (idIt = gmlIDs.begin(); idIt != gmlIDs.end(); ++idIt) {
		std::string polyType = idIt->first;
		if (polyType == "RoofSurface") {				color = "1 0 0"; transp = "0";
		} else if (polyType == "WallSurface") {			color = "0.8 0.8 0.8"; transp = "0";
		} else if (polyType == "OuterCeilingSurface") {	color = "0.2 1 0.2";  transp = "0";
		} else if (polyType == "OuterFloorSurface") {	color = "0 1 1";  transp = "0";
		} else if (polyType == "GroundSurface") {		color = "1 1 0.4"; transp = "0";
		} else if (polyType == "InteriorWallSurface") {	color = "0.7 0.7 0.7"; transp = "0";
		} else if (polyType == "CeilingSurface") {		color = "0.8 0.2 0.2"; transp = "0";
		} else if (polyType == "FloorSurface") {		color = "0 0 1"; transp = "0";
		} else if (polyType == "Window") {				color = "0 0.4 0.2"; transp = "0.5";
		} else if (polyType == "Door") {				color = "0.5 0.165 0.165"; transp = "0";
		} else if (polyType == "ClosureSurface") {		color = "1 0.3 0.5"; transp = "0.5";
		} else if (polyType == "BuildingInstallation") {color = "0.8 0.7 0.6"; transp = "0";
		} else {										color = "1 0.3 0.5"; transp = "0";
		}

		gml_ofstream << indent++ << "<app:appearanceMember>"<<std::endl;
		gml_ofstream << indent++ << "<app:Appearance>"<<std::endl;
		gml_ofstream << indent << "<app:theme>default</app:theme>"<<std::endl;
		gml_ofstream << indent++ << "<app:surfaceDataMember>"<<std::endl;
		gml_ofstream << indent++ << "<app:X3DMaterial>"<<std::endl;
		gml_ofstream << indent << "<!-- "<<polyType<<" -->"<<std::endl;		// Comment polyType
		gml_ofstream << indent << "<app:transparency>" << transp << "</app:transparency>"<<std::endl;
		gml_ofstream << indent << "<app:diffuseColor>" << color << "</app:diffuseColor>"<<std::endl;
		for (int v = 0; v != idIt->second.size(); ++v)				// Write gmlIDs
			gml_ofstream << indent			<<	"<app:target>#" << idIt->second[v] << "</app:target>"<<std::endl;		
		gml_ofstream << --indent << "</app:X3DMaterial>"<<std::endl;
		gml_ofstream << --indent << "</app:surfaceDataMember>"<<std::endl;
		gml_ofstream << --indent << "</app:Appearance>"<<std::endl;
		gml_ofstream << --indent << "</app:appearanceMember>"<<std::endl;
	}
}


std::string temp2FinalSem (std::string& oldSem) {
	if (oldSem=="Wall") return "WallSurface";
	else if (oldSem=="Roof") return "RoofSurface";
	else if (oldSem=="Ceiling") return "OuterCeilingSurface";
	else if (oldSem=="Floor") return "OuterFloorSurface";
	else if (oldSem=="Ground") return "GroundSurface";
	else if (oldSem=="Window") return "Window";
	else if (oldSem=="Door") return "Door";
	else if (oldSem=="Closure") return "ClosureSurface";
	else if (oldSem=="Install") return "BuildingInstallation";
	else return oldSem;
}

void groupSemanticFacets(Polyhedron& polyhe,sfsVec& semFacetSetsVector,sfsVec& openSFSVector,std::map<int,std::vector<int>>& openingMap){
	bool skip;
	// Group non-openings
	for (Polyhedron::Facet_iterator fIt=polyhe.facets_begin();fIt!=polyhe.facets_end();++fIt) {
		if (fIt->semanticBLA == "Window" || fIt->semanticBLA == "Door") continue;				// Skip openings
		skip = false;																			// Check of facet has already been processed
		for (unsigned int i=0;i<semFacetSetsVector.size();++i) {
			if (semFacetSetsVector[i].first == fIt->semanticBLA									// Check sem of set
			&& (semFacetSetsVector[i].second.find(fIt)!=semFacetSetsVector[i].second.end())) {	// if in set -> skip
				skip = true;
				break;
			}
		}
		if (skip) continue;

		std::set<Polyhedron::Facet_handle> fhSet;
		connectedSemFacets(fIt,fIt->semanticBLA,false,fhSet,false);								// Get list of all (in)directly connected facets
		semFacetSetsVector.push_back(sfsPair(fIt->semanticBLA,fhSet));							// Store connected facet list
	}
	// Group and match openings
	for (Polyhedron::Facet_iterator fIt=polyhe.facets_begin();fIt!=polyhe.facets_end();++fIt) {
		if (!(fIt->semanticBLA == "Window" || fIt->semanticBLA == "Door")) continue;		// Skip non-openings
		skip = false;																		// Check of facet has already been processed
		for (unsigned int i=0;i<openSFSVector.size();++i) {
			if (openSFSVector[i].first == fIt->semanticBLA									// Check sem of set
			&& (openSFSVector[i].second.find(fIt)!=openSFSVector[i].second.end())) {		// if in set -> skip
				skip = true;
				break;
			}
		}
		if (skip) continue;

		// Store opening facets
		std::set<Polyhedron::Facet_handle> fhSet;
		connectedSemFacets(fIt,fIt->semanticBLA,false,fhSet,false);
		openSFSVector.push_back(sfsPair(fIt->semanticBLA,fhSet));

		// Match/Find Surface containing the opening
		bool found = false;	
		bool setRoof = false;
		Polyhedron::Facet_handle otherFace;
		otherFace->semanticBLA;
		Polyhedron::Facet::Halfedge_around_facet_circulator itHDS;
		for (std::set<Polyhedron::Facet_handle>::iterator setIt=fhSet.begin();setIt!=fhSet.end();++setIt) {
			itHDS = (*setIt)->facet_begin();				// Loop over the edges
			do {
				if (itHDS->opposite()->facet()->semanticBLA=="Wall" || itHDS->opposite()->facet()->semanticBLA=="WallSurface") {
					otherFace = itHDS->opposite()->facet();	// Prefer wall surfaces
					found = true;
					break;
				} else if (itHDS->opposite()->facet()->semanticBLA=="Roof" || itHDS->opposite()->facet()->semanticBLA=="RoofSurface") {
					otherFace = itHDS->opposite()->facet();	// Prefer wall surfaces
					setRoof = true;
				} else if (!setRoof // Roof is better than other (except wall)
						&& itHDS->opposite()->facet()->semanticBLA!="Window"
						&& itHDS->opposite()->facet()->semanticBLA!="Door"
						&& itHDS->opposite()->facet()->semanticBLA!="Install"
						&& itHDS->opposite()->facet()->semanticBLA!="BuildingInstallation")
					otherFace = itHDS->opposite()->facet();

			} while (++itHDS != (*setIt)->facet_begin());
			if (found) break;			// Break when wall or roof found
		}

		int openingContainer = 0;		// Index of container surface
		for (unsigned int i=0;i<semFacetSetsVector.size();++i) {
			if (semFacetSetsVector[i].first == otherFace->semanticBLA									// Check sem of set
			&& (semFacetSetsVector[i].second.find(otherFace)!=semFacetSetsVector[i].second.end())) {	// if in set -> skip
				openingContainer = i;
				break;
			}
		}	
		openingMap[openingContainer].push_back(openSFSVector.size()-1); // Add opening reference to container vector
	}
}


// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}