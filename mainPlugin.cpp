//McKenzie Burch
//Rigging Dojo Maya API
//August 2020
//mainPlugin.cpp
//standard setup for event node

#include "snapDeformer.h"  //change include header for each node

#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h> //Maya class that redisters and deregisters plug-ins with Maya

MStatus initializePlugin(MObject obj){
  MStatus status;
  MFnPlugin fnplugin(obj, "McKenzie Burch", "1.0", "Any");

  //register a new dependency node with Maya
  status = fnplugin.registerNode("snapDeformer", snapDeformer::typeId, snapDeformer::creator, snapDeformer::initialize, snapDeformer::kDeformerNode);

  if(status != MS::kSuccess)
    status.perror("Could not regiser the poseReader node");

  return status;
}

MStatus uninitializePlugin(MObject obj){
  MFnPlugin pluginFn;

  //deregister the given user defined dependency node type Maya
  pluginFn.deregisterNode(MTypeId(0x00108b13));

  return MS::kSuccess;
}
