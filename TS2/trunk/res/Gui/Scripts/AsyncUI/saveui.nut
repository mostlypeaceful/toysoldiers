
sigimport "gui/scripts/controls/asyncstatus.nut"
sigimport "gui/textures/misc/loading_g.png"

sigexport function CanvasCreateSaveUI( loadScreen )
{
	return AsyncStatus( 
		"Saving", 
		"gui/textures/misc/loading_g.png",
		Math.Vec3.Construct( 540, 600, 0 ),
		AsyncStatusImageLocation.Left )
		
}