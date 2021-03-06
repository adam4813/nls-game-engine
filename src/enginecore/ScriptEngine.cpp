/**
* \file
* \author       Adam Martin
* \date         2012-04-14
* \brief        ScriptEngine definitions. Also contains stringCast utility declaration and definition.
* 
*/

#include "ScriptEngine.h"

// System Library Includes
#include <cassert>

// Application Library Includes
#include <boost/lexical_cast.hpp>
#include <angelscript/scriptany.h>
#include <angelscript/scriptarray.h>
#include <angelscript/scriptdictionary.h>
#include <angelscript/scriptstdstring.h>

// Local Includes
#include "../sharedbase/EventLogger.h"

// Static class member initialization

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Helper function prototypes
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
std::string GetPreviousCallstackLine(const unsigned int& = 1);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Class methods in the order they are defined within the class header
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
* \brief A simple function to cast a string type to a Target type
* \param[in] src The source string.
* \return The lexical_cast of string to Target, or a default constructed instance of Target.
*/
template<typename Target>
Target stringCast(const std::string& src) {
	try
	{
		return boost::lexical_cast<Target>(src);
	}
	catch(boost::bad_lexical_cast &)
	{
		return Target();
	}
}

ScriptEngine::ScriptEngine() : engine(nullptr), isRunning(true) {
	int ret = 0;

	// Create the script engine
	this->engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	if (ScriptEngine::engine == nullptr) {
		LOG(LOG_PRIORITY::ERR, "Angelscript has failed to start.");
		assert(false /* If the engine fails to init, this is a BIG problem in the source or logic flow. */);
	}

	// Set the message callback to receive information on errors in human readable form.
	ret = this->engine->SetMessageCallback(asMETHOD(ScriptEngine, MessageCallback), this, asCALL_THISCALL); assert(ret >=0);

	// Create our context we will use for enginecore
	asIScriptContext* ctx = this->engine->CreateContext(); assert( ctx != nullptr );

	// Set the exception callback to receive information on errors in human readable form.
	ret = ctx->SetExceptionCallback(asMETHOD(ScriptEngine, ExceptionCallback), this, asCALL_THISCALL); assert(ret >=0);

	this->scriptexec.SetContext(ctx);
	this->builder.StartNewModule(this->engine, "enginecore");

	// Script add-ons
	RegisterScriptAny(this->engine);
	RegisterScriptArray(this->engine, true);
	RegisterStdString(this->engine); // Must be AFTER array registration
	RegisterStdStringUtils(this->engine);
	RegisterScriptDictionary(this->engine); // Must be AFTER string registration
	ScriptEngine::RegisterMathTypes(this->engine);

	// String Cast functions
	ret = this->engine->RegisterGlobalFunction("int stoi(string)", asFUNCTION(stringCast<int>), asCALL_CDECL); assert(ret >= 0);
	ret = this->engine->RegisterGlobalFunction("uint stou(string)", asFUNCTION(stringCast<unsigned int>), asCALL_CDECL); assert(ret >= 0);
	ret = this->engine->RegisterGlobalFunction("float stof(string)", asFUNCTION(stringCast<float>), asCALL_CDECL); assert(ret >= 0);
	ret = this->engine->RegisterGlobalFunction("double stod(string)", asFUNCTION(stringCast<double>), asCALL_CDECL); assert(ret >= 0);

	// This is in the global namespace like all global singleton instances.
	ret = engine->RegisterObjectType("ScriptEngine", 0, asOBJ_REF | asOBJ_NOHANDLE ); assert(ret >= 0);
	ret = engine->RegisterGlobalProperty("ScriptEngine Engine", this); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptEngine", "void Shutdown()", asMETHOD(ScriptEngine, Shutdown), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptEngine", "void SetUserDataFolder(const string &in)", asMETHOD(ScriptEngine, SetUserDataFolder), asCALL_THISCALL); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("ScriptEngine", "void SetGameScript(const string &in)", asMETHOD(ScriptEngine, SetGameScript), asCALL_THISCALL); assert(ret >= 0);

	ret = this->engine->SetDefaultNamespace("Engine"); assert(ret >= 0);

	ret = this->engine->RegisterGlobalFunction("void AbortExecution()", asFUNCTION(ScriptEngine::AbortExecution), asCALL_CDECL); assert(ret >= 0);

	// Register callstack analysis commands for enabling unit test systems.
	ret = this->engine->SetDefaultNamespace("Engine::Debug"); assert(ret >= 0);
	ret = this->engine->RegisterGlobalFunction("string GetPreviousCallstackLine(const uint &in a = 1)", asFUNCTION(GetPreviousCallstackLine), asCALL_CDECL); assert(ret >= 0);
	// *NOTE: This function is designed to, normally, return the previous line of execution from the call stack.  Hence the parameter is defaulted to 1, and the name is as it is: GetPreviousCallstackLine.
	//  This distinction is helpful when the function is used as it is supposed to be used: as a part of a simple message to the log from a script-based error handling system like the unit test framework.
	//  However, other uses will need to set the value differently.  Just to document the result: a passed value of 0 will return the current line of execution - whatever use that might be.

	ret = this->engine->SetDefaultNamespace(""); assert(ret >= 0);
}

