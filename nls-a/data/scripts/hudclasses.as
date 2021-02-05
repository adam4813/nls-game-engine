class SpriteData
{
	SpriteData() {
		this.texfilename = "";
		this.offsetX = 0;
		this.offsetY = 0;
		this.srcX = 0;
		this.srcY = 0;
		this.width = 0;
		this.height = 0;
	}
	SpriteData(string fname, float offX, float offY, uint x, uint y, uint w, uint h) {
		this.texfilename = fname;
		this.offsetX = offX;
		this.offsetY = offY;
		this.srcX = x;
		this.srcY = y;
		this.width = w;
		this.height = h;
	}
	string texfilename;
	float offsetX;
	float offsetY;
	uint srcX;
	uint srcY;
	uint width;
	uint height;
}

class Rect
{
	int x;
	int y;
	int width;
	int height;
}

class Button
{
	string type;
	string data;
	Rect hotspot;
}

class VerticalGauge
{
	VerticalGauge() {
	
	}
	bool Init()
	{
		this.count = gprops.GetPropertyUInt(this.countPropertyName);
		this.maxCount = gprops.GetPropertyUInt(this.maxCountPropertyName);
		Entity parent;
		EntList.AddEntity("HUD:" + name, parent);
		parent.properties.SetProperty("viewportWidth", uint(1024));
		parent.properties.SetProperty("viewportHeight", uint(768));
		//if (!(this.leftEndPiece is null))
		{
			{ // Left end piece
				Entity entity;
				
				EntList.AddEntity(name + "LeftEndPiece", entity); // Add to the entity manager - this makes no sense, it should be how entities are created!
				
				entity.SetParent(parent);
				entity.positionOffset.x = this.offsetX;
				entity.positionOffset.y = this.offsetY;
				entity.positionOffset.z = 0.4f;
				
				entity.SetRotation(0.0f, 0.0f, 0.0f);
				
				entity.scale = 1.0f;
				
				CreateEntityFactory fact;
				fact.SetEntity(entity);
				fact.SetName(name + "LeftEndPiece");
				
				fact.AddAttribute("sprite", "texfilename", this.leftEndPiece.texfilename);
				fact.AddAttribute("sprite", "offsetX", any(this.leftEndPiece.offsetX));
				fact.AddAttribute("sprite", "offsetY", any(this.leftEndPiece.offsetY));
				fact.AddAttribute("sprite", "srcX", any(this.leftEndPiece.srcX));
				fact.AddAttribute("sprite", "srcY", any(this.leftEndPiece.srcY));
				fact.AddAttribute("sprite", "width", any(this.leftEndPiece.width));
				fact.AddAttribute("sprite", "height", any(this.leftEndPiece.height));
				
				EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
				envelope.get().msgid = CREATE;
				fact.Finalize(envelope);
				Engine.PrioritySend(envelope);
			}
		}
		//if (!(this.middlePiece is null))
		{
			for (int i = 0; i < (this.count / this.scale); ++i) {
				{ // Middle Pieces
					Entity entity;
					
					EntList.AddEntity(name + "MiddlePiece" + i, entity); // Add to the entity manager - this makes no sense, it should be how entities are created!
					
					entity.SetParent(parent);
					entity.positionOffset.x =  i * this.middlePiece.width + this.leftEndPiece.width + this.offsetX;
					entity.positionOffset.y =  this.offsetY;
					entity.positionOffset.z =  0.4f;
					
					entity.SetRotation(0.0f, 0.0f, 0.0f);
					
					entity.scale = 1.0f;
					
					CreateEntityFactory fact;
					fact.SetEntity(entity);
					fact.SetName(name + "middlePiece" + i);
					
					fact.AddAttribute("sprite", "texfilename", this.middlePiece.texfilename);
					fact.AddAttribute("sprite", "offsetX", any(this.middlePiece.offsetX));
					fact.AddAttribute("sprite", "offsetY", any(this.middlePiece.offsetY));
					fact.AddAttribute("sprite", "srcX", any(this.middlePiece.srcX));
					fact.AddAttribute("sprite", "srcY", any(this.middlePiece.srcY));
					fact.AddAttribute("sprite", "width", any(this.middlePiece.width));
					fact.AddAttribute("sprite", "height", any(this.middlePiece.height));
					
					EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
					envelope.get().msgid = CREATE;
					fact.Finalize(envelope);
					Engine.PrioritySend(envelope);
				}
			}
		}
		//if (!(this.rightEndPiece is null))
		{
			{ // Right end piece
				Entity entity;
				
				EntList.AddEntity(name + "RightEndPiece", entity); // Add to the entity manager - this makes no sense, it should be how entities are created!
				
				entity.SetParent(parent);
				entity.positionOffset.x =  (this.count / this.scale) * this.middlePiece.width + this.leftEndPiece.width + this.middlePiece.offsetX + this.offsetX;
				entity.positionOffset.y =  this.offsetY;
				entity.positionOffset.z =  0.4f;
				
				entity.SetRotation(0.0f, 0.0f, 0.0f);
				
				entity.scale = 1.0f;
				
				CreateEntityFactory fact;
				fact.SetEntity(entity);
				fact.SetName(name + "RightEndPiece");
				
				fact.AddAttribute("sprite", "texfilename", this.rightEndPiece.texfilename);
				fact.AddAttribute("sprite", "offsetX", any(this.rightEndPiece.offsetX));
				fact.AddAttribute("sprite", "offsetY", any(this.rightEndPiece.offsetY));
				fact.AddAttribute("sprite", "srcX", any(this.rightEndPiece.srcX));
				fact.AddAttribute("sprite", "srcY", any(this.rightEndPiece.srcY));
				fact.AddAttribute("sprite", "width", any(this.rightEndPiece.width));
				fact.AddAttribute("sprite", "height", any(this.rightEndPiece.height));
				
				EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
				envelope.get().msgid = CREATE;
				fact.Finalize(envelope);
				Engine.PrioritySend(envelope);
			}
		}
		return true;
	}
	void Update()
	{
		uint newcount = gprops.GetPropertyUInt(this.countPropertyName);
		if (newcount > this.maxCount) {
			newcount = this.maxCount;
		}
		// Check if we gain some battery and add more middles
		if (newcount > this.count)
		{
			Log(INFO, "Battery increased. Added more sections.");
			Entity parent = EntList.FindEntity("HUD:" + name);
			for (int i = this.count; i < (newcount / this.scale); ++i)
			{
				{ // Middle Pieces
					Entity entity;
					
					EntList.AddEntity(name + "MiddlePiece" + i, entity); // Add to the entity manager - this makes no sense, it should be how entities are created!
					
					entity.SetParent(parent);
					entity.positionOffset.x =  i * this.middlePiece.width + this.leftEndPiece.width + this.offsetX;
					entity.positionOffset.y =  this.offsetY;
					entity.positionOffset.z =  0.4f;
					
					entity.SetRotation(0.0f, 0.0f, 0.0f);
					
					entity.scale = 1.0f;
					
					CreateEntityFactory fact;
					fact.SetEntity(entity);
					fact.SetName(name + "middlePiece" + i);
					
					fact.AddAttribute("sprite", "texfilename", this.middlePiece.texfilename);
					fact.AddAttribute("sprite", "offsetX", any(this.middlePiece.offsetX));
					fact.AddAttribute("sprite", "offsetY", any(this.middlePiece.offsetY));
					fact.AddAttribute("sprite", "srcX", any(this.middlePiece.srcX));
					fact.AddAttribute("sprite", "srcY", any(this.middlePiece.srcY));
					fact.AddAttribute("sprite", "width", any(this.middlePiece.width));
					fact.AddAttribute("sprite", "height", any(this.middlePiece.height));
					
					EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
					envelope.get().msgid = CREATE;
					fact.Finalize(envelope);
					Engine.PrioritySend(envelope);
				}
			}
			this.count = newcount;
		}
		else
		{
			this.count = newcount;
			if ((this.count % this.scale) == 0)
			{
				int i = this.count / this.scale;
				EntList.RemoveEntity(name + "MiddlePiece" + i);
			}
		}
	}
	
