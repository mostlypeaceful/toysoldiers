
// currently this is just referencing the default level load screen - at some point we'll probably want to make a custom one

sigimport "Gui/Scripts/LoadScreens/default_level_load.nut"

sigexport function CanvasCreateLoadScreen( loadScreen )
{
	return DefaultLevelLoadScreen( loadScreen )
}

