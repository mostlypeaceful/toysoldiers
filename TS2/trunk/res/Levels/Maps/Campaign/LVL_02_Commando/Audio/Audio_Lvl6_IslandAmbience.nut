sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
	
	local volumeEntered = al.OnVolumeEnteredEvent
	volumeEntered.Event = "Play_Amb_Lvl6_Island"
	volumeEntered.Source = AUDIO_SOURCE_MASTER
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_DecayTime", 4, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_HFDamp", 8.7, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_EROut", -87, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_ReverbOut", -92, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB1_Freq", 20, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB1_Gain", 0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB1_Q", 10, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB2_Freq", 20, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB2_Gain", 0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB2_Q", 10, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB3_Freq", 20, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB3_Gain", 0, 0.0 )
	volumeEntered.AddEvent( ).SetGlobalRTPC( "RoomVerb_FB3_Q", 10, 0.0 )
	
	local volumeExited = al.OnVolumeExitedEvent
	volumeExited.Event = "Stop_Amb_Lvl6_Island"
	volumeExited.Source = AUDIO_SOURCE_MASTER

}