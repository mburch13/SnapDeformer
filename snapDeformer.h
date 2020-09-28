//snapDeformer.h

#ifndef snapDeformer_H
#define snapDeformer_H

#include <maya/MTypeId.h>  //Manage Maya Object type identifiers
#include <maya/MPxDeformerNode.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include "maya/MGlobal.h"

using namespace std;

class snapDeformer : public MPxDeformerNode{
  public:
    snapDeformer();
    static MStatus initialize();  //initialize node
    static void* creator();  //create node

    virtual MStatus deform(MDataBlock& data, MItGeometry& iter, const MMatrix& mat, unsigned int mIndex);  //implements core of the node
    virtual MStatus shouldSave(const MPlug& plug, bool& result);

  private:
    void initData(MObject& driverMesh, MPointArray& deformedPoints, MIntArray& bindArray, MObject& attribute);
    void ensureIndexes(MObject& attribute, int indexSize);

  public:
    //needed variables
    static MTypeId typeId;
    static MObject referencedMesh;
    static MObject rebind;
    static MObject driverMesh;
    static MObject idAssociation;
  
  private:
    unsigned int elemCount;
    MIntArray bindArray;
    bool initialized;

};

#endif
