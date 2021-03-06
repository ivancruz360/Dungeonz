#ifndef ITEM_HPP
#define ITEM_HPP
#include "../Script/ScriptHandler.hpp"
#include "../Core/Error.hpp"
#include <string>
#include <memory>
#include <fstream>

struct Item
{
    using Ptr = std::shared_ptr<Item>;

    enum class Type
    {
        Plain = 0,
        Weapon,
        Armor,
        Spell,
        Potion,
        Food,
        Book,
        Bow,
        Misc
    };

    Type type;
    std::string code;
    std::string name;
    std::string image;
    std::string desc;

    std::unique_ptr<luabridge::LuaRef> effect = nullptr;
    std::unique_ptr<luabridge::LuaRef> equip = nullptr;
    std::unique_ptr<luabridge::LuaRef> takeoff = nullptr;

    Item() {}
    Item(const std::string& path)
    {
        loadFromFile(path);
    }

    void loadFromFile(const std::string& path)
    {
        std::string finalpath = "data/Items/" + path;
        std::ifstream file(finalpath.c_str());

        if (!file.good())
        {
            ShowErrorBox("Failed to load Item file \"" + finalpath + "\"");
        }
        else
        {

        //*/
            using namespace luabridge;

            lua_State* lua = luaL_newstate();
            luaL_openlibs(lua);
            ScriptHandler::Get().loadLibs(lua);
            luaL_dofile(lua, finalpath.c_str());
            lua_pcall(lua, 0,0,0);

            LuaRef ref_theItem = getGlobal(lua, "Item");

            code = ref_theItem["code"].cast<std::string>();
            name = ref_theItem["name"].cast<std::string>();
            image = ref_theItem["image"].cast<std::string>();
            desc = ref_theItem["desc"].cast<std::string>();

            std::string str_type = ref_theItem["type"];

            if (str_type == "weapon")
            {
                type = Type::Weapon;

                equip = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["equip"]));
                takeoff = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["takeoff"]));
            }
            else if (str_type == "armor")
            {
                type = Type::Armor;
                
                equip = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["equip"]));
                takeoff = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["takeoff"]));
            }
            else if (str_type == "food")
            {
                type = Type::Food;
                
                effect = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["effect"]));
            }
            else if (str_type == "book")
            {
                type = Type::Book;
                
                effect = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["effect"]));
            }
            else if (str_type == "spell")
            {
                type = Type::Spell;

                effect = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["effect"]));
            }
            else if (str_type == "misc")
            {
                type = Type::Misc;
            }
            else if (str_type == "bow")
            {
                type = Type::Bow;

                equip = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["equip"]));
                takeoff = std::unique_ptr<LuaRef>(new LuaRef(ref_theItem["takeoff"]));
            }

        //*/

        // lua_close(lua);
        }
        file.close();
    }
};

#endif