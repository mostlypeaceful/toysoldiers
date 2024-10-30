sigimport "Gameplay/pickups/battery_recharge.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = BatteryRechargeLogic( )
	entity.Logic.Pickup = PICKUPS_BATTERY_1
}
