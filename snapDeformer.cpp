//snapDeformer.cpp
#include "snapDeformer.h"

#include <maya/MItGeometry.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include "maya/MDataHandle.h"
#include "maya/MArrayDataHandle.h"
#include <maya/MPlug.h>

#define SMALL (float)1e-6
#define BIG_DIST 99999

MTypeId snapDeformer::typeId(0x00108b13);  //define value for typeId

//needed attributes
MObject snapDeformer::referencedMesh;
MObject snapDeformer::rebind;
MObject snapDeformer::driverMesh;
MObject snapDeformer::idAssociation;

snapDeformer::snapDeformer(){
  initialized = 0;
  elemCount = 0;
  bindArray = MIntArray();
}

void* snapDeformer::creator(){
  return new snapDeformer();
}

MStatus snapDeformer::initialize(){
  MFnNumericAttribute numAttr;
  MFnTypedAttribute tAttr;

  rebind = numAttr.create("rebind", "rbn", MFnNumericData::kBoolean, 0);
  numAttr.setKeyable(true);
  numAttr.setStorable(true);
  addAttribute(rebind);

  idAssociation = numAttr.create("idAssociation", "ida", MFnNumericData::kInt, 0);
  numAttr.setKeyable(true);
  numAttr.setStorable(true);
  numAttr.setArray(true);
  addAttribute(idAssociation);

  driverMesh = tAttr.create("driverMesh", "drm", MFnData::kMesh);
  tAttr.setKeyable(true);
  tAttr.setStorable(true);
  addAttribute(driverMesh);

  attributeAffects(driverMesh, outputGeom);
  attributeAffects(rebind, outputGeom);

  MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer snapDeformer weights");

  return MS::kSuccess;
}

MStatus snapDeformer::deform(MDataBlock& data, MItGeometry& iter, const MMatrix& localToWorldMatrix, unsigned int mIndex){
  //check if driver mesh is connected
  MPlug driverMeshPlug(thisMObject(), driverMesh);
  if(driverMeshPlug.isConnected() == false){
    return MS::kNotImplemented;
  }

  MObject driverMeshV = data.inputValue(driverMesh).asMesh();
  MArrayDataHandle idAssociationV = data.inputArrayValue(idAssociation);  //handle for arrays accessing array value
  idAssociationV.jumpToArrayElement(0);
  double envelopeV = data.inputValue(envelope).asFloat();
  bool rebindV = data.inputValue(rebind).asBool();

  //if envelope is not 0
  if(envelopeV < SMALL){
    return MS::kSuccess;
  }

  //driver points
  MPointArray driverPoint;
  MFnMesh driverGeoFn(driverMeshV);
  driverGeoFn.getPoints(driverPoint, MSpace::kWorld);

  //get input point
  MPointArray pos;
  iter.allPositions(pos, MSpace::kWorld);

  //check for rebind
  if(rebindV == 1){
    initData(driverMeshV, pos, bindArray, idAssociation);
  }

  if(elemCount == 0){
    elemCount = iter.exactCount();
  }

  //check if bind array is empty or messed up
  int arrayLength = bindArray.length();
  if(elemCount != arrayLength || initialized == 0 || arrayLength == 0){
    ensureIndexes(idAssociation, iter.exactCount()); //read from attributes

    //loop attribute and read value
    MArrayDataHandle idsHandle = data.inputArrayValue(idAssociation);
    idsHandle.jumpToArrayElement(0);

    int count = iter.exactCount();
    bindArray.setLength(count);
    for(int i = 0; i < count; i++, idsHandle.next()){
      bindArray[i] = idsHandle.inputValue().asInt();
    }

    //set value in controller variables
    elemCount = count;
    initialized = 1;
  }

  if(elemCount != iter.exactCount()){
    MGlobal::displayError("Mismatch between saved bind index and current mesh vertex, please rebind");
    return MS::kSuccess;
  }

  //loop all elements and set the size
  MVector delta, current, target;
  for(int i=0; i<elemCount; i++){
    delta = driverPoint[bindArray[i]] - pos[i]; //compute delta from position and final posistion
    pos[i] = pos[i] + (delta*envelopeV); //scale the delta by the envelope amount
  }

  iter.setAllPositions(pos);
  return MS::kSuccess;
}

MStatus snapDeformer::shouldSave(const MPlug& plug, bool& result){
  //based on attribute name determine what to save
  //save everything
  result = true;
  return MS::kSuccess;
}

void snapDeformer::initData(MObject& driverMesh, MPointArray& deformedPoints, MIntArray& bindArray, MObject& attribute){
  int count = deformedPoints.length();
  bindArray.setLength(count);

  //declare needed functions sets and get all points
  MFnMesh meshFn(driverMesh);
  MItMeshPolygon faceIter(driverMesh);
  MPointArray driverPoints;
  meshFn.getPoints(driverPoints, MSpace::kWorld);

  //declare all the needed variables of the loop
  MPlug attrPlug(thisMObject(), attribute);
  MDataHandle handle;
  MPoint closest;
  int closestFace, oldIndex, minId;
  unsigned int v;
  MIntArray vertices;
  double minDist, dist;
  MVector base, end, vec;

  for(int i=0; i<count; i++){
    meshFn.getClosestPoint(deformedPoints[i], closest, MSpace::kWorld, &closestFace); //closest face

    //find closest vertex
    faceIter.setIndex(closestFace, oldIndex);
    vertices.setLength(0);
    faceIter.getVertices(vertices);

    //convert MPoint to MVector
    base = MVector(closest);
    minDist = BIG_DIST;

    for(v=0; v<vertices.length(); v++){
      end = MVector(driverPoints[vertices[v]]);
      vec = end - base;
      dist = vec.length();

      if(dist < minDist){
        minDist = dist;
        minId = vertices[v];
      }
    }
    bindArray[i] = int(minId);

    //ensure we got the attribute indicies
    attrPlug.selectAncestorLogicalIndex(i, attribute);
    attrPlug.getValue(handle);
    attrPlug.setValue(minId);

  }

  initialized = 1;
  elemCount = count;
}

void snapDeformer::ensureIndexes(MObject& attribute, int indexSize){
  //loops the index in order to creat them if needed
  MPlug attrPlug(thisMObject(), attribute);
  MDataHandle handle;

  for(int i=0; i<indexSize; i++){
    attrPlug.selectAncestorLogicalIndex(i, attribute);
    attrPlug.getValue(handle);
  }

}