ScriptEngine::~ScriptEngine() {
	if (this->engine != nullptr) {
		if (this->engine->Release() <= 0) {
			this->engine = nullptr;
		}
	}
}

/** 
* \param[in] msg A message info packet containing the details about the message generated by Angelscript.
*/
void ScriptEngine::MessageCallback(const asSMessageInfo *msg) {
	if (msg == nullptr) {
		LOG(LOG_PRIORITY::ERR, "asSMessageInfo was null. Something must have happened to the engine.");
		return;
	}
	std::string type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING) {
		type = "WARN";
	}
	else if (msg->type == asMSGTYPE_INFORMATION) {
		type = "INFO";
	}
	try {
		LOG(LOG_PRIORITY::WARN, 
			type + 
			": " + boost::lexical_cast<std::string>(msg->section) + 
			" at line " + boost::lexical_cast<std::string>(msg->row) + 
			", column " + boost::lexical_cast<std::string>(msg->col) + 
			": " + msg->message
			);
	}
	catch (boost::bad_lexical_cast* badcast) {
		LOG(LOG_PRIORITY::ERR, badcast->what());
	}
}

/** 
* \param[in] ctx The currently executing context. Certain information can be obtained from the engine about the executing function.
*/
void ScriptEngine::ExceptionCallback(asIScriptContext* ctx) {
	if (ctx == nullptr) {
		LOG(LOG_PRIORITY::ERR, "asIScriptContext was null. Something must have happened to the engine.");
		return;
	}
	int line, column;
	line = ctx->GetExceptionLineNumber(&column); assert( line >= 0 );
	const asIScriptFunction* function = ctx->GetExceptionFunction(); assert( function != nullptr );

	try {
		std::string errStr = 
			"Exception in script. Module " + std::string(function->GetModuleName()) +
			", section " + function->GetScriptSectionName() +
			", function " + function->GetDeclaration() +
			", line " + boost::lexical_cast<std::string>(line) +
			", column " + boost::lexical_cast<std::string>(column) +
			"\n\tMessage: " + ctx->GetExceptionString()
			;
		LOG(LOG_PRIORITY::ERR, errStr);
	}
	catch (boost::bad_lexical_cast* badcast) {
		LOG(LOG_PRIORITY::ERR, badcast->what());
	}
}

/**
* \return The newly created ScriptExecutor or nullptr if engine is nullptr. Ownership is transfered to the caller. 
*/
ScriptExecutor* ScriptEngine::ScriptExecutorFactory() {
	if (this->engine != nullptr) {
		ScriptExecutor* exec = new ScriptExecutor();

		asIScriptContext* ctx = this->engine->CreateContext(); assert( ctx != nullptr );

		// Set the exception callback to receive information on errors in human readable form.
		int ret = ctx->SetExceptionCallback(asMETHOD(ScriptEngine, ExceptionCallback), this, asCALL_THISCALL); assert(ret >=0);

		exec->SetContext(ctx);

		return exec;
	}

	return nullptr;
}

