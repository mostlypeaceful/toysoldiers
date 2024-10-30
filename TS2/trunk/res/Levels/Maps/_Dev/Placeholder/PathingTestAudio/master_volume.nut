
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	local volumeEntered = al.OnVolumeEnteredEvent
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_DecayTime", 10.0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_HFDamp", 0.5, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_EROut", -96, 0.0 )
	
	al.AlwaysInVolume = 1
}
