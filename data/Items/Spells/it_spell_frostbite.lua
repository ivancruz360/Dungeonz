
Spell_Frostbite = 1

Item = {
	code = "it_spell_frostbite",
	name = "Frostbite",
	image = "it_note.png",
	type = "spell",
	desc = "It'll freeze your balls off!\nRequires 20pt magicka",

	effect = function(user)
		if user:knowsSpell(Spell_Frostbite) == false then
			user:learnSpell(Spell_Frostbite, true)
			gui_read("Frosty saves the day!");
		end
	end
}
