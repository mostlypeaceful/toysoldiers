
sigimport "Gui/Scripts/RadialMenus/vehicle_purchase.nut"

sigimport "Gameplay/Mobile/Vehicle/USA/tank_heavy_01.sigml"
sigimport "Gameplay/Mobile/Vehicle/USSR/tank_heavy_01.sigml"
sigimport "Gameplay/Mobile/Vehicle/USA/tank_medium_01.sigml"
sigimport "Gameplay/Mobile/Vehicle/USSR/tank_medium_01.sigml"
sigimport "Gameplay/Mobile/Vehicle/USA/car_01.sigml"
sigimport "Gameplay/Mobile/Vehicle/USSR/car_01.sigml"
sigimport "Gameplay/Mobile/Vehicle/USA/infantry_atv.sigml"
sigimport "Gameplay/Mobile/Vehicle/USSR/infantry_atv.sigml"
sigimport "Gameplay/Mobile/Airborne/USA/plane_fighter_01.sigml"
sigimport "Gameplay/Mobile/Airborne/USSR/plane_fighter_01.sigml"
sigimport "Gameplay/Mobile/Helicopter/USA/helo_attack_01.sigml"
sigimport "Gameplay/Mobile/Helicopter/USSR/helo_attack_01.sigml"
sigimport "gameplay/mobile/vehicle/usa/apc_ifv_01.sigml"
sigimport "gameplay/mobile/vehicle/ussr/apc_ifv_01.sigml"
sigimport "gameplay/mobile/helicopter/usa/helo_transport_01.sigml"
sigimport "gameplay/mobile/helicopter/ussr/helo_transport_01.sigml"
sigimport "gameplay/mobile/helicopter/usa/helo_transport_02.sigml"
sigimport "gameplay/mobile/helicopter/ussr/helo_transport_02.sigml"
sigimport "gameplay/mobile/airborne/usa/plane_bomber_01.sigml"
sigimport "gameplay/mobile/airborne/ussr/plane_bomber_01.sigml"
sigimport "gameplay/mobile/vehicle/usa/apc_mg_01.sigml"
sigimport "gameplay/mobile/vehicle/ussr/apc_mg_01.sigml"
sigimport "gameplay/mobile/airborne/usa/plane_transport_01.sigml"
sigimport "gameplay/mobile/airborne/ussr/plane_transport_01.sigml"

sigimport "Gui/Textures/WaveIcons/USA/barrage_spin_g.png"
sigimport "Gui/Textures/WaveIcons/USA/infantry_lvl1_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_tank01_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_tank02_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl1_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_tank01_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_tank02_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_tank01_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_tank02_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_car_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_car_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_atv_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_atv_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_fighter_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_fighter_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_heligunner_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_heligunner_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_apc04_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_apc04_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_apc05_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_apc05_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_apc01_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_apc01_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_bomber_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_bomber_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_helitransport_02_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_helitransport_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_gunship_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_helitransport_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_transport_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_transport_g.png"
sigimport "gui/textures/waveicons/ussr/vehicle_helitransport_02_g.png"


// Note:
////////////////////////////////////////////////////////////////////////////////
//
//	1. The Name field of the OffensiveWaveDesc should be the Unit Name of the 
//		icon you want to show up in the radial menu and the pending wave list.
//
////////////////////////////////////////////////////////////////////////////////

//Heavy tanks
offensiveWave_HeavyTankUsa <- OffensiveWaveDesc( )
offensiveWave_HeavyTankUsa.Name = "USA_TANK_HEAVY_01_Class"
offensiveWave_HeavyTankUsa.Desc = "HeavyTank_USA_WaveDesc"
offensiveWave_HeavyTankUsa.Cost = 2000
offensiveWave_HeavyTankUsa.Country = COUNTRY_USA
offensiveWave_HeavyTankUsa.WaveID = "OffensiveHeavyTanksUSA"

offensiveWave_HeavyTankUssr <- OffensiveWaveDesc( )
offensiveWave_HeavyTankUssr.Name = "USSR_TANK_HEAVY_01_Class"
offensiveWave_HeavyTankUssr.Desc = "HeavyTank_USSR_WaveDesc"
offensiveWave_HeavyTankUssr.Cost = 2000
offensiveWave_HeavyTankUssr.Country = COUNTRY_USSR
offensiveWave_HeavyTankUssr.WaveID = "OffensiveHeavyTanksUSSR"


//Heavy tanks Gen 2
offensiveWave_HeavyTank2Usa <- OffensiveWaveDesc( )
offensiveWave_HeavyTank2Usa.Name = "USA_TANK_HEAVY_01_Class"
offensiveWave_HeavyTank2Usa.Desc = "HeavyTank_USA_WaveDesc"
offensiveWave_HeavyTank2Usa.Cost = 2000
offensiveWave_HeavyTank2Usa.Country = COUNTRY_USA
offensiveWave_HeavyTank2Usa.WaveID = "OffensiveHeavyTanks2USA"

