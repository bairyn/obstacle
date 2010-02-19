models/players/human_bsuit_chrome/human_bsuit
{
	cull disable
	{
		map models/players/human_bsuit_chrome/shiny.jpg
		tcGen environment
	}
	{
		map models/players/human_bsuit_chrome/human_bsuit.tga
		rgbGen lightingDiffuse
		blendFunc blend
	}
	{
		map models/players/human_bsuit_chrome/glow.jpg
		blendFunc add
		rgbGen wave sin 0.4 .2 0 0.2
	}
}

