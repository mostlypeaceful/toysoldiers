
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	local volumeEntered = al.OnVolumeEnteredEvent
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_DecayTime", 0.5, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_HFDamp", 2.3, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_EROut", -40, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_ReverbOut", -86, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB1_Freq", 0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB1_Gain", 0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB1_Q", 10, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB2_Freq", 20, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB2_Gain", 0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB2_Q", 10, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB3_Freq", 20, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB3_Gain", 0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB3_Q", 10, 0.0 )


	al.AlwaysInVolume = 1
}
