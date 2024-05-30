#ifndef __tEditableWaypointEntity__
#define __tEditableWaypointEntity__
#include "tEditableWaypointBase.hpp"

namespace Sig { namespace Sigml { class tWaypointObjectBase; }}

namespace Sig
{
	class tEditableWaypointEntity;
	define_smart_ptr( tools_export, tRefCounterPtr, tEditableWaypointEntity );

	/// 
	/// \brief A basic type of waypoint that can be linked arbitrarily.
	class tools_export tEditableWaypointEntity : public tEditableWaypointBase
	{
		define_dynamic_cast( tEditableWaypointEntity, tEditableWaypointBase );
	public:
		tEditableWaypointEntity( tEditableObjectContainer& container );
		tEditableWaypointEntity( tEditableObjectContainer& container, const Sigml::tWaypointObjectBase* ao );
		virtual ~tEditableWaypointEntity( );
		virtual void fConnect( tEditableWaypointBase* to, b32 removeConnection = false );
		virtual std::string fGetToolTip( ) const;
	protected:
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
	};

}

#endif//__tEditableWaypointEntity__
