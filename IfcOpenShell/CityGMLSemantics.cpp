#include "CityGMLSemantics.h"

Ifc2x3::IfcSlabTypeEnum::IfcSlabTypeEnum getSlabType(const Ifc2x3::IfcSlab::ptr slab) {
	if (slab->hasPredefinedType())										// Check if it has a predefined type
		return slab->PredefinedType();

	Ifc2x3::IfcRelAssociates::list relList = slab->HasAssociations();	// Check if a type is associated
	for (Ifc2x3::IfcRelAssociates::it it = relList->begin(); it != relList->end();++it) {
		if ((*it)->is(Ifc2x3::IfcRelDefinesByType::Class())) {
			Ifc2x3::IfcRelDefinesByType::ptr relDefType = reinterpret_pointer_cast<Ifc2x3::IfcRelAssociates, Ifc2x3::IfcRelDefinesByType>(*it);
			Ifc2x3::IfcTypeObject* typeO = relDefType->RelatingType();
			if (typeO->is(Ifc2x3::IfcSlabType::Class())) {
				return ((Ifc2x3::IfcSlabType::ptr)typeO)->PredefinedType();
	}	}	}	
	return Ifc2x3::IfcSlabTypeEnum::IfcSlabType_NOTDEFINED;
}

Ifc2x3::IfcCoveringTypeEnum::IfcCoveringTypeEnum getCoveringType(const Ifc2x3::IfcBuildingElement::ptr element) {
	const Ifc2x3::IfcCovering::ptr Covering = reinterpret_pointer_cast<Ifc2x3::IfcBuildingElement,Ifc2x3::IfcCovering>(element);
	if (Covering->hasPredefinedType())										// Check if it has a predefined type
		return Covering->PredefinedType();

	Ifc2x3::IfcRelAssociates::list relList = element->HasAssociations();	// Check if a type is associated
	for (Ifc2x3::IfcRelAssociates::it it = relList->begin(); it != relList->end();++it) {
		if ((*it)->is(Ifc2x3::IfcRelDefinesByType::Class())) {
			Ifc2x3::IfcRelDefinesByType::ptr relDefType = reinterpret_pointer_cast<Ifc2x3::IfcRelAssociates, Ifc2x3::IfcRelDefinesByType>(*it);
			Ifc2x3::IfcTypeObject* typeO = relDefType->RelatingType();
			if (typeO->is(Ifc2x3::IfcCoveringType::Class())) {
				return ((Ifc2x3::IfcCoveringType::ptr)typeO)->PredefinedType();
	}	}	}
	return Ifc2x3::IfcCoveringTypeEnum::IfcCoveringType_NOTDEFINED;
}

std::string get_IDstr(Ifc2x3::IfcObjectDefinition* objectDef) {
	std::stringstream idSS;
	idSS << objectDef->entity->id();
	return idSS.str();
}

bool find_semantic(Ifc2x3::IfcObjectDefinition* objectDef, std::string& sem, semanticMAP sMap,bool canBeWindowDoor) {
	if (objectDef->is(Ifc2x3::IfcSpace::Class())) {					// Ignore exterior Spaces
		const Ifc2x3::IfcSpace::ptr space = reinterpret_pointer_cast<Ifc2x3::IfcObjectDefinition,Ifc2x3::IfcSpace>(objectDef);
		if (space->InteriorOrExteriorSpace() ==Ifc2x3::IfcInternalOrExternalEnum::IfcInternalOrExternal_EXTERNAL)
			return false;
	}

	std::map<std::string,std::string>::const_iterator it;						
	it = sMap[-1].find(Ifc2x3::Type::ToString(objectDef->type()));	// Search for types
	bool found = it != sMap[-1].end();



																				// Search for PreDefinedTypes
	std::map<int,std::map<std::string,std::string>>::const_iterator intmapIt = sMap.find(objectDef->type());
    if (intmapIt != sMap.end()) {
		if (objectDef->is(Ifc2x3::IfcSlab::Class())) {							// Slab
			const Ifc2x3::IfcSlab::ptr slab = reinterpret_pointer_cast<Ifc2x3::IfcObjectDefinition,Ifc2x3::IfcSlab>(objectDef);
			it = intmapIt->second.find(Ifc2x3::IfcSlabTypeEnum::ToString(getSlabType(slab)));
		} else if (objectDef->is(Ifc2x3::IfcCovering::Class())) {				// Covering
			const Ifc2x3::IfcCovering::ptr covering = reinterpret_pointer_cast<Ifc2x3::IfcObjectDefinition,Ifc2x3::IfcCovering>(objectDef);
			it = intmapIt->second.find(Ifc2x3::IfcCoveringTypeEnum::ToString(getCoveringType(covering)));
		}
		found |=  it != intmapIt->second.end();
	} 
	if (found)
		if (!canBeWindowDoor && (it->second =="Window" || it->second =="Door"))
		 	sem = "Wall " + get_IDstr(objectDef);					// Set semantic if found
		else
			sem = it->second + " " + get_IDstr(objectDef);					// Set semantic if found
	return found;
}

