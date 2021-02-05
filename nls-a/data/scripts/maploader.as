#include "tmxclasses.as"

class MapLoader {
	MapLoader(string fname) {
		this.fname = fname;
		NLTmxMap@ map = NLLoadTmxMap(this.fname);
		
		this.mapWidth = map.width;
		this.mapHeight = map.height;
		this.tileWidth = map.tileWidth;
		this.tileHeight = map.tileHeight;
		
		Entity view = EntList.FindEntity("viewport");
		if (map.properties.PropertyExists("viewportWidth")) {
			view.properties.SetProperty("viewportWidth", stou(map.properties.GetPropertyString("viewportWidth")));
		}
		else {
			view.properties.SetProperty("viewportWidth", uint(800));
		}
		if (map.properties.PropertyExists("viewportHeight")) {
			view.properties.SetProperty("viewportHeight", stou(map.properties.GetPropertyString("viewportHeight")));
		}
		else {
			view.properties.SetProperty("viewportHeight", uint(768));
		}
		
		if (map.properties.PropertyExists("battery")) {
			gprops.SetProperty("battery", stou(map.properties.GetPropertyString("battery")));
		}
		else {
			gprops.SetProperty("battery", uint(5));
		}
		if (map.properties.PropertyExists("maxbattery")) {
			gprops.SetProperty("maxbattery", stou(map.properties.GetPropertyString("maxbattery")));
		}
		else {
			gprops.SetProperty("maxbattery", uint(5));
		}		
		if (map.properties.PropertyExists("lives")) {
			gprops.SetProperty("lives", stou(map.properties.GetPropertyString("lives")));
		}
		else {
			gprops.SetProperty("lives", uint(5));
		}
		if (map.properties.PropertyExists("maxlives")) {
			gprops.SetProperty("maxlives", stou(map.properties.GetPropertyString("maxlives")));
		}
		else {
			gprops.SetProperty("maxlives", uint(5));
		}
		// Tilesets
		NLTmxMapTileset@ nltileset = map.FirstTileset(); // Get the first tileset
		while (!(nltileset is null)) {
			// Store the data need to turn the tileids into useful tile data			
			TileSet set(nltileset.filename, uint(nltileset.firstGid), uint(nltileset.tileWidth), uint(nltileset.tileHeight), uint(nltileset.filewidth), uint(nltileset.fileheight));
			this.tileSets.insertLast(set);
			
			@nltileset = map.NextTileset(); // Get the next tileset
		}
		Log(INFO, "Stored " + this.tileSets.length() + " tilesets.");
		// End Tilesets
		
		// Layers
		NLTmxMapLayer@ nllayer = map.FirstLayer(); // Get the first layer
		while (!(nllayer is null)) {
			Layer layer;
			layer.name = nllayer.name;
			for (uint r = 0; r < this.mapHeight; ++r) { // Row
				for (uint c = 0; c < this.mapWidth; ++c) { // Column
					int i = nllayer.GetData((r * this.mapWidth) + c);
					layer.tileIds.insertLast(TileID(r, c, i));
				}
			}
			this.layers.insertLast(layer);
			
			@nllayer = map.NextLayer(); // Get the next layer
		}
		Log(INFO, "Stored " + layers.length() + " layers.");
		// End Layers
		
		// Object Groups
		NLTmxMapObjectGroup@ nlobjectgroup = map.FirstObjectGroup(); // Get the first layer
		while (!(nlobjectgroup is null)) {
			// Objects
			NLTmxMapObject@ nlobject = nlobjectgroup.FirstObject(); // Get the first layer
			while (!(nlobject is null)) {
				EventObject obj(nlobject.name, nlobject.x, nlobject.y - map.tileHeight, uint(nlobject.width), uint(nlobject.height), uint(nlobject.gid));
				if (nlobject.properties.PropertyExists("type")) {
					obj.type = nlobject.properties.GetPropertyString("type");
				}
				if (nlobject.properties.PropertyExists("data")) {
					obj.data = nlobject.properties.GetPropertyString("data");
				}
				
				// Add a SpriteData if gid exists
				if (obj.gid != -1) {
					for (uint i = 0; i < this.tileSets.length(); ++i) {
						if (this.tileSets[i].ContainsGID(obj.gid)) {
							int offX = 0.0f;
							if (nlobject.properties.PropertyExists("offsetX")) {
								offX =  nlobject.properties.GetPropertyInt("offsetX");
							}
							int offY = 0.0f;
							if (nlobject.properties.PropertyExists("offsetY")) {
								offY =  nlobject.properties.GetPropertyInt("offsetY");
							}
							int tileOffset = obj.gid - this.tileSets[i].firstGid; // We need to normalize the offset
							int rowOffset = tileOffset / (this.tileSets[i].fileWidth / this.tileSets[i].tileWidth); // Next, we determine how many rows are in the id
							int colOffset = tileOffset - ((this.tileSets[i].fileWidth / this.tileSets[i].tileWidth) * rowOffset); // Finally, get the column by subtracting the rows.
							
							SpriteData sprite(this.tileSets[i].filename, offX, offY, uint(colOffset * this.tileSets[i].tileWidth), uint(rowOffset * this.tileSets[i].tileHeight), uint(this.tileSets[i].tileWidth), uint(this.tileSets[i].tileHeight));
							@obj.sprite = sprite;
						}
					}
				}
				
				objects.insertLast(obj);
				@nlobject = nlobjectgroup.NextObject(); // Get the next layer
			}
			// End Objects
			
			@nlobjectgroup = map.NextObjectGroup(); // Get the next layer
		}
		Log(INFO, "Stored " + objects.length() + " event objects.");
		// End Object Groups
	}
	
	string fname;
	array<TileSet> tileSets;
	array<Layer> layers;
	array<EventObject> objects;
	uint mapWidth;
	uint mapHeight;
	uint tileWidth;
	uint tileHeight;
}