offensiveWave_HeavyTank2Ussr <- OffensiveWaveDesc( )
offensiveWave_HeavyTank2Ussr.Name = "USSR_TANK_HEAVY_01_Class"
offensiveWave_HeavyTank2Ussr.Desc = "HeavyTank_USSR_WaveDesc"
offensiveWave_HeavyTank2Ussr.Cost = 2000
offensiveWave_HeavyTank2Ussr.Country = COUNTRY_USSR
offensiveWave_HeavyTank2Ussr.WaveID = "OffensiveHeavyTanks2USSR"


//Medium tanks
offensiveWave_MediumTankUsa <- OffensiveWaveDesc( )
offensiveWave_MediumTankUsa.Name = "USA_TANK_MEDIUM_01_Class"
offensiveWave_MediumTankUsa.Desc = "MediumTank_USA_WaveDesc"
offensiveWave_MediumTankUsa.Cost = 500
offensiveWave_MediumTankUsa.Country = COUNTRY_USA
offensiveWave_MediumTankUsa.WaveID = "OffensiveMediumTanksUSA"

offensiveWave_MediumTankUssr <- OffensiveWaveDesc( )
offensiveWave_MediumTankUssr.Name = "USSR_TANK_MEDIUM_01_Class"
offensiveWave_MediumTankUssr.Desc = "MediumTank_USSR_WaveDesc"
offensiveWave_MediumTankUssr.Cost = 500
offensiveWave_MediumTankUssr.Country = COUNTRY_USSR
offensiveWave_MediumTankUssr.WaveID = "OffensiveMediumTanksUSSR"

//Medium tanks Gen 2
offensiveWave_MediumTank2Usa <- OffensiveWaveDesc( )
offensiveWave_MediumTank2Usa.Name = "USA_TANK_MEDIUM_01_Class"
offensiveWave_MediumTank2Usa.Desc = "MediumTank_USA_WaveDesc"
offensiveWave_MediumTank2Usa.Cost = 500
offensiveWave_MediumTank2Usa.Country = COUNTRY_USA
offensiveWave_MediumTank2Usa.WaveID = "OffensiveMediumTanks2USA"

offensiveWave_MediumTank2Ussr <- OffensiveWaveDesc( )
offensiveWave_MediumTank2Ussr.Name = "USSR_TANK_MEDIUM_01_Class"
offensiveWave_MediumTank2Ussr.Desc = "MediumTank_USSR_WaveDesc"
offensiveWave_MediumTank2Ussr.Cost = 500
offensiveWave_MediumTank2Ussr.Country = COUNTRY_USSR
offensiveWave_MediumTank2Ussr.WaveID = "OffensiveMediumTanks2USSR"

//Cars
offensiveWave_CarUsa <- OffensiveWaveDesc( )
offensiveWave_CarUsa.Name = "USA_CAR_01"
offensiveWave_CarUsa.Desc = "Car_USA_WaveDesc"
offensiveWave_CarUsa.Cost = 200
offensiveWave_CarUsa.Country = COUNTRY_USA
offensiveWave_CarUsa.WaveID = "OffensiveCarUSA"

offensiveWave_CarUssr <- OffensiveWaveDesc( )
offensiveWave_CarUssr.Name = "USSR_CAR_01"
offensiveWave_CarUssr.Desc = "Car_USSR_WaveDesc"
offensiveWave_CarUssr.Cost = 200
offensiveWave_CarUssr.Country = COUNTRY_USSR
offensiveWave_CarUssr.WaveID = "OffensiveCarUSSR"

//Apcs
offensiveWave_ApcUsa <- OffensiveWaveDesc( )
offensiveWave_ApcUsa.Name = "USA_APC_MG_01"
offensiveWave_ApcUsa.Desc = "Apc_USA_WaveDesc"
offensiveWave_ApcUsa.Cost = 500
offensiveWave_ApcUsa.Country = COUNTRY_USA
offensiveWave_ApcUsa.WaveID = "OffensiveApcUSA"

offensiveWave_ApcUssr <- OffensiveWaveDesc( )
offensiveWave_ApcUssr.Name = "USSR_APC_MG_01"
offensiveWave_ApcUssr.Desc = "Apc_USSR_WaveDesc"
offensiveWave_ApcUssr.Cost = 500
offensiveWave_ApcUssr.Country = COUNTRY_USSR
offensiveWave_ApcUssr.WaveID = "OffensiveApcUSSR"

