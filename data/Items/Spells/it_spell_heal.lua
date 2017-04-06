
Spell_Heal = 4

Item = {
	code = "it_spell_heal",
	name = "Heal",
	image = "it_note.png",
	type = "spell",
	desc = "Put that there.",

	effect = function(user)
		if user:knowsSpell(Spell_Heal) == false then
			user:learnSpell(Spell_Heal, true)
			gui_read("Now that's some useful suff\nyour having here.");
		end
	end
}