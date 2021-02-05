#include <cassert>
#include <boost/lexical_cast.hpp>

#include "../sharedbase/EventLogger.h"

#include "ScriptManager.h"

asIScriptEngine* ScriptManager::engine = nullptr;
int ScriptManager::instanceCount = 0;

asIScriptEngine* ScriptManager::GetEngine() {
	return ScriptManager::engine;
}


ScriptManager::ScriptManager( void ) : ctx(nullptr) {
	++ScriptManager::instanceCount;
}

ScriptManager::~ScriptManager( void ) {
	this->ctx->Release();
	if (--ScriptManager::instanceCount <= 0) {
		ScriptManager::engine->Release();
		ScriptManager::engine = nullptr;
	}
}


void ScriptManager::MessageCallback(const asSMessageInfo *msg) {
	std::string type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING) {
		type = "WARN";
	}
	else if (msg->type == asMSGTYPE_INFORMATION) {
		type = "INFO";
	}
	LOG(LOG_PRIORITY::RESTART, 
		type + 
		": " + boost::lexical_cast<std::string>(msg->section) + 
		" at line " + boost::lexical_cast<std::string>(msg->row) + 
		", column " + boost::lexical_cast<std::string>(msg->col) + 
		": " + msg->message
		);
	
	//assert(msg->type != asMSGTYPE_ERROR); // Angelscript tends to make a mess of memory when it errors out
}

void ScriptManager::ExceptionCallback(asIScriptContext* ctx) {
	asIScriptEngine* engine = ctx->GetEngine();
	
	int funcID = ctx->GetExceptionFunction();
	const asIScriptFunction* function = engine->GetFunctionById(funcID);
	
	int line, column;
	line = ctx->GetExceptionLineNumber(&column);
	
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

int ScriptManager::InitAngelScript( void ) {
	int ret = 0;
	
	// Create the script engine
	if (ScriptManager::engine == nullptr) {
		ScriptManager::engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		
		if (ScriptManager::engine == nullptr) {
			assert(false /* If the engine fails to init, this is a BIG problem in the source or logic flow. */);
			return -1;
		}

		// Set the message callback to receive information on errors in human readable form.
		ret = ScriptManager::engine->SetMessageCallback(asMETHOD(ScriptManager, MessageCallback), this, asCALL_THISCALL);
	}
	
	this->ctx = ScriptManager::engine->CreateContext(); // Create our context we will use for everything
	
	// Set the exception callback to receive information on errors in human readable form.
	ret = this->ctx->SetExceptionCallback(asMETHOD(ScriptManager, ExceptionCallback), this, asCALL_THISCALL);
	
	return ret;
}

int ScriptManager::PrepareFunction( std::string funcname, std::string modname ) {
	// Find the function that is to be called. 
	asIScriptModule *mod = ScriptManager::engine->GetModule(modname.c_str());
	int funcId = mod->GetFunctionIdByDecl(funcname.c_str());
	if( funcId >= 0 ) {
		return this->ctx->Prepare(funcId);
	}
	else {
		return -1;
	}
}

int ScriptManager::PrepareFunction( int funcID ) {
	// Find the function that is to be called. 
	if( funcID >= 0 ) {
		return this->ctx->Prepare(funcID);
	}
	else {
		return -1;
	}
}

int ScriptManager::SetFunctionParamObject(int argIndex, void* param) {
	return this->ctx->SetArgObject(argIndex, param);
}

int ScriptManager::SetFunctionParamFloat(int argIndex, float param) {
	return this->ctx->SetArgFloat(argIndex, param);
}

int ScriptManager::SetFunctionParamDouble(int argIndex, double param) {
	return this->ctx->SetArgDouble(argIndex, param);
}

int ScriptManager::ExecuteFunction() {
	return this->ctx->Execute();
}

std::string* ScriptManager::GetReturnString() {
	return (std::string*)this->ctx->GetReturnObject();
}

int ScriptManager::CreateModule( std::string modname ) {
	return this->StartNewModule(ScriptManager::engine, modname.c_str());
}