//fighter planes
offensiveWave_FighterUsa <- OffensiveWaveDesc( )
offensiveWave_FighterUsa.Name = "USA_PLANE_FIGHTER_01_Class"
offensiveWave_FighterUsa.Desc = "Fighter_USA_WaveDesc"
offensiveWave_FighterUsa.Cost = 1800
offensiveWave_FighterUsa.Country = COUNTRY_USA
offensiveWave_FighterUsa.WaveID = "OffensiveFighterUSA"

offensiveWave_FighterUssr <- OffensiveWaveDesc( )
offensiveWave_FighterUssr.Name = "USSR_PLANE_FIGHTER_01_Class"
offensiveWave_FighterUssr.Desc = "Fighter_USSR_WaveDesc"
offensiveWave_FighterUssr.Cost = 1800
offensiveWave_FighterUssr.Country = COUNTRY_USSR
offensiveWave_FighterUssr.WaveID = "OffensiveFighterUSSR"

//Ifvs
offensiveWave_IfvUsa <- OffensiveWaveDesc( )
offensiveWave_IfvUsa.Name = "USA_APC_IFV_01_Class"
offensiveWave_IfvUsa.Desc = "Ifv_USA_WaveDesc"
offensiveWave_IfvUsa.Cost = 1500
offensiveWave_IfvUsa.Country = COUNTRY_USA
offensiveWave_IfvUsa.WaveID = "OffensiveIfvUSA"

offensiveWave_IfvUssr <- OffensiveWaveDesc( )
offensiveWave_IfvUssr.Name = "USSR_APC_IFV_01_Class"
offensiveWave_IfvUssr.Desc = "Ifv_USSR_WaveDesc"
offensiveWave_IfvUssr.Cost = 1500
offensiveWave_IfvUssr.Country = COUNTRY_USSR
offensiveWave_IfvUssr.WaveID = "OffensiveIfvUSSR"

//Atvs
offensiveWave_AtvUsa <- OffensiveWaveDesc( )
offensiveWave_AtvUsa.Name = "USA_INFANTRY_ATV_Class"
offensiveWave_AtvUsa.Desc = "Atv_USA_WaveDesc"
offensiveWave_AtvUsa.Cost = 300
offensiveWave_AtvUsa.Country = COUNTRY_USA
offensiveWave_AtvUsa.WaveID = "OffensiveAtvUSA"

offensiveWave_AtvUssr <- OffensiveWaveDesc( )
offensiveWave_AtvUssr.Name = "USSR_INFANTRY_ATV_Class"
offensiveWave_AtvUssr.Desc = "Atv_USSR_WaveDesc"
offensiveWave_AtvUssr.Cost = 300
offensiveWave_AtvUssr.Country = COUNTRY_USSR
offensiveWave_AtvUssr.WaveID = "OffensiveAtvUSSR"

//Atvs Gen 2
offensiveWave_Atv2Usa <- OffensiveWaveDesc( )
offensiveWave_Atv2Usa.Name = "USA_INFANTRY_ATV_Class"
offensiveWave_Atv2Usa.Desc = "Atv_USA_WaveDesc"
offensiveWave_Atv2Usa.Cost = 300
offensiveWave_Atv2Usa.Country = COUNTRY_USA
offensiveWave_Atv2Usa.WaveID = "OffensiveAtv2USA"

offensiveWave_Atv2Ussr <- OffensiveWaveDesc( )
offensiveWave_Atv2Ussr.Name = "USSR_INFANTRY_ATV_Class"
offensiveWave_Atv2Ussr.Desc = "Atv_USSR_WaveDesc"
offensiveWave_Atv2Ussr.Cost = 300
offensiveWave_Atv2Ussr.Country = COUNTRY_USSR
offensiveWave_Atv2Ussr.WaveID = "OffensiveAtv2USSR"

//Helicopters
offensiveWave_HeloUsa <- OffensiveWaveDesc( )
offensiveWave_HeloUsa.Name = "USA_HELO_ATTACK_01_Class"
offensiveWave_HeloUsa.Desc = "Helo_USA_WaveDesc"
offensiveWave_HeloUsa.Cost = 2000
offensiveWave_HeloUsa.Country = COUNTRY_USA
offensiveWave_HeloUsa.WaveID = "OffensiveHeloUSA"

offensiveWave_HeloUssr <- OffensiveWaveDesc( )
offensiveWave_HeloUssr.Name = "USSR_HELO_ATTACK_01_Class"
offensiveWave_HeloUssr.Desc = "Helo_USSR_WaveDesc"
offensiveWave_HeloUssr.Cost = 2000
offensiveWave_HeloUssr.Country = COUNTRY_USSR
offensiveWave_HeloUssr.WaveID = "OffensiveHeloUSSR"

