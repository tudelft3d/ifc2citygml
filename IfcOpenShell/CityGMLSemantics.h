#ifndef CITYGML_SEMANTICS_GRIVY
#define CITYGML_SEMANTICS_GRIVY

#include "../ifcparse/IfcParse.h"
#include <set>
#include <boost/algorithm/string.hpp>
typedef std::map<int,std::map<std::string,std::string>>		semanticMAP;


Ifc2x3::IfcSlabTypeEnum::IfcSlabTypeEnum getSlabType(const Ifc2x3::IfcSlab::ptr slab);
Ifc2x3::IfcCoveringTypeEnum::IfcCoveringTypeEnum getCoveringType(const Ifc2x3::IfcBuildingElement::ptr element);
std::string get_IDstr(Ifc2x3::IfcObjectDefinition* objectDef);
bool find_semantic(Ifc2x3::IfcObjectDefinition* objectDef, std::string& sem, semanticMAP sMap,bool canBeWindow=true);
bool determine_buildingElementSemantic(Ifc2x3::IfcObjectDefinition* objectDef,std::string& sem,semanticMAP smapONLY,semanticMAP smapANY,semanticMAP smapPART,bool canBeWindow=true, std::set<unsigned int> processedIDset=std::set<unsigned int>());
std::string get_semantic(const Ifc2x3::IfcObjectDefinition::ptr objectDef, semanticMAP smapONLY, semanticMAP smapANY,semanticMAP smapPART);
std::vector<semanticMAP> read_semanticSettings();

#endif