
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	local volumeEntered = al.OnVolumeEnteredEvent
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_DecayTime", 20.0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_HFDamp", 1.0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_EROut", -30, 0.0 )
}
