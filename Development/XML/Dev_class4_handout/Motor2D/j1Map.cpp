#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Map.h"
#include <math.h>

j1Map::j1Map() : j1Module(), map_loaded(false)
{
	name.create("map");
}

// Destructor
j1Map::~j1Map()
{}

// Called before render is available
bool j1Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;


	folder.create(config.child("folder").child_value());

	return ret;
}

void j1Map::Draw()
{
	if(map_loaded == false)
		return;

	// TODO 6: Iterate all tilesets and draw all their 
	// images in 0,0 (you should have only one tileset for now)
	DrawLayers();

}

// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// TODO 2: Make sure you clean up any memory allocated
	// from tilesets / map
	
	for (uint i = 0; i < tile_texture.count(); i++)
		App->tex->UnLoad(tile_texture[i]);

	for (uint i = 0; i < layers.count(); i++) //PRUEBA
		delete[] layers[i]->data;

	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load(const char* file_name)
{
	bool ret = true;
	p2SString tmp("%s%s", folder.GetString(), file_name);

	pugi::xml_parse_result result = map_file.load_file(tmp.GetString());

	if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}

	bool loadmap = true;

	if(ret == true)
	{
		// TODO 3: Create and call a private function to load and fill
		// all your map data
		loadmap = FillInfoMap(map_file.child("map"));
	}

	// TODO 4: Create and call a private function to load a tileset
	// remember to support more any number of tilesets!
	bool loadtiles = FillTileSet();
	bool loadlayers = FillLayer();

	if(ret == true)
	{
		// TODO 5: LOG all the data loaded
		// iterate all tilesets and LOG everything
		LogMapData(loadmap, loadtiles, loadlayers);

		for (uint i = 0; i < tilesets.count(); i++)
			tile_texture.add(App->tex->Load(tilesets[i]->name_file.GetString()));
		
		if (!(loadmap && loadtiles && loadlayers))
			map_loaded = false;
	}

	map_loaded = ret;

	return ret;
}

bool j1Map::FillInfoMap(pugi::xml_node& Map)
{
	bool ret = true;

	p2SString orientation = Map.attribute("orientation").as_string();

	if (orientation == "orthogonal")
		map.orientation = Map::MapOrientation::OR_ORTOGONAL;
	else if (orientation == "isometric")
		map.orientation = Map::MapOrientation::OR_ISOMETRIC;
	else if (orientation == "hexagonal")
		map.orientation = Map::MapOrientation::OR_UNDEFINED;
	else if (orientation == "undefined")
		map.orientation = Map::MapOrientation::OR_HEXAGONAL;

	p2SString renderer = Map.attribute("renderorder").as_string();

	if (renderer == "right-down")
		map.renderer = Map::MapRenderOrder::RE_RIGHT_DOWN;
	else if (renderer == "right-up")
		map.renderer = Map::MapRenderOrder::RE_RIGHT_UP;
	else if (renderer == "left-down")
		map.renderer = Map::MapRenderOrder::RE_LEFT_DOWN;
	else if (renderer == "left-up")
		map.renderer = Map::MapRenderOrder::RE_LEFT_UP;

	map.width = Map.attribute("width").as_uint();
	map.height = Map.attribute("height").as_uint();
	map.tilewidth = Map.attribute("tilewidth").as_uint();
	map.tileheight = Map.attribute("tileheight").as_uint();

	return ret;
}

bool j1Map::FillTileSet()
{
	bool ret = true;

	for (pugi::xml_node tileset = map_file.child("map").child("tileset"); tileset; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* tile = new TileSet;

		tile->name = tileset.attribute("name").as_string();

		tile->firstgid = tileset.attribute("firstgid").as_uint();

		tile->tileheight = tileset.attribute("tileheight").as_uint();

		tile->tilewidth = tileset.attribute("tilewidth").as_uint();

		tile->spacing = tileset.attribute("spacing").as_uint();

		tile->margin = tileset.attribute("margin").as_uint();

		tile->name_file = folder.GetString();
		tile->name_file += map_file.child("map").child("tileset").child("image").attribute("source").as_string();

		tile->width_file = map_file.child("map").child("tileset").child("image").attribute("width").as_uint();
		tile->height_file = map_file.child("map").child("tileset").child("image").attribute("height").as_uint();

		tilesets.add(tile);

	}

	return ret;

}

bool j1Map::FillLayer()
{
	bool ret = true;

	for (pugi::xml_node layer = map_file.child("map").child("layer"); layer; layer = layer.next_sibling("layer"))
	{
		Layer* ret_layer = new Layer;

		ret_layer->name = layer.attribute("name").as_string();
		ret_layer->width = layer.attribute("width").as_uint();
		ret_layer->height = layer.attribute("height").as_uint();

		uint data_total_nums = ret_layer->height*ret_layer->width;

		ret_layer->data = new uint[data_total_nums];

		memset(ret_layer->data, 0, data_total_nums);

		p2SString data_str = layer.child_value("data");
		char* str = (char*)data_str.GetString();
		char* temp = nullptr;
		uint count_data = 0;

		for (uint j = 0; j < ret_layer->height; j++)
		{
			str++;
			for (uint i = 0; i < ret_layer->width; i++)
			{
				if (i % 2)
				{
					strtok_s(temp, ",", &str);
					ret_layer->data[count_data++] = atoi(temp);
				}
				else
				{
					strtok_s(str, ",", &temp);
					ret_layer->data[count_data++] = atoi(str);
				}
			}
		}
		if (temp != NULL)
		{
			strtok_s(temp, "\n", &str);
			ret_layer->data[--count_data] = atoi(temp);
		}

		if (count_data != data_total_nums - 1)
			ret = false;


		layers.add(ret_layer);
	}

	return ret;
}

