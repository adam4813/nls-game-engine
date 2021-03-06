/*
 Tests and example configuration.
*/

#include "ScriptUnitTests/TestFramework.as"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void main() {
	UnitTest::ExecuteTests(); // Run the tests.  This has nothing to do with config, feel free to not use in production code.
	Engine.SetUserDataFolder(OS.GetPath(SYSTEM_DIRS::EXECUTABLE));
	Engine.SetGameScript("main.as");
	// Configure the engine.
}
