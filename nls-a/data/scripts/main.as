/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Constants
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "keyconstants.as"
#include "hudclasses.as"
#include "tmxclasses.as"
#include "maploader.as"

const float PI = 3.1415926535897932384626433832795f;
const float PI_BY_TWO = 1.5707963267948966192313216916398f;
const float TWO_PI = 6.283185307179586476925286766559f;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Globals
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
Entity gCamera;
VerticalGauge batterygauge;
VerticalGauge lifegauge;
MessageBox@ mainmb = null;
array<Button@> buttons;

string currentObstacle;
array<SpriteData@> obstacles;

array<string> mapentities;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Main
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void main(void) {
	Log(FLOW, "Loadup script started. Loading modules.");
	
	// Load the Graphics module
	EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
	envelope.get().msgid = LOADLIBRARY;
	envelope.get().AddDataInt(2); // Count of modules being requested for loading
	envelope.get().AddDataString("GraphicsDLL");
	envelope.get().AddDataString("SimplePhysics");
	
	Engine.PrioritySend(envelope);
	
	EnvelopeSPTR envelopestart = Engine.EnvelopeSPTRFactory();
	
	// Graphics
	envelopestart.get().msgid = 1002;
	Engine.PrioritySend(envelopestart);
	
	// Physics
	envelopestart.get().msgid = 3002;
	Engine.PrioritySend(envelopestart);

	Log(FLOW, "Modules started. Beginning to load game data.");
	
	{ // Camera. We need this for graphics to work
		Entity entity;
		
		EntList.AddEntity("Camera", entity); // Add to the entity manager - this makes no sense, it should be how entities are created!
		
		CreateEntityFactory fact;
		fact.SetEntity(entity);
		fact.SetName("Camera");
		
		fact.AddAttribute("camera", "name", "Camera");
		fact.AddAttribute("camera", "nearclip", @any(1.0f));
		fact.AddAttribute("camera", "farclip",  @any(10000.0f));
		
		EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
		envelope.get().msgid = CREATE;
		fact.Finalize(envelope);
		Engine.PrioritySend(envelope);
	}
	
	{ // Viewport - The map viewport that all map elements need to be parented to for proper scrolling
		Entity entity;
		
		EntList.AddEntity("viewport", entity); // Add to the entity manager - this makes no sense, it should be how entities are created!		
		
		entity.positionOffset.x =  0.0f;
		entity.positionOffset.y =  0.0f;
		entity.positionOffset.z =  0.0f;
		
		entity.SetRotation(0.0f, 0.0f, 0.0f);
		
		entity.scale = 1.0f;
	}
	MapLoad("../data/maps/title.tmx");
	currentObstacle = "";
	
	{ // Dialog Box
		//SpriteData sdata("/sprites/textbox.png", 0.0f, 0.0f, 0, 0, 800, 128);
		//MessageBox temp("mainmb", 0.0f, 640.0f, -1.0f, sdata, 5, 5, 790, 118, 22, 0, 400, false);
		//@mainmb = @temp;
	}
	
	
	{ // HUD
		{ // Battery or health gauge
			SpriteData left("/sprites/battery_gauge.png", 0.0f, 0.0f, 0, 0, 6, 32);
			SpriteData middle("/sprites/battery_gauge.png", -1.0f, 0.0f, 6, 0, 18, 32);
			SpriteData right("/sprites/battery_gauge.png", -1.0f, 0.0f, 24, 0, 8, 32);
			
			batterygauge.count = 5;
			batterygauge.scale = 1;
			batterygauge.name = "BatteryGuage";
			batterygauge.countPropertyName = "battery";
			batterygauge.maxCountPropertyName = "maxbattery";
			batterygauge.leftEndPiece = left;
			batterygauge.rightEndPiece = right;
			batterygauge.middlePiece = middle;
			batterygauge.offsetX = 800.0f;
			batterygauge.offsetY = 650.0f;
			
			batterygauge.Init();
			batterygauge.Update();
		}
		
		{ // Lives left guage
			SpriteData left("/sprites/battery_gauge.png", 0.0f, 0.0f, 0, 0, 6, 32);
			SpriteData middle("/sprites/Android_green.png", -1.0f, 0.0f, 0, 0, 32, 32);
			SpriteData right("/sprites/battery_gauge.png", -1.0f, 0.0f, 24, 0, 8, 32);
			
			lifegauge.count = 5;
			lifegauge.scale = 1;
			lifegauge.name = "LifeGuage";
			lifegauge.countPropertyName = "lives";
			lifegauge.maxCountPropertyName = "maxlives";
			lifegauge.leftEndPiece = left;
			lifegauge.middlePiece = middle;
			lifegauge.rightEndPiece = right;
			lifegauge.offsetX = 800.0f;
			lifegauge.offsetY = 32.0f;
			
			lifegauge.Init();
			lifegauge.Update();
		}
	}
	Log(FLOW, "Data loaded. Beginning demo.");
	
	Engine.RegisterMessageHandler("WinEventHandler", 9001);
	Engine.RegisterMessageHandler("WinEventHandler", 9002);
	Engine.RegisterMessageHandler("MouseEventHandler", 9003);
	Engine.RegisterMessageHandler("PointEnterActionHandler", 3101);
	Engine.RegisterMessageHandler("PointLeaveActionHandler", 3102);
}

