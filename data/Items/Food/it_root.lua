Item = {
	code = "it_root",
	name = "Edible Root",
	image = "it_meat.png",
	type = "food",
	desc = "This peed on root smells like magicka.",

	effect = function(user)
		user:restoreMana(10);
	end
}