//Transport Helicopters
offensiveWave_HeloTransportUsa <- OffensiveWaveDesc( )
offensiveWave_HeloTransportUsa.Name = "USA_HELO_TRANSPORT_01_Class"
offensiveWave_HeloTransportUsa.Desc = "Helo_Transport_USA_WaveDesc"
offensiveWave_HeloTransportUsa.Cost = 700
offensiveWave_HeloTransportUsa.Country = COUNTRY_USA
offensiveWave_HeloTransportUsa.WaveID = "OffensiveHeloTransportUSA"

offensiveWave_HeloTransportUssr <- OffensiveWaveDesc( )
offensiveWave_HeloTransportUssr.Name = "USSR_HELO_TRANSPORT_01_Class"
offensiveWave_HeloTransportUssr.Desc = "Helo_Transport_USSR_WaveDesc"
offensiveWave_HeloTransportUssr.Cost = 700
offensiveWave_HeloTransportUssr.Country = COUNTRY_USSR
offensiveWave_HeloTransportUssr.WaveID = "OffensiveHeloTransportUSSR"

//Gunship Helicopters
offensiveWave_HeloGunshipUsa <- OffensiveWaveDesc( )
offensiveWave_HeloGunshipUsa.Name = "USA_HELO_TRANSPORT_02_Class"
offensiveWave_HeloGunshipUsa.Desc = "Helo_Transport_USA_WaveDesc"
offensiveWave_HeloGunshipUsa.Cost = 800
offensiveWave_HeloGunshipUsa.Country = COUNTRY_USA
offensiveWave_HeloGunshipUsa.WaveID = "OffensiveHeloGunshipUSA"

offensiveWave_HeloGunshipUssr <- OffensiveWaveDesc( )
offensiveWave_HeloGunshipUssr.Name = "USSR_HELO_TRANSPORT_02_Class"
offensiveWave_HeloGunshipUssr.Desc = "Helo_Transport_USSR_WaveDesc"
offensiveWave_HeloGunshipUssr.Cost = 800
offensiveWave_HeloGunshipUssr.Country = COUNTRY_USSR
offensiveWave_HeloGunshipUssr.WaveID = "OffensiveHeloGunshipUSSR"

//Bombers
offensiveWave_BomberUsa <- OffensiveWaveDesc( )
offensiveWave_BomberUsa.Name = "USA_PLANE_BOMBER_01_Class"
offensiveWave_BomberUsa.Desc = "Bomber_USA_WaveDesc"
offensiveWave_BomberUsa.Cost = 3000
offensiveWave_BomberUsa.Country = COUNTRY_USA
offensiveWave_BomberUsa.WaveID = "OffensiveBomberUSA"

offensiveWave_BomberUssr <- OffensiveWaveDesc( )
offensiveWave_BomberUssr.Name = "USSR_PLANE_BOMBER_01_Class"
offensiveWave_BomberUssr.Desc = "Bomber_USSR_WaveDesc"
offensiveWave_BomberUssr.Cost = 3000
offensiveWave_BomberUssr.Country = COUNTRY_USSR
offensiveWave_BomberUssr.WaveID = "OffensiveBomberUSSR"

//Transports
offensiveWave_TransportUsa <- OffensiveWaveDesc( )
offensiveWave_TransportUsa.Name = "USA_PLANE_TRANSPORT_01_Class"
offensiveWave_TransportUsa.Desc = "Transport_USA_WaveDesc"
offensiveWave_TransportUsa.Cost = 500
offensiveWave_TransportUsa.Country = COUNTRY_USA
offensiveWave_TransportUsa.WaveID = "OffensiveTransportUSA"

offensiveWave_TransportUssr <- OffensiveWaveDesc( )
offensiveWave_TransportUssr.Name = "USSR_PLANE_TRANSPORT_01_Class"
offensiveWave_TransportUssr.Desc = "Transport_USSR_WaveDesc"
offensiveWave_TransportUssr.Cost = 500
offensiveWave_TransportUssr.Country = COUNTRY_USSR
offensiveWave_TransportUssr.WaveID = "OffensiveTransportUSSR"

//Barrages
offensiveWave_BarrageUSA <- OffensiveWaveDesc( )
offensiveWave_BarrageUSA.Name = "VSBarrageUSA_Name"
offensiveWave_BarrageUSA.Desc = "VSBarrageUSA_WaveDesc"
offensiveWave_BarrageUSA.Cost = 8000
offensiveWave_BarrageUSA.Country = COUNTRY_USA
offensiveWave_BarrageUSA.WaveID = "OffensiveBarrageUSA"

offensiveWave_BarrageUSSR <- OffensiveWaveDesc( )
offensiveWave_BarrageUSSR.Name = "VSBarrageUSSR_Name"
offensiveWave_BarrageUSSR.Desc = "VSBarrageUSSR_WaveDesc"
offensiveWave_BarrageUSSR.Cost = 8000
offensiveWave_BarrageUSSR.Country = COUNTRY_USSR
offensiveWave_BarrageUSSR.WaveID = "OffensiveBarrageUSSR"