void PointEnterActionHandler(EnvelopeSPTR envelope) {
	Log(FLOW, "Action at point: " + envelope.get().GetDataInt(0) + "," + envelope.get().GetDataInt(1) + ".");
	if (envelope.get().GetCount() == 5) { // Extra point handler data
		string type = envelope.get().GetDataString(2);
		string name = envelope.get().GetDataString(3);
		string data = envelope.get().GetDataString(4);
		Log(FLOW, "Point contains a handler for type: " + type + " named: " + name + ".");
		if (type == "enemy") {
			uint newbattery = gprops.GetPropertyUInt("battery") - 1;
			gprops.SetProperty("battery", newbattery);
			batterygauge.Update();
			EntList.RemoveEntity(name);
		}
		else if (type == "message") {
			Log(INFO, "Message: " + data);
			mainmb.ShowMessage(data, true);
		}
		else if (type == "battery") {
			Log(INFO, "Battery increased by: " + data);
			uint newbattery = gprops.GetPropertyUInt("battery") + stoi(data);
			Log(INFO, "Battery is now: " + newbattery);
			gprops.SetProperty("battery", newbattery);
			EntList.RemoveEntity(name);
		}
		else if (type == "levelend") {
			Log(INFO, "Message: " + data);
			mainmb.ShowMessage(data, true);
		}
	}
	
	uint newbattery = gprops.GetPropertyUInt("battery") - 1;
	if (newbattery == 0)
	{
		uint lives = gprops.GetPropertyUInt("lives");
		if ( lives > 0)
		{
			gprops.SetProperty("lives", lives - 1);
			lifegauge.Update();
			gprops.SetProperty("battery", gprops.GetPropertyUInt("maxbattery"));
			batterygauge.Update();
		}
		else
		{
			Log(FLOW, "You ran out of battery! Game Over!");
			
			// For now just quit
			EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
			envelope.get().msgid = QUIT;
			Engine.PrioritySend(envelope);
		}
	} else {
		gprops.SetProperty("battery", newbattery);
		batterygauge.Update();
	}
	
	EnvelopeSPTR retenv = Engine.EnvelopeSPTRFactory();
	retenv.get().msgid = 3100;
	retenv.get().AddDataUInt(0);
	Engine.Send(retenv);
}

void PointLeaveActionHandler(EnvelopeSPTR envelope) {
	EnvelopeSPTR actionenv = Engine.EnvelopeSPTRFactory();
	actionenv.get().msgid = 3104;
	Engine.DirectedSend(actionenv);

	if (envelope.get().GetCount() == 5) { // Extra point handler data
		string type = envelope.get().GetDataString(2);
		string name = envelope.get().GetDataString(3);
		string data = envelope.get().GetDataString(4);
		if (type == "levelend") {
			buttons.resize(0);
			gprops.SetProperty("PhysicsPause", true);
			gprops.SetProperty("GraphicsPause", true);
			MapUnload();
			MapLoad("../data/maps/title.tmx");
			gprops.SetProperty("GraphicsPause", false);
			gprops.SetProperty("PhysicsPause", false);
			batterygauge.LevelChange();
			lifegauge.LevelChange();
		}
	}
	
	EnvelopeSPTR retenv = Engine.EnvelopeSPTRFactory();
	retenv.get().msgid = 3100;
	retenv.get().AddDataUInt(1);
	Engine.Send(retenv);
}