/**
* \return The Angelscript engine pointer. No refCount increase/decrease happens/needs to happen.
*/
asIScriptEngine* const ScriptEngine::GetasIScriptEngine() {
	return this->engine;
}

/** 
* \param[in] fname The name of the file to load.
* \return Returns a value from SCRIPT_STATUS based on when failure or success happens.
*/
SCRIPT_STATUS::TYPE ScriptEngine::LoadScriptFile( const std::string &fname ) {
	int ret = 0;

	ret = this->builder.AddSectionFromFile(fname.c_str());
	if (ret < 0) {
		LOG(LOG_PRIORITY::SYSERR, "Script '" + fname + "' failed to load!  Does the file exist at the given path?");

		return SCRIPT_STATUS::FAILED_LOADING;
	}

	ret = this->builder.BuildModule();
	if (ret < 0) {
		LOG(LOG_PRIORITY::SYSERR, "Script '" + fname + "' failed to build. Possibly a syntax error in the script?");

		return SCRIPT_STATUS::FAILED_BUILDING;
	}

	return SCRIPT_STATUS::LOAD_OK;
}

/**
* \param[in] name Name of the config group.
* \return True if the config group can be registered to. False if another config group is being registered to.
*/
bool ScriptEngine::BeginConfigGroup( const std::string &name ) {
	return (this->engine->BeginConfigGroup(name.c_str()) > 0);
}

/**
* \return True if the config group registration was ended. Only returns false if the config group was never started.
*/
bool ScriptEngine::EndConfigGroup() {
	return (this->engine->EndConfigGroup() > 0);
}

/**
* \param[in] name Name of the config group.
* \return True if the config group was removed or false if it is in use or doesn't exist.
*/
bool ScriptEngine::RemoveConfigGroup( const std::string &name ) {
	return (this->engine->RemoveConfigGroup(name.c_str()) > 0);
}

/**
* \param[in] folder The relative folder name.
*/
void ScriptEngine::SetUserDataFolder( const std::string &folder ) {
	this->userDataFolder = folder;
	if ((this->userDataFolder.compare(this->userDataFolder.size() - 1, 1, "\\") != 0) && (this->userDataFolder.compare(this->userDataFolder.size() - 1, 1, "/") != 0)) {
		this->userDataFolder += "/";
	}
}

/**
* \param[in] filename The name of the log file.
*/
void ScriptEngine::SetLogFile( const std::string &filename ) {
	EventLogger::GetEventLogger()->SetLogFile(this->userDataFolder + filename);
}

/**
* \return True if the engine is running.
*/
bool ScriptEngine::IsRunning() {
	return this->isRunning;
}

void ScriptEngine::Shutdown() {
	this->isRunning = false;
}

/**
* \param[in] name The name of the script to load.
*/
void ScriptEngine::SetGameScript( const std::string &name ) {
	this->gameplayScript = name;
}

/**
* \return The gameplay script including the working directory specified in the script.
*/
const std::string ScriptEngine::GetGameScript() {
	return this->userDataFolder + this->gameplayScript;
}

/**
*/
void ScriptEngine::AbortExecution() {
	asIScriptContext* ctx = asGetActiveContext();
	
	if (ctx != nullptr) {
		ctx->Abort();
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Helper functions
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
std::string GetPreviousCallstackLine(const unsigned int& callstack_line_number) {
	asIScriptContext* ctx = asGetActiveContext();
	
	std::string result;
	
	if (ctx != nullptr && callstack_line_number < ctx->GetCallstackSize()) {
		asIScriptFunction* func;
		const char* script_section;
		int line, column;
		
		func = ctx->GetFunction(callstack_line_number);
		line = ctx->GetLineNumber(callstack_line_number, &column, &script_section);
		
		result = std::string(script_section) + "(" + boost::lexical_cast<std::string>(line) + "):" + std::string(func->GetDeclaration());
	}
	
	return result;
}
