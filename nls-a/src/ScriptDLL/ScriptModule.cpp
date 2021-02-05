/**
* \file file base name
* \author Adam Martin
* \date 2011-10-23
* \brief A manager class to load and start modules.
*
* A manager that can load/unload and start/stop modules at runtime through dynamically loaded
* libraries. The use of a common interface ModuleInterface allows us to have a uniform loading
* and starting procedure.
*/

#include "ScriptModule.h"

// Standard Includes
#include <cassert>

// Library Includes
#include <angelscript/scriptany/scriptany.h>
#include <angelscript/scriptarray/scriptarray.h>
#include <boost/lexical_cast.hpp>
#include <angelscript/scriptstdstring/scriptstdstring.h>

// Local Includes
#include "../sharedbase/MessageRouter.h"
#include "../sharedbase/PropertyMap.h"
#include "../sharedbase/EntityList.h"
#include "../sharedbase/PointList.h"
#include "EntityFactory.h"
#include "customtypes.h"
#include "sptrtypes.h"

// *HACK: This is indicating that we need to re-think how modules register their information with the script engine.
#include "../GraphicsDLL/component/RenderableComponent.h"

// Forward declares (functions in same order at bottom of file)
void ScriptLog(const LOG_PRIORITY::TYPE&, const std::string&);
void RegisterEventLogger(asIScriptEngine* const);

// Static class member initialization

