#include "Map.hpp"
#include "LivingProfile.hpp"
#include "Door.hpp"
#include "Exit.hpp"
#include "Level.hpp"
#include "Lever.hpp"
#include "Chest.hpp"
#include "Living.hpp"
#include "ItemBag.hpp"
#include "SpikeTrap.hpp"
#include "PressPlate.hpp"
#include "Decoration.hpp"
#include "ShootTrap.hpp"
#include "../Gui/Gui.hpp"
#include "../Ai/AiMob.hpp"
#include "../Ai/AiPlayer.hpp"
#include "../Ai/AiMobMage.hpp"
#include "../Ai/AiBoss.hpp"
#include "../Core/Error.hpp"
#include "../base64/base64.h"
#include "../Render/Renderer.hpp"
#include "../Resource/TextureCache.hpp"
#include "../Collision/CollisionHandler.hpp"
#include <zlib.h>
#include <memory>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include "../Core/MinGWSucks.hpp"
#endif

void Map::loadFromFile(const std::string& path)
{
    using namespace rapidxml;

    std::string finalpath = "data/Maps/" + path;
    std::ifstream file(finalpath.c_str());
    std::string line;
    std::string content;

    if (!file.good())
    {
        ShowErrorBox("Something went wrong with " + finalpath + " file.");
    }
    else
    {
        while (!file.eof())
        {
            std::getline(file, line);
            content += line;
            content += "\n";
        }

        xml_document<> doc;
        doc.parse<0>(&content[0]);

        xml_node<>* map = doc.first_node("map");

        m_width = atoi(map->first_attribute("width")->value());
        m_height = atoi(map->first_attribute("height")->value());

        xml_node<>* tileset = map->first_node("tileset");
        loadTileset(tileset);
        loadLayers(map);
        collisions();
        makeChunks();
    }
}

void Map::loadTileset(rapidxml::xml_node<>* tileset)
{
    using namespace rapidxml;

    xml_node<>* image = tileset->first_node("image");
    std::string texturePath = image->first_attribute("source")->value();
    int width = atoi(image->first_attribute("width")->value());
    int height = atoi(image->first_attribute("height")->value());

    m_texture = TextureCache::Get().getTexture(texturePath);

    for (int i = 0; i < height/32; i++)
    {
        for (int j = 0; j < width/32; j++)
        {
            Tile tile;
            tile.sprite.setTexture(*m_texture);
            tile.rect = Recti(j * 32, i * 32, 32, 32);
            tile.sprite.setTextureRect({tile.rect.x,
                                        tile.rect.y,
                                        tile.rect.w,
                                        tile.rect.h});
            m_tileset.push_back(tile);
        }
    }
}

void Map::loadLayers(rapidxml::xml_node<>* map)
{
    using namespace rapidxml;

    xml_node<>* groundLayer = map->first_node("layer");
    loadLayer(groundLayer);

    xml_node<>* decorationLayer = map->first_node("layer")->next_sibling("layer");
    loadLayer(decorationLayer);

    xml_node<>* collisionLayer = decorationLayer->next_sibling("layer");
    loadLayer(collisionLayer);

    xml_node<>* objects = map->first_node("objectgroup");
    loadObjects(objects);
}

void Map::loadLayer(rapidxml::xml_node<>* layere)
{
    std::string rawData = layere->first_node("data")->value();

    for (int i = 0; i < rawData.size(); i++)
        if (rawData[i] == '=') rawData[i] = ' ';

    std::stringstream ass(rawData);
    std::string raw;
    ass >> raw;
    rawData = raw;

    std::string decodedData = base64_decode(rawData);

    uLongf finalSize = m_width * m_height * sizeof(int);
    std::vector<int> data(m_width * m_height);
    uncompress((Bytef*)&data[0], &finalSize, (const Bytef*)decodedData.c_str(), decodedData.size());

    TileLayer layer;
    layer.tiles.resize(m_width * m_height);

    for (int i = 0; i < data.size(); i++)
    {
        if (data[i] >= 1)
        {
            layer.tiles[i] = m_tileset[data[i]-1];
        }
        else
        {
            Tile tile;
            tile.empty = true;
            layer.tiles[i] = tile;
        }
    }

    for (int i = 0; i < m_height; i++)
    {
        for (int j = 0; j < m_height; j++)
        {
            layer.tiles[m_width * i + j].position = vec2i(j * 32, i * 32);
            layer.tiles[m_width * i + j].sprite.setPosition(j * 32, i * 32);
        }
    }

    m_layers.push_back(layer);
}