bool determine_buildingElementSemantic(Ifc2x3::IfcObjectDefinition* objectDef,std::string& sem,semanticMAP smapONLY,semanticMAP smapANY,semanticMAP smapPART,bool canBeWindowDoor, std::set<unsigned int> processedIDset) {
	if (processedIDset.find(objectDef->entity->id())!=processedIDset.end()) return false;				// Prevent infinite loop when referencing itself
	processedIDset.insert(objectDef->entity->id());

	if (find_semantic(objectDef,sem,smapANY,canBeWindowDoor)) return true;												// Check for top-level semantics
	find_semantic(objectDef,sem,smapPART,canBeWindowDoor);																// Check for low-level semantics (Continue looking foor aggregate)
			
	Ifc2x3::IfcRelDecomposes::list relDecompList = objectDef->Decomposes();
	for (Ifc2x3::IfcRelDecomposes::it it = relDecompList->begin(); it != relDecompList->end();++it) {		// Check if this is a part of a bigger object
		if (determine_buildingElementSemantic((*it)->RelatingObject(),sem,smapONLY,smapANY,smapPART,canBeWindowDoor,processedIDset))		// If better semantic found
			return true;
	}
	if (objectDef->is(Ifc2x3::IfcElement::Class())) {															// Check if part/contained by Site
		const Ifc2x3::IfcElement::ptr element = reinterpret_pointer_cast<Ifc2x3::IfcObjectDefinition,Ifc2x3::IfcElement>(objectDef);
		Ifc2x3::IfcRelContainedInSpatialStructure::list relContainedList = element->ContainedInStructure();
		for (Ifc2x3::IfcRelContainedInSpatialStructure::it it = relContainedList->begin();it != relContainedList->end();++it) {
			if (find_semantic((*it)->RelatingStructure(),sem,smapONLY,canBeWindowDoor))		// If better semantic found
			return true;
		}
	}
	return false;	// 
}

std::string get_semantic(const Ifc2x3::IfcObjectDefinition::ptr objectDef, semanticMAP smapONLY, semanticMAP smapANY,semanticMAP smapPART) {		
	std::string sem = "Skip " + get_IDstr(objectDef);				// Global default semantic
		
	if(!find_semantic(objectDef,sem,smapONLY,true)) {						// Check if in ONLY
		Ifc2x3::Type::Enum tp = objectDef->type();
		while (tp!=Ifc2x3::IfcRoot::Class()) {							// Check if BuildingElement
			tp = Ifc2x3::Type::Parent(tp);
			if (tp==Ifc2x3::IfcBuildingElement::Class()) {
				sem = "Anything " + get_IDstr(objectDef);				// BuildingElement default semantic
				bool canBeWindowDoor = true;//objectDef->is(Ifc2x3::IfcPlate::Class());
				determine_buildingElementSemantic(objectDef,sem,smapONLY,smapANY,smapPART,canBeWindowDoor);
				break;
	}	}	} 
	return sem + " " + Ifc2x3::Type::ToString(objectDef->type());
}


std::vector<semanticMAP> read_semanticSettings() {
	std::ifstream settings_ifstream("IFC2CityGML_Settings.ini");	
	int currSection = -1;

	std::vector<semanticMAP> semanticMV;

	if (settings_ifstream.is_open()) {
		semanticMAP currSmap;
		std::string line;
		while (currSection<3 && std::getline(settings_ifstream,line)) {
			if (line !="") {
				if (line[0]=='#') {						// If new section
					++currSection;			
					if (currSection>0) {
						semanticMV.push_back(currSmap);	// Store ONLY, ANY & PART
						currSmap.clear();				// Clear currMap
					}
				}
				else if (line[0]!='-') {
					if (currSection<0) {std::cerr << "ERROR: ONLY, ANY & PART sections should start with '#'" <<std::endl;break;}
					std::vector<std::string> tokenizedLine;
					boost::split(tokenizedLine, line, boost::is_any_of(" "));
					if (tokenizedLine.size()==2) {
						currSmap[-1][tokenizedLine[0]] = tokenizedLine[1];
					}else if (tokenizedLine.size()==3) {
						for (std::string::iterator c = tokenizedLine[0].begin(); c != tokenizedLine[0].end(); ++c) 
							*c = toupper(*c);
						currSmap[Ifc2x3::Type::FromString(tokenizedLine[0])][tokenizedLine[1]] = tokenizedLine[2];
					} else std::cerr << "WARNING: objectType [refType] outputSemantic" <<std::endl;continue;				
				}
			}
		}
		if (currSection<3) std::cerr << "ERROR: ONLY, ANY & PART sections should start with '#'" <<std::endl;
		settings_ifstream.close();
	} else {
		std::cerr << "WARNING: Can't open file: IFC2CityGML_Settings.ini" <<std::endl;
		std::map<int,std::map<std::string,std::string>> smapONLY;
		std::map<int,std::map<std::string,std::string>> smapANY;
		std::map<int,std::map<std::string,std::string>> smapPART;
		
		smapONLY[-1]["IfcWindow"]	= "Window";
		smapONLY[-1]["IfcDoor"]		= "Door";
		smapONLY[-1]["IfcSite"]		= "Site";

		smapANY[-1]["IfcRoof"]					= "Roof";
		smapANY[-1]["IfcWall"]					= "Wall";
		smapANY[-1]["IfcWallStandardCase"]		= "Wall";
		smapANY[-1]["IfcCurtainWall"]			= "Wall";
		smapANY[-1]["IfcBuildingElementProxy"]	= "Install";
		smapANY[-1]["IfcRailing"]				= "Install";
		smapANY[-1]["IfcRamp"]					= "Install";
		smapANY[-1]["IfcRampFlight"]			= "Install";
		smapANY[-1]["IfcStair"]					= "Install";
		smapANY[-1]["IfcStairFlight"]			= "Install";
		smapANY[Ifc2x3::Type::IfcSlab]["BASESLAB"]	= "Ground";
		smapANY[Ifc2x3::Type::IfcSlab]["LANDING"]	= "Install";

		smapPART[Ifc2x3::Type::IfcSlab]["ROOF"]			= "Roof";
		smapPART[Ifc2x3::Type::IfcCovering]["ROOFING"]	= "Roof";

		semanticMV.push_back(smapONLY);
		semanticMV.push_back(smapANY);
		semanticMV.push_back(smapPART);
	}	
	return semanticMV;
}
