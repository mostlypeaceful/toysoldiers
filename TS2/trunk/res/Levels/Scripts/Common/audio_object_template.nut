
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	// * all of the events have the same properties
	//   all properties are optional
	local onSpawn = al.OnSpawnEvent
	onSpawn.Event = "PLAY_AMBIENT_OBJ1"
	//onSpawn.SwitchToSet = "DAMAGE_LEVEL"
	//onSpawn.SwitchValue = "TOTALLY_DAMAGED"
	//onSpawn.StateToSet = "GAME_STATE"
	//onSpawn.StateValue = "LVL1_IN_TOWN"
	//onSpawn.RTPCToSet = "HEALTH"
	//onSpawn.RTPCValue = 25.0
	//onSpawn.RTPCChangeTotalTime = 2.0 // change over two seconds
	//onSpawn.Source = AUDIO_SOURCE_LOCAL //or AUDIO_SOURCE_MASTER, or AUDIO_SOURCE_GLOBAL
	//onSpawn.SnapValues = 0 //set to 1 to apply RTPC changes instantly
	
	// * convienience function to initialize the rtpc event the same as above.
	//   if time is negative or zero, Snap valus will be set to 1
	//onSpawn.SetRTPC( "HEALTH", 25.0, 2.0 )  //syntax, (rtpcname, value, time over which to change)
	//onSpawn.SetMasterRTPC( "HEALTH", 25.0, 2.0 ) // same as above but sets Source to AUDIO_SOURCE_MASTER	
	//onSpawn.SetGlobalRTPC( "HEALTH", 25.0, 2.0 ) // same as above but sets Source to AUDIO_SOURCE_GLOBAL	
	//onSpawn.SetState( "GAME_STATE", "LVL1_IN_TOWN" )  //syntax, (statename, value)
	
	// * you can daisy chain, setting multiple parameters in a particular order like so.
	//local subEvent = onSpawn.AddEvent( )
	//subEvent.Event = "Additiona_event"
	
	// * if you only need to set one parameter on the event, there's no need to store it in a local var
	//onSpawn.AddEvent( ).Event = "Another_aditional_event"
	
	// * Lots of RTPC example, these could be added to any event, such as volumeEntered 
	//   and their times could be set to positive values to change over time
	//onSpawn.AddEvent( ).SetGlobalRTPC( "RoomVerb_DecayTime23", 10.0, 0.0 )
	//onSpawn.AddEvent( ).SetGlobalRTPC( "RoomVerb_HFDamp23", 0.5, 0.0 )
	//onSpawn.AddEvent( ).SetGlobalRTPC( "RoomVerb_EROut23", -96, 0.0 )
			
	local onDelete = al.OnDeleteEvent
	onDelete.Event = "STOP_AMBIENT_OBJ1"
	// all properties optionaly available for onDelete event
	
	local volumeEntered = al.OnVolumeEnteredEvent
	volumeEntered.SetGlobalRTPC( "WIND_INTENSITY", 100.0, 0.0 )
	// all properties optionaly available for volumeEntered event
	
	local volumeExited = al.OnVolumeExitedEvent
	volumeExited.SetGlobalRTPC( "WIND_INTENSITY", 1.0, 0.0 )
	
	// if the this is the master volume you can set this flag:
	//al.AlwaysInVolume = 1
	
	// State stuff
	local state0 = al.StateEvents( 0 )
	state0.OnEnter.Event = "play_sound"
	state0.OnExit.Event = "stop_sound"

}