void Map::loadObjects(rapidxml::xml_node<>* objects)
{
    using namespace rapidxml;
    xml_node<>* object = objects->first_node("object");

    while (object)
    {
        std::string type = object->first_attribute("type")->value();

        if (type == "spawn")
        {
            std::string item0 = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            std::string item1 = object->first_node("properties")->first_node("property")->next_sibling()->first_attribute("value")->value();
            std::string item2 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item3 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item4 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string whom  = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            
            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            LivingProfile profile;
            profile.loadFromFile(whom + ".chr");
            // profile.loadFromFile(whom + ".chr");

            auto living = (Living*)m_level->addEntity(Entity::Ptr(new Living()));
            living->init(profile);
            living->setAi(Ai::Ptr(new AiMob()));
            living->setPosition(pos + vec2f(16,16));

            if (item0 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item0 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item1 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item1 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item2 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item2 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item3 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item3 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item4 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item4 + ".lua")));
                living->accessInv().addItem(item);
            }
        }
        else if (type == "spawn_mage")
        {
            std::string item0 = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            std::string item1 = object->first_node("properties")->first_node("property")->next_sibling()->first_attribute("value")->value();
            std::string item2 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item3 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item4 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string whom  = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            
            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            LivingProfile profile;
            profile.loadFromFile(whom + ".chr");
            // profile.loadFromFile(whom + ".chr");

            auto living = (Living*)m_level->addEntity(Entity::Ptr(new Living()));
            living->init(profile);
            living->setAi(Ai::Ptr(new AiMobMage()));
            living->setPosition(pos + vec2f(16,16));

            if (item0 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item0 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item1 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item1 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item2 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item2 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item3 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item3 + ".lua")));
                living->accessInv().addItem(item);
            }
            if (item4 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item4 + ".lua")));
                living->accessInv().addItem(item);
            }
        }
        else if (type == "spawn_boss")
        {
            std::string item0 = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            std::string whom  = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            
            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            LivingProfile profile;
            profile.loadFromFile(whom + ".chr");
            // profile.loadFromFile(whom + ".chr");

            auto living = (Living*)m_level->addEntity(Entity::Ptr(new Living()));
            living->init(profile);
            living->setAi(Ai::Ptr(new AiBoss()));
            living->setPosition(pos + vec2f(16,16));

            if (item0 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item0 + ".lua")));
                living->accessInv().addItem(item);
            }
        }
        else if (type == "door")
        {
            std::string name = object->first_attribute("name")->value();
            std::string item = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            auto door = (Door*)m_level->addEntity(Entity::Ptr(new Door()));
            door->setCode(name);
            door->setPosition(pos);
            door->setRequiredItem(item);
        }
        else if (type == "chest")
        {
            std::string name = object->first_attribute("name")->value();
            std::string item0 = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            std::string item1 = object->first_node("properties")->first_node("property")->next_sibling()->first_attribute("value")->value();
            std::string item2 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item3 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item4 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string key = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();

            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value()) - 13;
            pos.y = std::stof(object->first_attribute("y")->value());

            auto chest = (Chest*)m_level->addEntity(Entity::Ptr(new Chest()));
            chest->setCode(name);
            chest->setPosition(pos + vec2f(16,16));

            if (key != "-")
                chest->setRequiredItem(key);
            else
                chest->open();

            if (item0 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item0 + ".lua")));
                chest->accessInv().addItem(item);
            }
            if (item1 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item1 + ".lua")));
                chest->accessInv().addItem(item);
            }
            if (item2 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item2 + ".lua")));
                chest->accessInv().addItem(item);
            }
            if (item3 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item3 + ".lua")));
                chest->accessInv().addItem(item);
            }
            if (item4 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item4 + ".lua")));
                chest->accessInv().addItem(item);
            }
        }
        else if (type == "lever")
        {
            std::string name = object->first_attribute("name")->value();
            std::string whom = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            std::string type = object->first_node("properties")->first_node("property")->next_sibling()->first_attribute("value")->value();

            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            auto lever = (Lever*)m_level->addEntity(Entity::Ptr(new Lever()));
            lever->setCode(name);
            lever->setPosition(pos + vec2f(16,16));
            lever->setActivateFunc(
            [=]()
            {
                if (type == "door")
                {
                    auto door = m_level->getEntitiesByCode(whom);
                    for (int i = 0; i < door.size(); i++)
                    {
                        static_cast<Door*>(door[i])->open();
                    }
                }
                else if (type == "spike_trap")
                {
                    auto spikes = m_level->getEntitiesByCode(whom);
                    for (int i = 0; i < spikes.size(); i++)
                    {
                        static_cast<SpikeTrap*>(spikes[i])->disable();
                    }
                }
            });
        }
        else if (type == "shoot_trap")
        {
            std::string name = object->first_attribute("name")->value();
            std::string dire = object->first_node("properties")->first_node("property")->first_attribute("value")->value();

            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            Direction_t dir = Direction::Down;
            if (dire == "up")
                dir = Direction::Up;
            else if (dire == "down")
                dir = Direction::Down;
            else if (dire == "left")
                dir = Direction::Left;
            else 
                dir = Direction::Right;

            auto trap = (ShootTrap*)m_level->addEntity(Entity::Ptr(new ShootTrap(dir)));
            trap->setCode(name);
            trap->setPosition(pos);
        }
        else if (type == "spike_trap")
        {
            std::string name = object->first_attribute("name")->value();

            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            auto trap = (SpikeTrap*)m_level->addEntity(Entity::Ptr(new SpikeTrap()));
            trap->setCode(name);
            trap->setPosition(pos);
        }
        else if (type == "press_plate")
        {
            std::string name = object->first_attribute("name")->value();
            std::string whom = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            std::string type = object->first_node("properties")->first_node("property")->next_sibling()->first_attribute("value")->value();

            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            auto press = (PressPlate*)m_level->addEntity(Entity::Ptr(new PressPlate()));
            press->setCode(name);
            press->setPosition(pos);

            press->setActivateFunc(
            [=]()
            {
                if (type == "door")
                {
                    Door* door = (Door*)m_level->getEntityByCode(whom);

                    if (door)
                        door->open();
                }
                else if (type == "spike_trap")
                {
                    auto spikes = m_level->getEntitiesByCode(whom);
                    
                    for (std::size_t i = 0; i < spikes.size(); i++)
                    {
                        static_cast<SpikeTrap*>(spikes[i])->disable();
                    }
                }
            });

            press->setDeactivateFunc(
            [=]()
            {
                if (type == "spike_trap")
                {
                    auto spikes = m_level->getEntitiesByCode(whom);

                    for (std::size_t i = 0; i < spikes.size(); i++)
                    {
                        static_cast<SpikeTrap*>(spikes[i])->enable();
                    }
                }
            });
        }
        else if (type == "item_bag")
        {
            std::string name = object->first_attribute("name")->value();
            std::string item0 = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            std::string item1 = object->first_node("properties")->first_node("property")->next_sibling()->first_attribute("value")->value();
            std::string item2 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item3 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();
            std::string item4 = object->first_node("properties")->first_node("property")->next_sibling()->next_sibling()->next_sibling()->next_sibling()->first_attribute("value")->value();

            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            auto bag = (ItemBag*)m_level->addEntity(Entity::Ptr(new ItemBag()));
            bag->setCode(name);
            bag->setPosition(pos + vec2f(16,16));

            if (item0 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item0 + ".lua")));
                bag->accessInv().addItem(item);
            }
            if (item1 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item1 + ".lua")));
                bag->accessInv().addItem(item);
            }
            if (item2 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item2 + ".lua")));
                bag->accessInv().addItem(item);
            }
            if (item3 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item3 + ".lua")));
                bag->accessInv().addItem(item);
            }
            if (item4 != "-")
            {
                auto item = m_level->addItem(Item::Ptr(new Item(item4 + ".lua")));
                bag->accessInv().addItem(item);
            }
        }
        else if (type == "exit")
        {
            std::string name = object->first_attribute("name")->value();
            std::string next = object->first_node("properties")->first_node("property")->first_attribute("value")->value();

            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value()) + 32;

            auto exit = (Exit*)m_level->addEntity(Entity::Ptr(new Exit()));
            exit->setCode("exit");
            exit->setPosition(pos);
            exit->setNext(next);
        }
        else if (type == "decoration")
        {
            std::string name = object->first_attribute("name")->value();
            std::string visual = object->first_node("properties")->first_node("property")->first_attribute("value")->value();
            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            auto decor = (Decoration*)m_level->addDecoration(Decoration::Ptr(new Decoration()));
            decor->init(visual);
            decor->setPosition(pos);
        }
        else if (type == "start")
        {
            // std::string name = object->first_attribute("name")->value();
            vec2f pos;
            pos.x = std::stof(object->first_attribute("x")->value());
            pos.y = std::stof(object->first_attribute("y")->value());

            LivingProfile profile;
            profile.loadFromFile("pc_player.chr");

            auto player = (Living*)m_level->addEntity(Entity::Ptr(new Living()));
            player->init(profile);
            player->setCode("pc_player");
            player->setAi(Ai::Ptr(new AiPlayer()));
            player->setPosition(pos);

            GUI::Get().setTarget(player);
        }
        object = object->next_sibling("object");
    }
}