void j1Map::LogMapData(bool loadmap, bool loadtiles, bool loadlayers) const
{
	if (loadmap && loadmap && loadlayers)
	{
		LOG("Successfully parsed map XML file: %s", path_map.GetString());
	}
	else
		LOG("Error loading map XML file: %s", path_map.GetString());

	LOG("MapInfo----");

	LOG("width: %d", map.width);
	LOG("height: %d", map.height);
	LOG("tilewidth: %d", map.tilewidth);
	LOG("tileheight: %d", map.tileheight);

	LOG("TileSet----");

	for (uint i = 0; i < tilesets.count(); i++)
	{
		LOG("name: %s", tilesets[i]->name.GetString());
		LOG("firstgid: %d", tilesets[i]->firstgid);
		LOG("tile width: %d", tilesets[i]->tilewidth);
		LOG("tile height: %d", tilesets[i]->tileheight);
		LOG("spacing: %d", tilesets[i]->spacing);
		LOG("margin: %d", tilesets[i]->margin);
		LOG("---");
		LOG("image name: %s", tilesets[i]->name_file.GetString());
		LOG("image width: %d", tilesets[i]->width_file);
		LOG("image height: %d", tilesets[i]->height_file);
	}

	LOG("LayersInfo----");

	for (uint i = 0; i < layers.count(); i++)
	{
		LOG("name: %s", layers[i]->name.GetString());
		LOG("width: %d", layers[i]->width);
		LOG("height: %d", layers[i]->height);

		if (loadlayers == true)
			LOG("Layer data is successful load.");
		else
			LOG("Problems while program was loading layer data.");

		LOG("Printing all data:");
		for (uint j = 0; j < layers[i]->width*layers[i]->height; j++)
			LOG("data[%d]: %d", j, layers[i]->data[j]);
	}
}

void j1Map::DrawLayers(uint num) const
{
	uint id_tileset = 0;
	p2Point<uint> coordenates;

	for (uint y = 0; y < layers[num]->height; y++)
	{
		for (uint x = 0; x < layers[num]->width; x++)
		{
			SDL_Rect tile = tile_id(layers[num]->data[Get(x, y, num)], &id_tileset);

			coordenates = GetWorldPos(x, y);

			App->render->Blit(tile_texture[id_tileset], coordenates.x, coordenates.y, &tile);

			
		}
	}
}

inline uint j1Map::Get(uint x, uint y, uint num) const
{
	return (y*layers[num]->width + x);
}
p2Point<uint> j1Map::GetTilePos(uint wx, uint wy) const
{
	p2Point<uint> ret;

	ret.x = wx / map.tilewidth;

	ret.y = wy / map.tileheight;

	return ret;
}

inline p2Point<uint> j1Map::GetWorldPos(uint x, uint y) const
{

	p2Point<uint> ret;

	ret.x = x * map.tilewidth;
	ret.y = y * map.tileheight;

	return ret;
}



SDL_Rect j1Map::tile_id(uint id, uint* id_tileset) const
{
	uint ret_x = 0;
	uint ret_y = 0;

	uint t = 0;

	for (t; t < tilesets.count(); t++)
	{
		uint width = tilesets[t]->width_file - (tilesets[t]->margin * 2);
		uint height = tilesets[t]->height_file - (tilesets[t]->margin * 2);

		uint tiles_x = (width % tilesets[t]->tilewidth) + 1;
		uint tiles_y = (height % tilesets[t]->tileheight) + 1;

		if (id > tiles_x*tiles_y)
			continue;

		id_tileset = new uint{ t };

		ret_x = tilesets[t]->margin;
		ret_y = tilesets[t]->margin;

		uint count_tile = tilesets[t]->firstgid;

		for (uint y = 0; y < tiles_y; y++)
		{
			for (uint x = 0; x < tiles_x; x++, count_tile++)
			{

				if (count_tile != id)
					ret_x += tilesets[t]->tilewidth + tilesets[t]->spacing;
				else
					break;
			}

			if (count_tile != id)
			{
				ret_y += tilesets[t]->tileheight + tilesets[t]->spacing;
				ret_x = tilesets[t]->margin;
			}
			else
			{
				if (ret_x == tilesets[t]->width_file)
				{
					ret_x = tilesets[t]->margin;
					ret_y += tilesets[t]->tileheight + tilesets[t]->spacing;
				}
				break;
			}
		}
	}

	SDL_Rect ret = { ret_x, ret_y, tilesets[t - 1]->tilewidth, tilesets[t - 1]->tileheight };

	return ret;
}