	void LevelChange() {
		this.count = gprops.GetPropertyUInt(this.countPropertyName);
		this.maxCount = gprops.GetPropertyUInt(this.maxCountPropertyName);
		if (this.count > 0) {
			Entity parent = EntList.FindEntity("HUD:" + name);
			parent.properties.SetProperty("viewportWidth", uint(1024));
			parent.properties.SetProperty("viewportHeight", uint(768));
		} else {
			Entity parent = EntList.FindEntity("HUD:" + name);
			parent.properties.SetProperty("viewportWidth", uint(0));
			parent.properties.SetProperty("viewportHeight", uint(0));
		}
	}
	
	uint count;
	uint maxCount;
	int scale; // How many counts make up 1 middle piece
	string countPropertyName;
	string maxCountPropertyName;
	string name;
	SpriteData leftEndPiece;
	SpriteData rightEndPiece;
	SpriteData middlePiece;
	float offsetX;
	float offsetY;
};

class MessageBox {
	MessageBox(string name, float x, float y, float depth, SpriteData data, uint rectOffX, uint rectOffY, uint rectW, uint rectH, uint fontH, uint fontW, uint fontWeight, bool fontItalic ) {
		this.name = name;
		
		Entity entity;
		EntList.AddEntity(this.name, entity); // Add to the entity manager - this makes no sense, it should be how entities are created!	
		
		entity.positionOffset.x =  x;
		entity.positionOffset.y =  y;
		entity.positionOffset.z =  depth;
		
		entity.SetRotation(0.0f, 0.0f, 0.0f);
		
		entity.scale = 1.0f;
		
		CreateEntityFactory fact;
		fact.SetEntity(entity);
		fact.SetName(this.name);
		
		fact.AddAttribute("sprite", "texfilename", data.texfilename);
		fact.AddAttribute("sprite", "offsetX", any(data.offsetX));
		fact.AddAttribute("sprite", "offsetY", any(data.offsetY));
		fact.AddAttribute("sprite", "srcX", any(data.srcX));
		fact.AddAttribute("sprite", "srcY", any(data.srcY));
		fact.AddAttribute("sprite", "width", any(data.width));
		fact.AddAttribute("sprite", "height", any(data.height));
		fact.AddAttribute("d3dfont", "height", any(fontH));
		fact.AddAttribute("d3dfont", "width", any(fontW));
		fact.AddAttribute("d3dfont", "weight", any(fontWeight));
		fact.AddAttribute("d3dfont", "italic", any(fontItalic));
		fact.AddAttribute("d3dfont", "rectX", any(rectOffX));
		fact.AddAttribute("d3dfont", "rectY", any(rectOffY));
		fact.AddAttribute("d3dfont", "rectW", any(rectW));
		fact.AddAttribute("d3dfont", "rectH", any(rectH));
		
		
		
		EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
		envelope.get().msgid = CREATE;
		fact.Finalize(envelope);
		Engine.PrioritySend(envelope);
	}
	void ShowMessage(string message, bool pause) {
		Entity entity = EntList.FindEntity(this.name);
		entity.positionOffset.z = 0.0f;
		if (pause) {
			gprops.SetProperty("PhysicsPause", true);
		}
		EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
		envelope.get().AddDataString(message);
		envelope.get().msgid = 1101;
		Engine.PrioritySend(envelope);
		Engine.RegisterMessageHandler("HideMessage", 9002);
		Engine.UnregisterMessageHandler("MouseEventHandler", 9003);
	}
	string name;
}

void HideMessage(EnvelopeSPTR envelope) {
	uint msg    = envelope.get().GetDataUInt(0);
	uint wparam = envelope.get().GetDataUInt(1);
	int64 lparam = envelope.get().GetDataLong(2);
	
	switch (msg) {
		case WM_KEYDOWN: {
			switch (wparam) {
				case VK_RETURN: {
					Entity entity = EntList.FindEntity("mainmb");
					entity.positionOffset.z = -1.0f;
					gprops.SetProperty("PhysicsPause", false);
					EnvelopeSPTR envelope = Engine.EnvelopeSPTRFactory();
					envelope.get().AddDataString("");
					envelope.get().msgid = 1101;
					Engine.PrioritySend(envelope);
					Engine.UnregisterMessageHandler("HideMessage", 9002);
					Engine.RegisterMessageHandler("MouseEventHandler", 9003);
				}
				break;
			}
		}
		break;
	}
}