void Map::makeChunks()
{
    for(int l = 0; l < m_layers.size(); l++)
    {
        m_layers[l].chunks.resize(m_width/16);
        for(auto& i : m_layers[l].chunks)
            i.resize(m_width/16);

        for(uint h = 0; h < m_layers[l].chunks.size(); h++)
        {
            for(uint w = 0; w < m_layers[l].chunks[h].size(); w++)
            {
                for(uint i = h*16; i < h*16 + 16; i++)
                {
                    for(uint j = w*16; j < w*16 + 16; j++)
                    {
                        if(!m_layers[l].tiles[m_width * j + i].empty)
                        {
                            Tile& tile = m_layers[l].tiles[m_width * j + i];
                            m_layers[l].chunks[h][w].addTile(tile);
                        }
                    }
                }

                m_layers[l].chunks[h][w].setTexture(m_texture);
            }
        }
    }
}

void Map::collisions()
{
    for (int i = 0; i < m_layers[2].tiles.size(); i++)
    {
        auto& tile = m_layers[2].tiles[i];

        if (tile.rect.x == 96 and tile.rect.y == 128)
        {
            Box::Ptr box(new Box());
            box->rect = {tile.position.x, tile.position.y, tile.rect.w, tile.rect.h};
            box->material = CollMaterial::Regular;
            m_boxes.push_back(box);

            CollisionHandler::Get().addBody(m_boxes.back());
        }
    }
}

void Map::update()
{
    /*/
    for (int i = 0; i < m_tileset.size(); i++)
    {
        m_tileset[i].sprite.setPosition(i * 32, 0);
        Renderer::Get().submit(&m_tileset[i].sprite);
    }
    //*/

    for (std::size_t i = 0; i < m_layers[0].chunks.size(); i++)
    {
        for (std::size_t j = 0; j < m_layers[0].chunks[i].size(); j++)
        {
            m_layers[0].chunks[i][j].render();
        }
    }

    for (std::size_t i = 0; i < m_layers[1].chunks.size(); i++)
    {
        for (std::size_t j = 0; j < m_layers[1].chunks[i].size(); j++)
        {
            m_layers[1].chunks[i][j].render();
        }
    }

    if (false)
    for (std::size_t i = 0; i < m_layers[2].tiles.size(); i++)
    {
        Renderer::Get().submitBackground(&m_layers[2].tiles[i].sprite);
    }
}

void Map::setLevel(Level* level)
{
    m_level = level;
}
