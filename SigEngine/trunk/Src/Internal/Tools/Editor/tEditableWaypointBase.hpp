#ifndef __tEditableWaypointBase__
#define __tEditableWaypointBase__
#include "tEditableObject.hpp"
#include "Gfx/tWorldSpaceLines.hpp"
#include "tSigmlConverter.hpp"
#include "tPathEntity.hpp"

namespace Sig { namespace Sigml { class tWaypointObjectBase; class tPathDecalWaypointObject; }}

namespace Sig
{
	namespace Sigml
	{
		class tools_export tWaypointObjectBase : public tObject
		{
		public:
			tDynamicArray<u32> mConnectionGuids;
			tDynamicArray<u32> mBackConnectionGuids;
		public:
			tWaypointObjectBase( );
			virtual void fSerialize( tXmlSerializer& s );
			virtual void fSerialize( tXmlDeserializer& s );
			virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
			virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
		private:
			tPathEntityDef* fCreateEntityDefInternal( tSigmlConverter& sigmlConverter, tGrowableArray<tWaypointObjectBase*>& visited );
		};
	}

	class tEditableWaypointBase;
	define_smart_ptr( tools_export, tRefCounterPtr, tEditableWaypointBase );

	/// 
	/// \brief Base class for any type of series of waypoints.
	class tools_export tEditableWaypointBase : public tEditableObject
	{
		define_dynamic_cast( tEditableWaypointBase, tEditableObject );
	protected:
		Gfx::tRenderableEntityPtr mShellSphere;
		Gfx::tWorldSpaceLinesPtr mConnectionLines;
		Gfx::tRenderState mShellRenderState;
		tGrowableArray<tEditableWaypointBase*> mBackConnections;
		tGrowableArray<tEditableWaypointBasePtr> mConnections;
		tDynamicArray<u32> mSavedConnectionGuids;
	public:
		tEditableWaypointBase( tEditableObjectContainer& container );
		tEditableWaypointBase( tEditableObjectContainer& container, const Sigml::tWaypointObjectBase* ao );
		virtual ~tEditableWaypointBase( );
		const tGrowableArray<tEditableWaypointBase*>& fBackConnections( ) const { return mBackConnections; }
		const tGrowableArray<tEditableWaypointBasePtr>& fConnections( ) const { return mConnections; }
		void fRefreshLines( );
		virtual void fConnect( tEditableWaypointBase* to, b32 removeConnection = false );
		virtual void fDisconnect( b32 outwardOnly = false );
		void fAcquireEntirePath( tGrowableArray<tEditableWaypointBase*>& wayPoints );
		virtual std::string fGetToolTip( ) const = 0;
		void fFixUpGuidRefs( const tHashTable< u32, u32 >& conversionTable );
	private:
		void fCommonCtor( );
	protected:
		virtual void fAfterAllObjectsDeserialized( );
		virtual void fAddToWorld( );
		virtual void fRemoveFromWorld( );
		virtual void fOnMoved( b32 recomputeParentRelative );
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const = 0;
		///
		/// \brief Returns if the link was added properly.
		b32 fConnectEnsureSingleLinkForward( tEditableWaypointBase* to );
		void fRecordConnections( Sigml::tWaypointObjectBase* ao ) const;
		void fReverseConnectionsBackward( tEditableWaypointBase* backWaypoint );
		void fReverseConnectionsForward( tEditableWaypointBase* foreWaypoint );
	};
}

#endif//__tEditableWaypointBase__
