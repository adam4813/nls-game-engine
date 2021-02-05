#pragma once

#include <angelscript.h>
#include <angelscript/scriptbuilder/scriptbuilder.h>

class ScriptManager : public CScriptBuilder {
public:
	static asIScriptEngine* GetEngine();
	
	ScriptManager(void);
	~ScriptManager(void);
	int InitAngelScript(void);
	int CreateModule(std::string modname);
	int PrepareFunction(std::string funcname, std::string modname);
	int PrepareFunction(int funcID);
	int SetFunctionParamObject(int argIndex, void* param);
	int SetFunctionParamFloat(int argIndex, float param);
	int SetFunctionParamDouble(int argIndex, double param);
	int ExecuteFunction();

	void MessageCallback(const asSMessageInfo *msg);
	void ExceptionCallback(asIScriptContext* ctx);

	std::string* GetReturnString();
private:
	static asIScriptEngine *engine;
	static int instanceCount;
	
	asIScriptContext *ctx; // We only need one context at this time
};
