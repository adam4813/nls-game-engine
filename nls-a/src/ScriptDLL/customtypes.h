#pragma once

#include <assert.h>
#include <angelscript.h>
#include <d3dx9math.h>
#include <NLTmx\NLTmxMap.h>

void QuatFactory(void* memory) {
	new(memory) D3DXQUATERNION(0.0f, 0.0f, 0.0f, 1.0f);
}

void QuatFactory(const float& x, const float& y, const float& z, const float& w, void* memory) {
	new(memory) D3DXQUATERNION(x, y, z, w);
}

// Wrapper to convert calling convention from Win32's silly stdcall to a normal cdecl.
D3DXQUATERNION* MyQuaternionRotationYawPitchRoll(
  __inout  D3DXQUATERNION *pOut,
  __in     FLOAT Yaw,
  __in     FLOAT Pitch,
  __in     FLOAT Roll
) {
	return D3DXQuaternionRotationYawPitchRoll(pOut, Yaw, Pitch, Roll);
}

void VectorFactory(void* memory) {
	new(memory) D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

void VectorFactory(const float& x, const float& y, const float& z, void* memory) {
	new(memory) D3DXVECTOR3(x, y, z);
}

D3DXVECTOR3 VectorApplyRotation(const D3DXVECTOR3& left, const D3DXQUATERNION& right) {
	D3DXVECTOR3 out(left);
	RotateVectorByQuaternion(&out, &right);
	return out;
}

D3DXVECTOR3 AddVectors(const D3DXVECTOR3& left, const D3DXVECTOR3& right) {
	return left + right;
}

D3DXVECTOR3 SubtractVectors(const D3DXVECTOR3& left, const D3DXVECTOR3& right) {
	return left - right;
}

float VectorMagnitudeSq(const D3DXVECTOR3& vector) {
	return D3DXVec3LengthSq(&vector);
}

float VectorMagnitude(const D3DXVECTOR3& vector) {
	return sqrt(VectorMagnitudeSq(vector));
}

D3DXVECTOR3 VectorNormalize(const D3DXVECTOR3& vector) {
	D3DXVECTOR3 out(vector);
	D3DXVec3Normalize(&out, &out);
	return out;
}

void ColorFactory(void* memory) {
	new(memory) D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
}

void ColorFactory(const float& r, const float& g, const float& b, const float& a, void* memory) {
	new(memory) D3DXCOLOR(r, g, b, a);
}



void RegisterCustomScriptTypes(asIScriptEngine *engine) {
	int ret = 0;
	
	// Quaternion rotations
	ret = engine->RegisterObjectType("Rotation", sizeof(D3DXQUATERNION), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Rotation", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(QuatFactory, (void*), void), asCALL_CDECL_OBJLAST); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Rotation", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in, const float &in, const float &in)", asFUNCTIONPR(QuatFactory, (const float&, const float&, const float&, const float&, void*), void), asCALL_CDECL_OBJLAST); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Rotation", "float x", offsetof(D3DXQUATERNION, x)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Rotation", "float y", offsetof(D3DXQUATERNION, y)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Rotation", "float z", offsetof(D3DXQUATERNION, z)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Rotation", "float w", offsetof(D3DXQUATERNION, w)); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Rotation", "Rotation SetYawPitchRoll(float, float, float)", asFUNCTION(MyQuaternionRotationYawPitchRoll), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	// 3D Vectors
	ret = engine->RegisterObjectType("Vector", sizeof(D3DXVECTOR3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Vector", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(VectorFactory, (void*), void), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Vector", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in, const float &in)", asFUNCTIONPR(VectorFactory, (const float&, const float&, const float&, void*), void), asCALL_CDECL_OBJLAST); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Vector", "float x", offsetof(D3DXVECTOR3, x)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Vector", "float y", offsetof(D3DXVECTOR3, y)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Vector", "float z", offsetof(D3DXVECTOR3, z)); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Vector", "Vector opMul(const Rotation &in) const",   asFUNCTION(VectorApplyRotation), asCALL_CDECL_OBJFIRST); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("Vector", "Vector opMul_r(const Rotation &in) const", asFUNCTION(VectorApplyRotation), asCALL_CDECL_OBJFIRST); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("Vector", "Vector opAdd(const Vector &in) const", asFUNCTION(AddVectors), asCALL_CDECL_OBJFIRST); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("Vector", "Vector opSub(const Vector &in) const", asFUNCTION(SubtractVectors), asCALL_CDECL_OBJFIRST); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("Vector", "float MagnitudeSq(const Vector &in) const", asFUNCTION(VectorMagnitudeSq), asCALL_CDECL_OBJFIRST); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("Vector", "float Magnitude(const Vector &in) const", asFUNCTION(VectorMagnitude), asCALL_CDECL_OBJFIRST); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("Vector", "Vector Normalize(const Vector &in) const", asFUNCTION(VectorNormalize), asCALL_CDECL_OBJFIRST); assert( ret >= 0 );
	
	// Color
	ret = engine->RegisterObjectType("Color", sizeof(D3DXCOLOR), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ColorFactory, (void*), void), asCALL_CDECL_OBJLAST); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(const float &in, const float &in, const float &in, const float &in)", asFUNCTIONPR(ColorFactory, (const float&, const float&, const float&, const float&, void*), void), asCALL_CDECL_OBJLAST); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Color", "float r", offsetof(D3DXCOLOR, r)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Color", "float g", offsetof(D3DXCOLOR, g)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Color", "float b", offsetof(D3DXCOLOR, b)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("Color", "float a", offsetof(D3DXCOLOR, a)); assert(ret >= 0);


	// NLTmxMap Types
	// Tileset
	ret = engine->RegisterObjectType("NLTmxMapTileset", sizeof(NLTmxMapTileset), asOBJ_REF); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapTileset", "int firstGid", offsetof(NLTmxMapTileset, firstGid)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapTileset", "string name", offsetof(NLTmxMapTileset, name)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapTileset", "int filewidth", offsetof(NLTmxMapTileset, filewidth)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapTileset", "int fileheight", offsetof(NLTmxMapTileset, fileheight)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapTileset", "int tileWidth", offsetof(NLTmxMapTileset, tileWidth)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapTileset", "int tileHeight", offsetof(NLTmxMapTileset, tileHeight)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapTileset", "string filename", offsetof(NLTmxMapTileset, filename)); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("NLTmxMapTileset", asBEHAVE_ADDREF, "void f()", asMETHOD(NLTmxMapTileset,AddRef), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMapTileset", asBEHAVE_RELEASE, "void f()", asMETHOD(NLTmxMapTileset,Release), asCALL_THISCALL); assert( ret >= 0 );

	// Layer
	ret = engine->RegisterObjectType("NLTmxMapLayer", sizeof(NLTmxMapLayer), asOBJ_REF); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapLayer", "string name", offsetof(NLTmxMapLayer, name)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapLayer", "int width", offsetof(NLTmxMapLayer, width)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapLayer", "int height", offsetof(NLTmxMapLayer, height)); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("NLTmxMapLayer", "int GetData(uint)", asMETHOD(NLTmxMapLayer, GetData), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMapLayer", asBEHAVE_ADDREF, "void f()", asMETHOD(NLTmxMapLayer,AddRef), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMapLayer", asBEHAVE_RELEASE, "void f()", asMETHOD(NLTmxMapLayer,Release), asCALL_THISCALL); assert( ret >= 0 );

	// Object
	ret = engine->RegisterObjectType("NLTmxMapObject", sizeof(NLTmxMapObject), asOBJ_REF); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObject", "string name", offsetof(NLTmxMapObject, name)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObject", "int gid", offsetof(NLTmxMapObject, gid)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObject", "int x", offsetof(NLTmxMapObject, x)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObject", "int y", offsetof(NLTmxMapObject, y)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObject", "int width", offsetof(NLTmxMapObject, width)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObject", "int height", offsetof(NLTmxMapObject, height)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObject", "PropertyMap properties", offsetof(NLTmxMapObject, properties)); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("NLTmxMapObject", asBEHAVE_ADDREF, "void f()", asMETHOD(NLTmxMapObject,AddRef), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMapObject", asBEHAVE_RELEASE, "void f()", asMETHOD(NLTmxMapObject,Release), asCALL_THISCALL); assert( ret >= 0 );

	// ObjectGroup
	ret = engine->RegisterObjectType("NLTmxMapObjectGroup", sizeof(NLTmxMapObjectGroup), asOBJ_REF); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObjectGroup", "string name", offsetof(NLTmxMapObjectGroup, name)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObjectGroup", "int width", offsetof(NLTmxMapObjectGroup, width)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObjectGroup", "int height", offsetof(NLTmxMapObjectGroup, height)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMapObjectGroup", "bool visible", offsetof(NLTmxMapObjectGroup, visible)); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("NLTmxMapObjectGroup", "NLTmxMapObject@ FirstObject()", asMETHOD(NLTmxMapObjectGroup, FirstObject), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMapObjectGroup", "NLTmxMapObject@ LastObject()", asMETHOD(NLTmxMapObjectGroup, LastObject), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMapObjectGroup", "NLTmxMapObject@ NextObject()", asMETHOD(NLTmxMapObjectGroup, NextObject), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMapObjectGroup", "NLTmxMapObject@ PrevObject()", asMETHOD(NLTmxMapObjectGroup, PrevObject), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMapObjectGroup", asBEHAVE_ADDREF, "void f()", asMETHOD(NLTmxMapObjectGroup,AddRef), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMapObjectGroup", asBEHAVE_RELEASE, "void f()", asMETHOD(NLTmxMapObjectGroup,Release), asCALL_THISCALL); assert( ret >= 0 );

	// Map
	ret = engine->RegisterObjectType("NLTmxMap", sizeof(NLTmxMap), asOBJ_REF); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMap", "int width", offsetof(NLTmxMap, width)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMap", "int height", offsetof(NLTmxMap, height)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMap", "int tileWidth", offsetof(NLTmxMap, tileWidth)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMap", "int tileHeight", offsetof(NLTmxMap, tileHeight)); assert(ret >= 0);
	ret = engine->RegisterObjectProperty("NLTmxMap", "PropertyMap properties", offsetof(NLTmxMap, properties)); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapObjectGroup@ FirstObjectGroup()", asMETHOD(NLTmxMap, FirstObjectGroup), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapObjectGroup@ LastObjectGroup()", asMETHOD(NLTmxMap, LastObjectGroup), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapObjectGroup@ NextObjectGroup()", asMETHOD(NLTmxMap, NextObjectGroup), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapObjectGroup@ PrevObjectGroup()", asMETHOD(NLTmxMap, PrevObjectGroup), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapLayer@ FirstLayer()", asMETHOD(NLTmxMap, FirstLayer), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapLayer@ LastLayer()", asMETHOD(NLTmxMap, LastLayer), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapLayer@ NextLayer()", asMETHOD(NLTmxMap, NextLayer), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapLayer@ PrevLayer()", asMETHOD(NLTmxMap, PrevLayer), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapTileset@ FirstTileset()", asMETHOD(NLTmxMap, FirstTileset), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapTileset@ LastTileset()", asMETHOD(NLTmxMap, LastTileset), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapTileset@ NextTileset()", asMETHOD(NLTmxMap, NextTileset), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectMethod("NLTmxMap", "NLTmxMapTileset@ PrevTileset()", asMETHOD(NLTmxMap, PrevTileset), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMap", asBEHAVE_ADDREF, "void f()", asMETHOD(NLTmxMap,AddRef), asCALL_THISCALL); assert( ret >= 0 );
	ret = engine->RegisterObjectBehaviour("NLTmxMap", asBEHAVE_RELEASE, "void f()", asMETHOD(NLTmxMap,Release), asCALL_THISCALL); assert( ret >= 0 );

	ret = engine->RegisterGlobalFunction("NLTmxMap@+ NLLoadTmxMap(string &in)", asFUNCTION(NLLoadTmxMap), asCALL_CDECL); assert( ret >= 0 );
	
}
