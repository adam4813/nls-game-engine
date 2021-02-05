#include "hudclasses.as"

class TileSet {
	TileSet() {
		this.filename = "";
		this.firstGid = 0;
		this.tileWidth = 0;
		this.tileHeight = 0;
		this.fileWidth = 0;
		this.fileHeight = 0;
		this.lastGid = 0;
	}
	TileSet(string fname, uint fgid, uint twidth, uint theight, uint fwidth, uint fheight) {
		this.filename = fname;
		this.firstGid = fgid;
		this.tileWidth = twidth;
		this.tileHeight = theight;
		this.fileWidth = fwidth;
		this.fileHeight = fheight;
		this.lastGid = this.fileWidth / this.tileWidth * this.fileHeight / this.tileHeight + this.firstGid;
	}
	
	bool ContainsGID(uint gid) {
		if ((gid >= this.firstGid) and (gid < this.lastGid)) {
			return true;
		}
		
		return false;
	}
	
	uint TilesAcross() {
		return this.fileWidth / this.tileWidth;
	}
	uint TilesDown() {
		return this.fileHeight / this.tileHeight;
	}
	
	string filename;
	uint firstGid;
	uint lastGid;
	uint tileWidth;
	uint tileHeight;
	uint fileWidth;
	uint fileHeight;
}

class TileID {
	TileID() {
		this.row = 0;
		this.column = 0;
		this.id = 0;
	}
	TileID(uint r, uint c, uint id) {
		this.row = r;
		this.column = c;
		this.id = id;
	}
	uint row;
	uint column;
	uint id;
}

class Layer {
	string name;
	array< TileID > tileIds;
}

class EventObject {
	EventObject() {
		this.name = "";
		this.type = "";
		this.x = 0;
		this.y = 0;
		this.width = 0;
		this.height = 0;
		this.gid = 0;
	}
	EventObject(string name, int x, int y, uint w, uint h, uint gid = -1) {
		this.name = name;
		this.type = type;
		this.x = x;
		this.y = y;
		this.width = w;
		this.height = h;
		this.gid = gid;
	}
	
	
	string name;
	string type;
	string data;
	int x;
	int y;
	uint width;
	uint height;
	int gid;
	
	SpriteData@ sprite;
}