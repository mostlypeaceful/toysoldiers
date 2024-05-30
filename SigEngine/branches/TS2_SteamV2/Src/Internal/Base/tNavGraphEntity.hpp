//------------------------------------------------------------------------------
// \file tNavGraphEntity.hpp - 13 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tNavGraphEntity__
#define __tNavGraphEntity__
#include "tEntityDef.hpp"
#include "AI/tBuiltNavGraph.hpp"

namespace Sig
{
	///
	/// \class tNavGraphEntityDef
	/// \brief 
	class base_export tNavGraphEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tNavGraphEntityDef, 0x7BC32A63 );

	public:
		AI::tBuiltNavGraph* mNavGraph;

	public:
		tNavGraphEntityDef( );
		tNavGraphEntityDef( tNoOpTag );
		~tNavGraphEntityDef( );

		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};

	///
	/// \class tNavGraphEntity
	/// \brief 
	class base_export tNavGraphEntity : public tSpatialEntity
	{
		define_dynamic_cast( tNavGraphEntity, tSpatialEntity );

		AI::tNavGraphPtr mGraphWrapper;
	public:
		tNavGraphEntity( AI::tBuiltNavGraph* graph );

		AI::tNavGraphPtr fNavGraph( ) { return mGraphWrapper; } //yuck
	};

	define_smart_ptr( base_export, tRefCounterPtr, tNavGraphEntity );
}

#endif //__tNavGraphEntity__
