#ifndef __j1MAP_H__
#define __j1MAP_H__

#include "PugiXml/src/pugixml.hpp"
#include "p2List.h"
#include "p2Point.h"
#include "j1Module.h"

// ----------------------------------------------------
struct MapLayer {

	p2SString	name;
	int			width;
	float		parallax;
	int			height;
	uint*		data;
	
	MapLayer() : data(NULL)
	{}

	~MapLayer()
	{
		RELEASE(data);
	}

	inline uint Get(int x, int y) const
	{
		return  x + y * width;
	}

};
// ----------------------------------------------------
struct TileSet
{
	
	SDL_Rect			GetTileRect(int id) const;

	p2SString			name;
	int					firstgid;
	int					margin;
	int					spacing;
	int					tile_width;
	int					tile_height;
	SDL_Texture*		texture;
	int					tex_width;
	int					tex_height;
	int					num_tiles_width;
	int					num_tiles_height;
	int					offset_x;
	int					offset_y;

};
// ----------------------------------------------------
struct ObjectGroup {
	
	p2SString		name = "NoNE";
	int				size = 0;
	SDL_Rect*		object;

};
// ----------------------------------------------------
enum MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};
// ----------------------------------------------------
struct MapData
{
	int						width;
	int						height;
	int						tile_width;
	int						tile_height;
	SDL_Color				background_color;
	MapTypes				type;
	
	p2List<TileSet*>		tilesets;
	p2List<MapLayer*>		layers;
	p2List<ObjectGroup*>	objectgroups;
	p2SString				musicFile;

};
// ----------------------------------------------------
class j1Map : public j1Module
{
public:

	// Constructor
	j1Map();

	// Destructor
	virtual ~j1Map();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called each loop iteration
	void Draw();

	// Called before quitting
	bool CleanUp();

	// Load new map
	bool Load(const char* path);

	iPoint MapToWorld(int x, int y) const;
	iPoint WorldToMap(int x, int y) const;

	

private:

	bool LoadMap(); //Load the map
	bool LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set); //Load all details of the tilset
	bool LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set); //Load the image of the tileset
	bool LoadLayer(pugi::xml_node& node, MapLayer* layer); //Load the layer
	bool LoadObject(pugi::xml_node& objectnode, ObjectGroup* objectgroup);	//Load all the Objects in map

	TileSet* GetTilesetFromTileId(int id) const;
	
	
public:

	MapData data;
	

private:

	pugi::xml_document	map_file;
	p2SString			folder;
	bool				map_loaded;
	
};
// ----------------------------------------------------
#endif // __j1MAP_H__