// Class methods in the order they are defined within the class header

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CreateEntityFactory* CreateEntityFactoryFact( void ) {
	return new CreateEntityFactory();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned long gRandomSeed = 0;

void RandomSetSeed(unsigned long seed) {
	gRandomSeed = seed;
}

unsigned long RandomULong(unsigned long max_value) {
	// Simple linear congruential generator using the constants for Borland Delphi,
	// and Visual Pascal as given on http://en.wikipedia.org/w/index.php?title=Linear_congruential_generator&oldid=375235243
	// These contants were chosen semi-randomly; I simply ported this code over from a project of mine that required Delphi compatability. ~Ricky
	gRandomSeed = ((unsigned long long)gRandomSeed * 134775813ul + 1ul) & 0xFFFFFFFFul;
	return (unsigned long long) gRandomSeed * max_value / 0xFFFFFFFFul;
}

float RandomFloat(float max_value) {
	float value = float(RandomULong(0x7FFFFFFFul) | 0x00800000ul); // Never get so high it sets the sign bit, and prevent both Inf and NaN before conversion.
	
	return float( double(value) * max_value / float(0x7F7FFFFFul));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void SaveToDisk(const EnvelopeSPTR* envelopep, const std::string str) {
	EnvelopeSPTR env(*envelopep);
	SaveToDisk(env, str);
}

bool LoadFromDisk(const EnvelopeSPTR* envelopep, const std::string str) {
	EnvelopeSPTR env(*envelopep);
	return LoadFromDisk(env, str);
}

template<typename Target>
Target stringCast(const std::string src) {
	try
	{
		return boost::lexical_cast<Target>(src);
	}
	catch(boost::bad_lexical_cast &)
	{
		return Target();
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ScriptModule::ScriptModule( PropertyMap* gprops, MessageRouter* msgrouter, EntityList* elist ) :
	ModuleInterface(gprops, msgrouter, elist),
	deltaAccumulator(0.0)
{
	std::shared_ptr<Subscriber> sshutdown(new Subscriber(boost::bind(&ScriptModule::Shutdown, this, _1)));
	this->msgrouter->Subscribe(CORE_MESSAGE::SHUTDOWN, sshutdown);

	std::shared_ptr<Subscriber> sstartup(new Subscriber(boost::bind(&ScriptModule::Startup, this, _1)));
	this->msgrouter->Subscribe(CORE_MESSAGE::STARTUP, sstartup);

	this->eventScriptManager.InitAngelScript();
	this->updateScriptManager.InitAngelScript();
	this->eventScriptManager.CreateModule("module1");
	
	int ret = 0;
	asIScriptEngine* engine = ScriptManager::GetEngine();

	// Custom types
	RegisterScriptAny(engine);
	RegisterScriptMath(engine);
	RegisterScriptArray(engine, true);
	RegisterStdString(engine);
	RegisterStdStringUtils(engine);
	
	// Register random number system
	ret = engine->RegisterGlobalFunction("void RandomSetSeed(uint)", asFUNCTION(RandomSetSeed), asCALL_CDECL); assert(ret >= 0);
	ret = engine->RegisterGlobalFunction("uint RandomUInt(uint)", asFUNCTION(RandomULong), asCALL_CDECL); assert(ret >= 0);
	ret = engine->RegisterGlobalFunction("float RandomFloat(float)", asFUNCTION(RandomFloat), asCALL_CDECL); assert(ret >= 0);

	// Cast functions
	ret = engine->RegisterGlobalFunction("int stoi(string)", asFUNCTION(stringCast<int>), asCALL_CDECL); assert(ret >= 0);
	ret = engine->RegisterGlobalFunction("uint stou(string)", asFUNCTION(stringCast<unsigned int>), asCALL_CDECL); assert(ret >= 0);
	ret = engine->RegisterGlobalFunction("float stof(string)", asFUNCTION(stringCast<float>), asCALL_CDECL); assert(ret >= 0);
	ret = engine->RegisterGlobalFunction("double stod(string)", asFUNCTION(stringCast<double>), asCALL_CDECL); assert(ret >= 0);
	
	// Register object types
	ret = engine->RegisterObjectType("ScriptModule", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(ret >= 0);
	ret = engine->RegisterObjectType("EntityList", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(ret >= 0);
	ret = engine->RegisterObjectType("Envelope", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(ret >= 0);
	ret = engine->RegisterObjectType("PropertyMap", 0, asOBJ_REF | asOBJ_NOHANDLE); assert(ret >= 0);
	ret = engine->RegisterObjectType("PointList", 0, asOBJ_REF); assert(ret >= 0);
	ret = engine->RegisterObjectType("CreateEntityFactory", 0, asOBJ_REF); assert(ret >= 0);

	RegisterCustomScriptTypes(engine); // After the base type delcarations in case any custom types have them for member variables
	
	this->RegisterSharedPointer<Envelope>("EnvelopeSPTR", "Envelope");
	

	// Register the core messages enum
	ret = engine->RegisterEnum("CORE_MESSAGE");
	ret = engine->RegisterEnumValue("CORE_MESSAGE", "STARTUP", CORE_MESSAGE::STARTUP); assert(ret >= 0);
	ret = engine->RegisterEnumValue("CORE_MESSAGE", "SHUTDOWN", CORE_MESSAGE::SHUTDOWN); assert(ret >= 0);
	ret = engine->RegisterEnumValue("CORE_MESSAGE", "CREATE", CORE_MESSAGE::CREATE); assert(ret >= 0);
	ret = engine->RegisterEnumValue("CORE_MESSAGE", "LOADLIBRARY", CORE_MESSAGE::LOADLIBRARY); assert(ret >= 0);
	ret = engine->RegisterEnumValue("CORE_MESSAGE", "MODULESTARTED", CORE_MESSAGE::MODULESTARTED); assert(ret >= 0);
	ret = engine->RegisterEnumValue("CORE_MESSAGE", "QUIT", CORE_MESSAGE::QUIT); assert(ret >= 0);

	// Register the render space enum
	ret = engine->RegisterEnum("ProjectionStyle");
	ret = engine->RegisterEnumValue("ProjectionStyle", "PROJECTION_ORTHOGRAPHIC", GraphicsCore::ECoordinateSpace::CS_SCREEN); assert(ret >= 0);
	ret = engine->RegisterEnumValue("ProjectionStyle", "PROJECTION_PERSPECTIVE", GraphicsCore::ECoordinateSpace::CS_WORLD); assert(ret >= 0);

	// Register the ScriptModule message event registration methods
	ret = engine->RegisterGlobalProperty("ScriptModule Engine", this); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int RegisterMessageHandler(string, uint)",       asMETHOD(ScriptModule, RegisterMessageHandler),       asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int RegisterMessageHandlerByName(string, uint)", asMETHOD(ScriptModule, RegisterMessageHandlerByName), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int RegisterMessageHandlerByDecl(string, uint)", asMETHOD(ScriptModule, RegisterMessageHandlerByDecl), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int UnregisterMessageHandler(string, uint)",       asMETHOD(ScriptModule, UnregisterMessageHandler),       asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int UnregisterMessageHandlerByName(string, uint)", asMETHOD(ScriptModule, UnregisterMessageHandlerByName), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int UnregisterMessageHandlerByDecl(string, uint)", asMETHOD(ScriptModule, UnregisterMessageHandlerByDecl), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int RegisterUpdateHandler(string)",       asMETHOD(ScriptModule, RegisterUpdateHandler),       asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "int RegisterUpdateHandlerByName(string)", asMETHOD(ScriptModule, RegisterUpdateHandlerByName), asCALL_THISCALL); assert(ret >= 0);

	// Register the PropertyMap
	ret = engine->RegisterGlobalProperty("PropertyMap gprops", this->gprops); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "bool GetPropertyBool(string)",       asMETHOD(PropertyMap, GetPropertyValue<bool>),       asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "int GetPropertyInt(string)",       asMETHOD(PropertyMap, GetPropertyValue<int>),       asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "uint GetPropertyUInt(string)", asMETHOD(PropertyMap, GetPropertyValue<unsigned int>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "float GetPropertyFloat(string)", asMETHOD(PropertyMap, GetPropertyValue<float>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "double GetPropertyDouble(string)", asMETHOD(PropertyMap, GetPropertyValue<double>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "string GetPropertyString(string)", asMETHOD(PropertyMap, GetPropertyValue<std::string>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "Color GetPropertyColor(string)", asMETHOD(PropertyMap, GetPropertyValue<D3DXCOLOR>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "PointList@ GetPropertyPointList(string)", asMETHOD(PropertyMap, GetPropertyReference<PointList*>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "bool PropertyExists(string)", asMETHOD(PropertyMap, PropertyExists), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "void SetProperty(string, bool)", asMETHODPR(PropertyMap, SetProperty<bool>, (std::string, bool), void), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "void SetProperty(string, int)", asMETHODPR(PropertyMap, SetProperty<int>, (std::string, int), void), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "void SetProperty(string, uint)", asMETHODPR(PropertyMap, SetProperty<unsigned int>, (std::string, unsigned int), void), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "void SetProperty(string, float)", asMETHODPR(PropertyMap, SetProperty<float>, (std::string, float), void), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "void SetProperty(string, double)", asMETHODPR(PropertyMap, SetProperty<double>, (std::string, double), void), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "void SetProperty(string, string)", asMETHODPR(PropertyMap, SetProperty<std::string>, (std::string, std::string), void), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("PropertyMap", "void SetProperty(string, Color)", asMETHODPR(PropertyMap, SetProperty<D3DXCOLOR>, (std::string, D3DXCOLOR), void), asCALL_THISCALL); assert(ret >= 0);

	// Logger
	RegisterEventLogger(engine);

	// PointList
	ret = engine->RegisterObjectBehaviour("PointList", asBEHAVE_ADDREF,  "void f()", asMETHOD(PointList, AddRef),  asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("PointList", asBEHAVE_RELEASE, "void f()", asMETHOD(PointList, Release), asCALL_THISCALL); assert(ret >= 0);
	// *TODO: Register the rest of the PointList API and Point type if script needs complete interaction.


	// Entity
	RegisterEntity(engine);

	// Register the Envelope type and methods
	ret = engine->RegisterObjectProperty("Envelope", "int msgid", offsetof(Envelope, msgid)); assert (ret >= 0);
	
	ret = engine->RegisterObjectMethod("Envelope", "uint GetCount()", asMETHOD(Envelope, GetCount), asCALL_THISCALL); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Envelope", "bool         GetDataBool        (const uint &in)", asMETHOD(Envelope, GetDataValue<bool          >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "int          GetDataInt         (const uint &in)", asMETHOD(Envelope, GetDataValue<int           >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "int64        GetDataLong        (const uint &in)", asMETHOD(Envelope, GetDataValue<long          >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "uint         GetDataUInt        (const uint &in)", asMETHOD(Envelope, GetDataValue<unsigned int  >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "float        GetDataFloat       (const uint &in)", asMETHOD(Envelope, GetDataValue<float         >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "string       GetDataString      (const uint &in)", asMETHOD(Envelope, GetDataValue<std::string   >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "Vector       GetDataVector      (const uint &in)", asMETHOD(Envelope, GetDataValue<D3DXVECTOR3   >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "Rotation     GetDataQuat        (const uint &in)", asMETHOD(Envelope, GetDataValue<D3DXQUATERNION>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "Color        GetDataColor       (const uint &in)", asMETHOD(Envelope, GetDataValue<D3DXCOLOR     >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "Entity       GetDataEntity      (const uint &in)", asMETHOD(Envelope, GetDataReference<EntitySPTR  >), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "EnvelopeSPTR GetDataEnvelopeSPTR(const uint &in)", asMETHOD(Envelope, GetDataReference<EnvelopeSPTR>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "PointList@   GetDataPointList   (const uint &in)", asMETHOD(Envelope, GetDataReference<PointList*  >), asCALL_THISCALL); assert(ret >= 0);

	ret = engine->RegisterObjectMethod("Envelope", "void AddDataBool        (const bool &in)",         asMETHOD(Envelope, AddDataValue<bool>          ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataInt         (const int &in)",          asMETHOD(Envelope, AddDataValue<int>           ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataUInt        (const uint &in)",         asMETHOD(Envelope, AddDataValue<unsigned int>  ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataFloat       (const float &in)",        asMETHOD(Envelope, AddDataValue<float>         ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataString      (const string &in)",       asMETHOD(Envelope, AddDataValue<std::string>   ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataVector      (const Vector &in)",       asMETHOD(Envelope, AddDataValue<D3DXVECTOR3>   ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataQuat        (const Rotation &in)",     asMETHOD(Envelope, AddDataValue<D3DXQUATERNION>), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataColor       (const Color &in)",        asMETHOD(Envelope, AddDataValue<D3DXCOLOR>     ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataEntity      (Entity)",       asMETHOD(Envelope, AddDataReference<EntitySPTR>    ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataEnvelopeSPTR(EnvelopeSPTR)", asMETHOD(Envelope, AddDataReference<EnvelopeSPTR>  ), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Envelope", "void AddDataPointList   (PointList@)",   asMETHOD(Envelope, AddDataReference<PointList*>    ), asCALL_THISCALL); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("EnvelopeSPTR", "void SaveToDisk(const string)",   asFUNCTIONPR(SaveToDisk  , (const EnvelopeSPTR*, const std::string), void), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("EnvelopeSPTR", "bool LoadFromDisk(const string)", asFUNCTIONPR(LoadFromDisk, (const EnvelopeSPTR*, const std::string), bool), asCALL_CDECL_OBJFIRST); assert(ret >= 0);

	ret = engine->RegisterObjectMethod("ScriptModule", "EnvelopeSPTR EnvelopeSPTRFactory()", asMETHOD(ScriptModule, EnvelopeSPTRFactory), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "void Send(EnvelopeSPTR)", asMETHOD(ScriptModule, SendMessage), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "void PrioritySend(EnvelopeSPTR)", asMETHOD(ScriptModule, PrioritySend), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptModule", "EnvelopeSPTR DirectedSend(EnvelopeSPTR)", asMETHOD(ScriptModule, DirectedSend), asCALL_THISCALL); assert(ret >= 0);

	// Entity Manager
	ret = engine->RegisterGlobalProperty("EntityList EntList", this->elist); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("EntityList", "bool AddEntity(Entity)", asMETHODPR(EntityList, AddEntity, (EntitySPTR const), bool), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("EntityList", "bool AddEntity(string, Entity)", asMETHODPR(EntityList, AddEntity, (const std::string, EntitySPTR const), bool), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("EntityList", "Entity FindEntity(string)", asMETHODPR(EntityList, FindEntity, (const std::string) const, EntitySPTR), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("EntityList", "bool RemoveEntity(string)", asMETHODPR(EntityList, RemoveEntity, (const std::string), bool), asCALL_THISCALL); assert(ret >= 0);

	CreateEntityFactory::engine = engine;

	// CreateEntityFactory
	ret = engine->RegisterObjectBehaviour("CreateEntityFactory", asBEHAVE_ADDREF,  "void f()", asMETHOD(CreateEntityFactory, Addref),  asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("CreateEntityFactory", asBEHAVE_RELEASE, "void f()", asMETHOD(CreateEntityFactory, Release), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("CreateEntityFactory", "void SetName(string)", asMETHOD(CreateEntityFactory, SetName), asCALL_THISCALL);
	ret = engine->RegisterObjectMethod("CreateEntityFactory", "void AddAttribute(string, string, string)", asMETHODPR(CreateEntityFactory, AddAttribute, (std::string, std::string, std::string), void), asCALL_THISCALL);
	ret = engine->RegisterObjectMethod("CreateEntityFactory", "void AddAttribute(string, string, any@)", asMETHODPR(CreateEntityFactory, AddAttribute, (std::string, std::string, CScriptAny*), void), asCALL_THISCALL);
	ret = engine->RegisterObjectMethod("CreateEntityFactory", "void SetEntity(Entity)", asMETHOD(CreateEntityFactory, SetEntity), asCALL_THISCALL);
	ret = engine->RegisterObjectMethod("CreateEntityFactory", "void CreateEntity(int)", asMETHOD(CreateEntityFactory, CreateEntity), asCALL_THISCALL);
	ret = engine->RegisterObjectMethod("CreateEntityFactory", "void Finalize(EnvelopeSPTR)", asMETHOD(CreateEntityFactory, Finalize), asCALL_THISCALL);
	ret = engine->RegisterObjectBehaviour("CreateEntityFactory", asBEHAVE_FACTORY, "CreateEntityFactory@ CreateEntityFactoryFact()", asFUNCTION(CreateEntityFactoryFact), asCALL_CDECL); assert(ret >= 0);
	ret = 0;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ScriptModule::~ScriptModule() {
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptModule::SendMessage( EnvelopeSPTR envelope ) const {
	this->msgrouter->SendSP(envelope);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptModule::PrioritySend( EnvelopeSPTR envelope ) const {
	this->msgrouter->SendSP(envelope, false);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
EnvelopeSPTR ScriptModule::DirectedSend( EnvelopeSPTR envelope ) const {
	return this->msgrouter->SendDirected(envelope, envelope->msgid);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptModule::Message( EnvelopeSPTR envelope ) {
	event_handler_list_type event_handlers;
	
	{
		Threading::ReadLock r_lock(this->eventHandlersMutex);
		
		event_handler_map_type::iterator iter = this->eventHandlers.find(envelope->msgid);
		if (iter != this->eventHandlers.end()) {
			event_handlers = (*iter).second;
		}
	}
	
	BOOST_FOREACH (event_handler_type handler, event_handlers) {
		int ret = 0;
		ret = this->eventScriptManager.PrepareFunction(handler);
		
		ret = this->eventScriptManager.SetFunctionParamObject(0, &envelope);
		
		ret = this->eventScriptManager.ExecuteFunction();
		if (ret == asEXECUTION_FINISHED) {
			// Handle return value.  None in this case.
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::RegisterMessageHandler( std::string funcName, event_handler_key_type eventID ) {
	return RegisterMessageHandlerByName(funcName, eventID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::RegisterMessageHandlerByName( std::string funcName, event_handler_key_type eventID ) {
	unsigned int funcID = this->eventScriptManager.GetEngine()->GetModule("module1")->GetFunctionIdByName(funcName.c_str());
	return RegisterMessageHandlerByID(funcID, eventID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::RegisterMessageHandlerByDecl( std::string funcDeclaration, event_handler_key_type eventID ) {
	unsigned int funcID = this->eventScriptManager.GetEngine()->GetModule("module1")->GetFunctionIdByDecl(funcDeclaration.c_str());
	return RegisterMessageHandlerByID(funcID, eventID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::RegisterMessageHandlerByID( unsigned int func_id, event_handler_key_type event_id ) {
	event_handler_list_type* handlers;
	
	Threading::UpgradeLock up_lock(this->eventHandlersMutex);
	
	// Find the event ID in the handlers list.
	event_handler_map_type::iterator handlers_it = this->eventHandlers.find(event_id);
	
	// If it was found, go get it
	if (handlers_it != this->eventHandlers.end()) {
		handlers = &handlers_it->second;
	}
	else { // Otherwise, set it up
		// Subscribe the script module's message handler to the event id
		std::shared_ptr<Subscriber> s2(new Subscriber(boost::bind(&ScriptModule::Message, this, _1)));
		this->msgrouter->Subscribe(event_id, s2);
		
		Threading::UpgradeToUniqueLock unique_lock(up_lock);
		
		// Create the handler set
		handlers = &this->eventHandlers[event_id];
	}
	
	// Only add the handler if it doesn't exist already (which will never be the case if this is the first subscription call!)
	if (handlers->find(event_handler_type(func_id)) == handlers->end()) {
		Threading::UpgradeToUniqueLock unique_lock(up_lock);
		
		// Append the script handler function id to the end of the set.
		handlers->insert(event_handler_type(func_id));
	}

	return func_id;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::UnregisterMessageHandler( std::string funcName, event_handler_key_type eventID ) {
	return UnregisterMessageHandlerByName(funcName, eventID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::UnregisterMessageHandlerByName( std::string funcName, event_handler_key_type eventID ) {
	unsigned int funcID = this->eventScriptManager.GetEngine()->GetModule("module1")->GetFunctionIdByName(funcName.c_str());
	return UnregisterMessageHandlerByID(funcID, eventID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::UnregisterMessageHandlerByDecl( std::string funcDeclaration, event_handler_key_type eventID ) {
	unsigned int funcID = this->eventScriptManager.GetEngine()->GetModule("module1")->GetFunctionIdByDecl(funcDeclaration.c_str());
	return UnregisterMessageHandlerByID(funcID, eventID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::UnregisterMessageHandlerByID( unsigned int funcID, event_handler_key_type eventID ) {
	if (funcID >= 0) {
		Threading::WriteLock w_lock(this->eventHandlersMutex);
		
		event_handler_list_type* handlers = &this->eventHandlers[eventID];
		
		// Remove the handler from the set.
		handlers->erase(event_handler_type(funcID));
		
		// *TODO: If the memory leak gets to be too much, then check if the handler set is empty.  If it is, remove it's entry from the eventHandlers map and unsubscribe ScriptModule::Message from that event ID.
	}

	return funcID;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptModule::CreateComponent( std::string type, std::map< std::string, boost::any > &attributes, EntitySPTR entity ) {
	if (type == "Script_File") {
		int ret;
		ret = this->eventScriptManager.AddSectionFromFile(boost::any_cast<std::string>(attributes["filename"]).c_str()); assert(ret >= 0);
		ret = this->eventScriptManager.BuildModule(); assert(ret >= 0);
		ret = this->eventScriptManager.PrepareFunction(boost::any_cast<std::string>(attributes["function"]).c_str(), "module1"); assert(ret >= 0);
		ret = this->eventScriptManager.ExecuteFunction(); assert(ret >= 0);
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
WHO_DELETES::TYPE ScriptModule::RemoveComponent(ComponentInterface* component) {
	// Nothing to do here
	return WHO_DELETES::CALLEE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptModule::Startup( EnvelopeSPTR envelope ) {
	int ret = 0;

	ret = this->eventScriptManager.AddSectionFromFile(STARTUP_SCRIPT.c_str());
	if (ret < 0) {
		LOG(LOG_PRIORITY::SYSERR, "Script '" + STARTUP_SCRIPT + "' failed to load!  Does the file exist at the given path?");
		
		// *TODO: Pop up some kind of dialog notifying of failure and need to look at log.
		
		// Kill the engine
		EnvelopeSPTR envelope(new Envelope);
		envelope->msgid = CORE_MESSAGE::QUIT;
		this->msgrouter->SendSP(envelope, false);
		
		return;
	}

	ret = this->eventScriptManager.BuildModule();
	if (ret < 0) {
		LOG(LOG_PRIORITY::SYSERR, "Script '" + STARTUP_SCRIPT + "' failed to build. Possibly a syntax error in the script?");
		
		// *TODO: Pop up some kind of dialog notifying of failure and need to look at log.
		
		// Kill the engine
		EnvelopeSPTR envelope(new Envelope);
		envelope->msgid = CORE_MESSAGE::QUIT;
		this->msgrouter->SendSP(envelope, false);
		
		return;
	}

	this->eventScriptManager.PrepareFunction("void main(void)", "module1");
	this->eventScriptManager.ExecuteFunction(); // *TODO make sure that no messages are sent during this function as message dispatcher isn't ready.

	LOG(LOG_PRIORITY::INFO, "Script startup complete. All scripts loaded and built.");
	//return true;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptModule::Shutdown( EnvelopeSPTR envelope ) {

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptModule::Update( double dt ) {
	this->deltaAccumulator += dt;

	if (this->deltaAccumulator >= 1.0f / 30.0f) {
		int ret = 0, handler = -1, size;
		
		// Iterate through ONLY the items added before this update round,
		// not through any added during.
		size = this->updateHandlers.size();
		while (--size >= 0) {
			{
				Threading::WriteLock w_lock(this->updateHandlersMutex);
				handler = this->updateHandlers.front();
				this->updateHandlers.pop();
			}
			
			ret = this->updateScriptManager.PrepareFunction(handler);
			
			ret = this->updateScriptManager.SetFunctionParamDouble(0, dt);
			
			ret = this->updateScriptManager.ExecuteFunction();
			if (ret == asEXECUTION_FINISHED) {
				// Handle return value.  None in this case.
			}
		}
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::RegisterUpdateHandler( std::string funcName ) {
	return RegisterUpdateHandlerByName(funcName);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::RegisterUpdateHandlerByName( std::string funcName ) {
	unsigned int funcID = this->updateScriptManager.GetEngine()->GetModule("module1")->GetFunctionIdByName(funcName.c_str());
	return RegisterUpdateHandlerByID(funcID);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
unsigned int ScriptModule::RegisterUpdateHandlerByID( unsigned int funcID ) {
	Threading::WriteLock w_lock(this->updateHandlersMutex);
	
	// Append the handler to the end of the queue.
	this->updateHandlers.push(event_handler_type(funcID));
	
	return funcID;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template <typename Type>
void ScriptModule::RegisterSharedPointer( const std::string& ptrTypeName, const std::string& typeName ) {
	asIScriptEngine* engine = this->eventScriptManager.GetEngine();

	int r;

	r = engine->RegisterObjectType(ptrTypeName.c_str(), sizeof(std::shared_ptr<Type>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA); assert(r >= 0);

	r = engine->RegisterObjectBehaviour(ptrTypeName.c_str(), asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(SharedPointerWrapper<Type>::construct), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour(ptrTypeName.c_str(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(SharedPointerWrapper<Type>::destruct), asCALL_CDECL_OBJLAST); assert(r >= 0);

	r = engine->RegisterObjectMethod(ptrTypeName.c_str(), (ptrTypeName + "& opAssign(const " + ptrTypeName + " &in other)").c_str(), asMETHODPR(std::shared_ptr<Type>, operator=, (std::shared_ptr<Type> const &), std::shared_ptr<Type>&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(ptrTypeName.c_str(), (typeName + "& get()").c_str(), asMETHOD(std::shared_ptr<Type>, get), asCALL_THISCALL); assert(r >= 0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
EnvelopeSPTR ScriptModule::EnvelopeSPTRFactory() {
	EnvelopeSPTR ptr(new Envelope);
	return ptr;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ScriptLog(const LOG_PRIORITY::TYPE& priority, const std::string& string) {
	asIScriptContext *ctx = asGetActiveContext();
	
	std::string message;
	
	if (ctx != nullptr) {
		asIScriptFunction* func = ctx->GetFunction();
		::EventLogger::GetEventLogger()->LogToDisk(priority, string, func->GetScriptSectionName(), static_cast<unsigned int>(ctx->GetLineNumber()), func->GetDeclaration());
	}
	else {
		::EventLogger::GetEventLogger()->LogToDisk(priority, string);
	}
}

void RegisterEventLogger(asIScriptEngine* const engine) {
	int ret;
	
	ret = engine->RegisterEnum("LOG");
	ret = engine->RegisterEnumValue("LOG", "INFO", LOG_PRIORITY::INFO); assert(ret >= 0);
	ret = engine->RegisterEnumValue("LOG", "FLOW", LOG_PRIORITY::FLOW); assert(ret >= 0);
	ret = engine->RegisterEnumValue("LOG", "WARN", LOG_PRIORITY::WARN); assert(ret >= 0);
	ret = engine->RegisterEnumValue("LOG", "CONFIG", LOG_PRIORITY::CONFIG); assert(ret >= 0);
	ret = engine->RegisterEnumValue("LOG", "ERR", LOG_PRIORITY::ERR); assert(ret >= 0);
	ret = engine->RegisterEnumValue("LOG", "MISSRESS", LOG_PRIORITY::MISSRESS); assert(ret >= 0);
	ret = engine->RegisterEnumValue("LOG", "DEPRICATE", LOG_PRIORITY::DEPRICATE); assert(ret >= 0);
	
	ret = engine->RegisterGlobalFunction("void Log(const LOG& in, const string& in)", asFUNCTION(ScriptLog), asCALL_CDECL); assert(ret >= 0);
}