void MouseEventHandler(EnvelopeSPTR envelope) {
	uint msg    = envelope.get().GetDataUInt(0);
	uint wparam = envelope.get().GetDataUInt(1);	
	switch (msg) {
		case WM_LBUTTONDOWN: {
			Entity view = EntList.FindEntity("viewport");
			
			float window_width = gprops.GetPropertyUInt("ClientWidth");
			float window_height = gprops.GetPropertyUInt("ClientHeight");
			int viewportWidth = view.properties.GetPropertyUInt("viewportWidth");
			int viewportHeight = view.properties.GetPropertyUInt("viewportHeight");
			
			int mouse_x = envelope.get().GetDataFloat(2) * window_width;
			int mouse_y = envelope.get().GetDataFloat(3) * window_width * -1;
			for (uint i = 0; i < buttons.length(); ++i) {
				Button button = buttons[i];
				if ((mouse_x > button.hotspot.x) and (mouse_x < (button.hotspot.x + button.hotspot.width))) {
					if ((mouse_y > button.hotspot.y) and (mouse_y < (button.hotspot.y + button.hotspot.height))) {
						if (button.type == "levelchange") {
							buttons.resize(0);
							gprops.SetProperty("GraphicsPause", true);
							gprops.SetProperty("PhysicsPause", true);
							MapUnload();
							MapLoad("../data/maps/" + button.data);
							gprops.SetProperty("GraphicsPause", false);
							gprops.SetProperty("PhysicsPause", false);
							batterygauge.LevelChange();
							lifegauge.LevelChange();
							
							return;
						}
						if (button.type == "quitgame") {
							Log(FLOW, "Pressed quit button.");
							EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
							envelope.get().msgid = QUIT;
							Engine.PrioritySend(envelope);
							return;
						}
						if (button.type == "obstacle") {
							currentObstacle = button.data;
							return;
						}
					}
				}
			}
			
			if ((mouse_x > viewportWidth) or (mouse_y > viewportHeight)) {
				Log(INFO, "Raw mouse click point: " + mouse_x + "," + mouse_y + " outside of viewport.");
				return;
			}
			
			int tile_x = mouse_x / 32;
			int tile_y = mouse_y / 32;
			
			int tileOffset_x = view.positionOffset.x / 32;
			int tileOffset_y = view.positionOffset.y / 32;
			
			int tileFinal_x = tile_x + tileOffset_x;
			int tileFinal_y = tile_y + tileOffset_y;
			
			// Make sure we have an obstacle
			if (currentObstacle != "") {
				// Send the message about the tile being clicked to astarmap
				EnvelopeSPTR clickenv = Engine.EnvelopeSPTRFactory();
				clickenv.get().msgid = 3103;
				clickenv.get().AddDataUInt(tileFinal_x);
				clickenv.get().AddDataUInt(tileFinal_y);
				clickenv.get().AddDataEntity(EntList.FindEntity(currentObstacle));
				EnvelopeSPTR retenv = Engine.DirectedSend(clickenv);
				if (retenv.get().GetDataBool(0) == true) {
					Log(FLOW, "Placed obstacle: " + currentObstacle + " at "  + tileFinal_x + "," + tileFinal_y + ".");
					Entity obs = EntList.FindEntity(currentObstacle);
					Entity parent = EntList.FindEntity("viewport");
					obs.SetParent(parent);
					currentObstacle = "";
				}
			}
		}
		break;
	}
}

void WinEventHandler(EnvelopeSPTR envelope) {
	uint msg    = envelope.get().GetDataUInt(0);
	uint wparam = envelope.get().GetDataUInt(1);
	int64 lparam = envelope.get().GetDataLong(2);
	
	switch (msg) {
		case WM_KEYDOWN: {
			switch (wparam) {
				case VK_ESCAPE: {
					Log(FLOW, "Escape pressed, quitting.");
					EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
					envelope.get().msgid = QUIT;
					Engine.PrioritySend(envelope);
				}
				break;
				case VK_LEFT: {
					Entity view = EntList.FindEntity("viewport");
					Entity map = EntList.FindEntity("map1");
					if (view.positionOffset.x > 0) {
						view.positionOffset.x -= 32.0f;
					}
				}
				break;
				case VK_RIGHT: {
					Entity view = EntList.FindEntity("viewport");
					Entity map = EntList.FindEntity("map1");
					if ((map.properties.GetPropertyUInt("mapWidthPixels") - view.positionOffset.x - view.properties.GetPropertyUInt("viewportWidth")) > 0) {
						view.positionOffset.x += 32.0f;
					}
				}
				break;
				case VK_UP: {
					Entity view = EntList.FindEntity("viewport");
					if (view.positionOffset.y > 0) {
						view.positionOffset.y -= 32.0f;
					}
				}
				break;
				case VK_DOWN: {
					Entity view = EntList.FindEntity("viewport");
					Entity map = EntList.FindEntity("map1");
					if ((map.properties.GetPropertyUInt("mapHeightPixels") - view.positionOffset.y - view.properties.GetPropertyUInt("viewportHeight")) > 0) {
						view.positionOffset.y += 32.0f;
					}
				}
				break;
				case VK_0: {
					Entity view = EntList.FindEntity("viewport");
					view.positionOffset.x = 0.0f;
					view.positionOffset.y = 0.0f;
				}
				break;
				case VK_1: {
					currentObstacle = "chipsprite2";
				}
				break;
			}
		}
		break;
		case WM_CLOSE: {
			Log(FLOW, "Window closing, quitting.");
			EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
			envelope.get().msgid = QUIT;
			Engine.PrioritySend(envelope);
		}
		break;
	}
}

