
| :exclamation:  We do not maintain this code anymore!                    |
|-------------------------------------------------------------------------|
| Improved code is available at: https://github.com/tudelft3d/esri_geobim |



# ifc2citygml

Automatic conversion of IFC models v2x3 to CityGML LOD3 (but LOD2 and LOD4 are possible too).

Description of what can be done with this code is in the MSc thesis of Sjors Donkers entitled [*Automatic generation of CityGML LoD3 building models from IFC models*](http://repository.tudelft.nl/view/ir/uuid%3A31380219-f8e8-4c66-a2dc-548c3680bb8d/).

It is also published as:

Sjors Donkers, Hugo Ledoux, Junqiao Zhao and Jantien Stoter (in press). *Automatic conversion of IFC datasets to geometrically and semantically correct CityGML LOD3 buildings*. Transactions in GIS. [pdf](https://3d.bk.tudelft.nl/hledoux/pdfs/16_tgis_ifcitygml.pdf) [doi](http://dx.doi.org/10.1111/tgis.12162)

## Code

The code is split in 2 parts:

  1. **ifc2off**: converts the relevant geometries in an IFC file to a set of polygons in an OFF file. Dependencies: [IfcOpenShell](http://ifcopenshell.org) and [OpenCascade](http://www.opencascade.org).
  2. **off2cityml**: converts the OFF file to a valid LOD3 CityGML file. Dependencies: [CGAL](http://www.cgal.org)

