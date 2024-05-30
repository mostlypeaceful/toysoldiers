#include "ToolsPch.hpp"
#include "tEditableWaypointEntity.hpp"
#include "tPathEntity.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSigmlConverter.hpp"

namespace Sig { namespace
{
	const f32 gSmallRadius = 0.1f;
	const f32 gBigRadius = 1.f;
}}

namespace Sig { namespace Sigml
{
	class tools_export tWaypointObject : public tWaypointObjectBase
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tWaypointObject, 0x7F1EFE95 );
	public:
		tWaypointObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	private:
		tPathEntityDef* fCreateEntityDefInternal( tSigmlConverter& sigmlConverter, tGrowableArray<tWaypointObjectBase*>& visited );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tWaypointObject& o )
	{
		s( "Connections", o.mConnectionGuids );
		s( "BackConnections", o.mBackConnectionGuids );
	}

	register_rtti_factory( tWaypointObject, false );

	tWaypointObject::tWaypointObject( )
	{
	}
	void tWaypointObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tWaypointObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	tEditableObject* tWaypointObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableWaypointEntity( container, this );
	}
}}


namespace Sig
{
	namespace
	{
		static const Math::tVec4f gInnerBallTint = Math::tVec4f( 0.1f, 0.3f, 1.0f, 0.75f );
		static const Math::tVec4f gShellSphereTint = Math::tVec4f( 1.0f, 0.25f, 1.0f, 0.5f );
	}


	tEditableWaypointEntity::tEditableWaypointEntity( tEditableObjectContainer& container )
		: tEditableWaypointBase( container )
	{
	}

	tEditableWaypointEntity::tEditableWaypointEntity( tEditableObjectContainer& container, const Sigml::tWaypointObjectBase* ao )
		: tEditableWaypointBase( container, ao )
	{ }

	tEditableWaypointEntity::~tEditableWaypointEntity( )
	{ }

	void tEditableWaypointEntity::fConnect( tEditableWaypointBase* to, b32 removeConnection )
	{
		tEditableWaypointBase::fConnect( to, removeConnection );
	}

	std::string tEditableWaypointEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Waypoint - " + name;
		return "Waypoint";
	}

	Sigml::tObjectPtr tEditableWaypointEntity::fSerialize( b32 clone ) const
	{
		Sigml::tWaypointObject* ao = new Sigml::tWaypointObject( );
		fSerializeBaseObject( ao, clone );

		if( !clone )
			fRecordConnections( ao );

		return Sigml::tObjectPtr( ao );
	}

}