void MapLoad(string fname) {
	MapLoader map1(fname);
	
	EnvelopeSPTR tilesets = Engine.EnvelopeSPTRFactory();
	TileSet@ tset = null;
	for (uint i = 0; i < map1.tileSets.length(); ++i) {
		@tset = map1.tileSets[i]; // Get the next tileset
		
		EnvelopeSPTR tsetEnv = Engine.EnvelopeSPTRFactory();
		
		// Store the data need to turn the tileids into useful tile data
		tsetEnv.get().AddDataString(tset.filename);		// Store the texture filename
		tsetEnv.get().AddDataUInt(tset.TilesAcross());	// Tiles across
		tsetEnv.get().AddDataUInt(tset.TilesDown());	// Tiles down
		tsetEnv.get().AddDataUInt(tset.firstGid);		// First tile id
		tsetEnv.get().AddDataUInt(tset.lastGid);		// Last tile id
		tilesets.get().AddDataEnvelopeSPTR(tsetEnv);
	}
	
	// Layers
	EnvelopeSPTR tilemaplayer = Engine.EnvelopeSPTRFactory();
	EnvelopeSPTR pathlayer = Engine.EnvelopeSPTRFactory();
	Layer@ layer = null;
	for (uint i = 0; i < map1.layers.length(); ++i) {
		@layer = map1.layers[i];
		
		EnvelopeSPTR layerEnv = Engine.EnvelopeSPTRFactory();
		
		TileID@ tile = null;
		for (uint t = 0; t < layer.tileIds.length(); ++t) {
			@tile = layer.tileIds[t];
			layerEnv.get().AddDataUInt(tile.row); // Store the row
			layerEnv.get().AddDataUInt(tile.column); // Store the column
			layerEnv.get().AddDataUInt(tile.id); // Store the tileid
		}
		
		if (layer.name == "Path") {
			pathlayer.get().AddDataEnvelopeSPTR(layerEnv); // Store the envelope with all the tileids
		}
		else {
			tilemaplayer.get().AddDataEnvelopeSPTR(layerEnv); // Store all layers
		}
	}
	
	EnvelopeSPTR gmoverObjectsEnv = Engine.EnvelopeSPTRFactory();
	EnvelopeSPTR menuObjectsEnv = Engine.EnvelopeSPTRFactory();
	EnvelopeSPTR objectsEnv = Engine.EnvelopeSPTRFactory();
	
	EventObject@ object = null;
	for (uint i = 0; i < map1.objects.length(); ++i) {
		@object = map1.objects[i];
		
		EnvelopeSPTR objectEnv = Engine.EnvelopeSPTRFactory();
		objectEnv.get().AddDataString(object.name);
		objectEnv.get().AddDataInt(object.x);
		objectEnv.get().AddDataInt(object.y);
		objectEnv.get().AddDataInt(object.width);
		objectEnv.get().AddDataInt(object.height);
		
		objectsEnv.get().AddDataEnvelopeSPTR(objectEnv);
		
		if ((object.type == "levelchange") or (object.type == "quitgame") or (object.type == "obstacle")) {
			Button button;
			button.type = object.type;
			button.data = object.data;
			button.hotspot.x = object.x;
			button.hotspot.y = object.y;
			button.hotspot.width = object.width;;
			button.hotspot.height = object.height;

			if (object.type == "obstacle") {
				Log(INFO, "Building obstacle: " + object.data);
				// Build our obstacle list
				obstacles.insertLast(@object.sprite);
				Entity parent;
				EntList.AddEntity("HUD:obstacleparent:" + object.data, parent);
				parent.properties.SetProperty("viewportWidth", uint(1024));
				parent.properties.SetProperty("viewportHeight", uint(768));
				Entity entity;
				EntList.AddEntity(object.data, entity); // Add to the entity manager - this makes no sense, it should be how entities are created!	
				
				entity.SetParent(parent);
				mapentities.insertLast(object.data);
				mapentities.insertLast("HUD:obstacleparent:" + object.data);
				entity.positionOffset.x =  float(object.x);
				entity.positionOffset.y =  float(object.y);
				entity.positionOffset.z =  0.0f;
				
				entity.SetRotation(0.0f, 0.0f, 0.0f);
				
				entity.scale = 1.0f;
				
				CreateEntityFactory fact;
				fact.SetEntity(entity);
				fact.SetName(object.data);
				
				fact.AddAttribute("sprite", "texfilename", "/maps/" + object.sprite.texfilename);
				fact.AddAttribute("sprite", "offsetX", any(object.sprite.offsetX));
				fact.AddAttribute("sprite", "offsetY", any(object.sprite.offsetY));
				fact.AddAttribute("sprite", "srcX", any(object.sprite.srcX));
				fact.AddAttribute("sprite", "srcY", any(object.sprite.srcY));
				fact.AddAttribute("sprite", "width", any(object.sprite.width));
				fact.AddAttribute("sprite", "height", any(object.sprite.height));
				
				
				EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
				envelope.get().msgid = CREATE;
				fact.Finalize(envelope);
				Engine.PrioritySend(envelope);
			}
			
			buttons.insertLast(button);
			continue;
		}
		else if (object.type != "") {
			EnvelopeSPTR gmoverobject = Engine.EnvelopeSPTRFactory();
			gmoverobject.get().AddDataString(object.type);	// Type
			gmoverobject.get().AddDataString(object.name);	// Name
			gmoverobject.get().AddDataString(object.data);	// Data
			gmoverobject.get().AddDataInt(object.x);		// x
			gmoverobject.get().AddDataInt(object.y);		// y
			gmoverObjectsEnv.get().AddDataEnvelopeSPTR(gmoverobject);
		}
		
		if (!(object.sprite is null)) {
			Entity entity;
			
			EntList.AddEntity(object.name, entity); // Add to the entity manager - this makes no sense, it should be how entities are created!
			mapentities.insertLast(object.name);
			Entity parent = EntList.FindEntity("viewport");
			
			entity.SetParent(parent);
			entity.positionOffset.x =  float(object.x);
			entity.positionOffset.y =  float(object.y);
			entity.positionOffset.z =  0.1f;
			
			entity.SetRotation(0.0f, 0.0f, 0.0f);
			
			entity.scale = 1.0f;
			
			CreateEntityFactory fact;
			fact.SetEntity(entity);
			fact.SetName(object.name);
			fact.AddAttribute("sprite", "texfilename", "/maps/" + object.sprite.texfilename);
			fact.AddAttribute("sprite", "offsetX", any(object.sprite.offsetX));
			fact.AddAttribute("sprite", "offsetY", any(object.sprite.offsetY));
			fact.AddAttribute("sprite", "srcX", any(object.sprite.srcX));
			fact.AddAttribute("sprite", "srcY", any(object.sprite.srcY));
			fact.AddAttribute("sprite", "width", any(object.sprite.width));
			fact.AddAttribute("sprite", "height", any(object.sprite.height));
			
			EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
			envelope.get().msgid = CREATE;
			fact.Finalize(envelope);
			Engine.PrioritySend(envelope);
		}
	}
	
	// Map	
	Entity mapEntity;
	
	EntList.AddEntity("map1", mapEntity); // Add to the entity manager - this makes no sense, it should be how entities are created!	
	mapentities.insertLast("map1");
	Entity mapParent = EntList.FindEntity("viewport");	
	mapEntity.SetParent(mapParent);
	
	mapEntity.positionOffset.x =  0.0f;
	mapEntity.positionOffset.y =  0.0f;
	mapEntity.positionOffset.z =  1.0f;
	
	mapEntity.SetRotation(0.0f, 0.0f, 0.0f);
	
	mapEntity.scale = 1.0f;
	
	CreateEntityFactory mapFact;
	mapFact.SetEntity(mapEntity);
	mapFact.SetName("map1");
	
	mapEntity.properties.SetProperty("mapWidthTiles", uint(map1.mapWidth));
	mapEntity.properties.SetProperty("mapHeightTiles", uint(map1.mapHeight));	
	mapEntity.properties.SetProperty("mapWidthPixels", uint(map1.mapWidth * map1.tileWidth));
	mapEntity.properties.SetProperty("mapHeightPixels", uint(map1.mapHeight * map1.tileHeight));			
	mapEntity.properties.SetProperty("tileWidth", uint(map1.tileWidth));
	mapEntity.properties.SetProperty("tileHeight", uint(map1.tileHeight));
	
	
	EnvelopeSPTR tilemapData = Engine.EnvelopeSPTRFactory();
	tilemapData.get().AddDataEnvelopeSPTR(tilesets); // Store the envelope with the tilesets
	tilemapData.get().AddDataEnvelopeSPTR(tilemaplayer); // Store the envelope with the visible tiles
	mapFact.AddAttribute("tiledmap", "mapdata", any(tilemapData));
	
	
	EnvelopeSPTR astarData = Engine.EnvelopeSPTRFactory();
	astarData.get().AddDataEnvelopeSPTR(pathlayer); // Store the envelope with the path tiles
	mapFact.AddAttribute("ASTARMap", "mapdata", any(astarData));
	mapFact.AddAttribute("ASTARMap", "objectdata", any(objectsEnv));
	
	mapFact.AddAttribute("pointline", "", "");
	
	{
		EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
		envelope.get().msgid = CREATE;
		mapFact.Finalize(envelope);
		Engine.PrioritySend(envelope);
	}

	if (gmoverObjectsEnv.get().GetCount() > 0)
	{
		// Mover	
		Entity gmoverEntity;
		
		EntList.AddEntity("gridmover", gmoverEntity); // Add to the entity manager - this makes no sense, it should be how entities are created!
		mapentities.insertLast("gridmover");	
		Entity gmoverParent = EntList.FindEntity("viewport");
		
		gmoverEntity.SetParent(gmoverParent);
		gmoverEntity.positionOffset.x =  0.0f;
		gmoverEntity.positionOffset.y =  0.0f;
		gmoverEntity.positionOffset.z =  0.0f;
		
		gmoverEntity.SetRotation(0.0f, 0.0f, 0.0f);
		
		gmoverEntity.scale = 1.0f;
		
		CreateEntityFactory gmoverFact;
		gmoverFact.SetEntity(gmoverEntity);
		gmoverFact.SetName("gridmover");
		
		gmoverFact.AddAttribute("GridMover", "scale", any(32));
		gmoverFact.AddAttribute("GridMover", "delay", any(0.5f));
		{ // Add the path pointlist
			Entity astar = EntList.FindEntity("map1");
			EnvelopeSPTR plistenv = Engine.EnvelopeSPTRFactory();
			plistenv.get().AddDataPointList(astar.properties.GetPropertyPointList("pointlist"));
			gmoverFact.AddAttribute("GridMover", "path", @any(plistenv));
		}
		gmoverFact.AddAttribute("GridMover", "pointenteractionhandler", @any(3101));
		gmoverFact.AddAttribute("GridMover", "pointleaveactionhandler", @any(3102));
		gmoverFact.AddAttribute("GridMover", "objectdata", any(gmoverObjectsEnv));
		
		gmoverFact.AddAttribute("sprite", "texfilename", "/sprites/StandingCI2.png");
		gmoverFact.AddAttribute("sprite", "offsetX", any(0.0f));
		gmoverFact.AddAttribute("sprite", "offsetY", any(-32.0f));
		gmoverFact.AddAttribute("sprite", "srcX", any(0));
		gmoverFact.AddAttribute("sprite", "srcY", any(0));
		gmoverFact.AddAttribute("sprite", "width", any(32));
		gmoverFact.AddAttribute("sprite", "height", any(64));
		
		{
			EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
			envelope.get().msgid = CREATE;
			gmoverFact.Finalize(envelope);
			Engine.PrioritySend(envelope);
		}
	}
	
	{ // Dialog Box
		SpriteData sdata("/sprites/textbox.png", 0.0f, 0.0f, 0, 0, 800, 128);
		MessageBox temp("mainmb", 0.0f, 640.0f, -1.0f, sdata, 5, 5, 790, 118, 22, 0, 400, false);
		@mainmb = @temp;
	}
}
void MapUnload() {
	for (uint i = 0; i < mapentities.length(); ++i) {
		EntList.RemoveEntity(mapentities[i]);
	}
	
	mapentities.resize(0);
}