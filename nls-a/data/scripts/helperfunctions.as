/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Helper Functions
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
Simplifies loading libraries
*/
void loadModule(string moduleName, string resultCallback) {
	uint successMessageID;
	
	if (moduleName == "GraphicsDLL") {
		successMessageID = 1001;
	}
	else {
		Log(2, "Unknown module name '" + moduleName + "' in loadModule(string, string)");
		return;
	}
	
	Envelope envelope;
	envelope.msgid = LOADLIBRARY;
	envelope.AddDataInt(1);
	envelope.AddDataString(moduleName);
	
	Engine.RegisterMessageHandler(resultCallback, successMessageID);
	
	Engine.Send(envelope);
}

/**
Simplifies starting libraries
*/
void startModule(string moduleName, string resultCallback) {
	uint startMessageID;
	uint successMessageID;
	
	if (moduleName == "GraphicsDLL") {
		startMessageID   = 1002;
		successMessageID = 1003;
	}
	else {
		Log(2, "Unknown module name '" + moduleName + "' in startModule(string, string)");
		return;
	}
	
	Envelope envelope;
	envelope.msgid = startMessageID;
	
	Engine.RegisterMessageHandler(resultCallback, successMessageID);
	
	Engine.Send(envelope);
}

/**
Vector magnitude calculation.
*TODO: put upstream in C++ to increase speed.
*/
float vecMag(Vector vector) {
	return sqrt(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
}
