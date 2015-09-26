#include "stdafx.h"

bool settings_t::load_xml(char * file)
{
	std::cout << "Loading XML from:" << file;
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file);

	if (!result)
	{
		std::cout << "\nError occured while parsing \""<< file <<"\"\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..." << (file + result.offset) << "]\n"; 
		std::cout << "Using default values instead\n\n";
		return false;
	}
	pugi::xml_node TheGame_node = doc.child("TheGame");
	for (pugi::xml_node game_node = TheGame_node.first_child(); game_node; game_node = game_node.next_sibling())
	{
		bool block_max_speed_set=false,
			block_acceleration_set = false,
			bullet_speed_set=false,
			gun_resolution_set = false;
		settings_t settings;

		for (pugi::xml_attribute_iterator attr = game_node.attributes_begin(); attr != game_node.attributes_end(); attr++)
		{
			string name = attr->name();
			if (name == "block_size")			settings.block_size = stoi(attr->value());
			else if (name == "map_size")			settings.map_size = stoi(attr->value());
			else if (name == "fps")					settings.fps = stoi(attr->value());
			else if (name == "block_max_speed")		settings.block_max_speed = stof(attr->value()), block_max_speed_set = true;
			else if (name == "block_acceleration")	settings.block_acceleration = stof(attr->value()), block_acceleration_set = true;
			else if (name == "bullet_size")			settings.bullet_size = stof(attr->value());
			else if (name == "gun_size")			settings.gun_size = stof(attr->value());
			else if (name == "footer_size")			settings.footer_size = stof(attr->value());
			else if (name == "bullet_speed")		settings.bullet_speed = stof(attr->value()), bullet_speed_set = true;
			else if (name == "respawn_time")		settings.respawn_time = stof(attr->value());
			else if (name == "gun_resolution")		settings.gun_resolution = stof(attr->value()), gun_resolution_set = true;
			else if (name == "max_ammo")			settings.max_ammo = stoi(attr->value());
			else if (name == "hp")					settings.hp = stoi(attr->value());
			else if (name == "port")				settings.port = stoi(attr->value());
			else
			{
				std::cout << "\nUnresolved atibute in node \"" << game_node.name() << "\" attribute: \"" << attr->name() << "\" value: \"" << attr->value() << "\"\n";
			}
		}
		if (!block_max_speed_set)
			settings.block_max_speed = (float)map_size / 70.f;
		if (!block_acceleration_set)
			settings.block_acceleration = (float)settings.block_max_speed / 70.f * 2.f;
		if (!bullet_speed_set)
			settings.bullet_speed = (float)1.5f * settings.block_max_speed;
		if (!gun_resolution_set)
			settings.gun_resolution = (float)settings.block_size / settings.map_size * 2.f;
		game->settings = settings;
	}
	std::cout << " ...success" << std::endl;
	return true;
}

