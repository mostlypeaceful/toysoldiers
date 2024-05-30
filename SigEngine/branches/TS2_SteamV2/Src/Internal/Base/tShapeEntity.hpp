#ifndef __tShapeEntity__
#define __tShapeEntity__
#include "tEntityDef.hpp"
#include "tSpatialEntity.hpp"

namespace Sig
{
	class base_export tShapeEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tShapeEntityDef, 0x5714AB93 );
	public:
		enum tShapeType
		{
			cShapeTypeBox,
			cShapeTypeSphere,
			// last
			cShapeTypeCount
		};
	public:
		tEnum<tShapeType,u8> mShapeType;
		u8 pad2;
		u16 mStateMask;
		Math::tVec4f pad3;
		Math::tVec4f pad4;

	public:
		tShapeEntityDef( );
		tShapeEntityDef( tNoOpTag );
		~tShapeEntityDef( );
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};

	class base_export tShapeEntity : public tSpatialEntity
	{
		define_dynamic_cast( tShapeEntity, tSpatialEntity );
	public:
		static const u32 cSpatialSetIndex;
	public:
		tShapeEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld );
		tShapeEntity( const Math::tAabbf& objectSpaceBox, tShapeEntityDef::tShapeType shape );

		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }
		virtual void fPropagateSkeleton( tAnimatedSkeleton& skeleton );
		virtual b32 fIsHelper( ) const { return true; }
		virtual u32	fSpatialSetIndex( ) const { return cSpatialSetIndex; }
		b32 fContains( const Math::tVec3f& point ) const;
		b32 fIntersects( const tShapeEntity& otherShape ) const;
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32 fIntersects( const Math::tFrustumf& v ) const;
		virtual b32 fIntersects( const Math::tAabbf& v ) const;
		virtual b32 fIntersects( const Math::tObbf& v ) const;
		virtual b32 fIntersects( const Math::tSpheref& v ) const;
		virtual void fOnMoved( b32 recomputeParentRelative );
		tShapeEntityDef::tShapeType fShapeType( ) const { return mShapeType; }
		u32 fShapeTypeAsInt( ) const { return ( u32 )mShapeType; } // hack for script
		void fSetEnabled( b32 enabled ) { mEnabled = enabled; }
		b32 fEnabled( ) const { return mEnabled; }
		inline const Math::tObbf& fBox( ) const { return mBox; }
		inline const Math::tSpheref& fSphere( ) const { return mSphere; }
		Math::tObbf fParentRelativeBox( ) const;
		Math::tSpheref fParentRelativeSphere( ) const;
		Math::tVec3f fClosestPoint( const Math::tVec3f& point ) const;
		Math::tVec3f fRandomPoint( tRandom& rand ) const; //returns a point in world-space that lies within the shape
		if_devmenu( void fDebugRender( ) const );

		virtual void fStateMaskEnable( u32 index ) { fSetEnabled( fStateEnabled( index ) ); }

	private:
		Math::tObbf fComputeBox( ) const;
		Math::tSpheref fComputeSphere( ) const;

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		const tShapeEntityDef*		mEntityDef;
		tShapeEntityDef::tShapeType mShapeType;
		s16							mStateIndex;
		b16							mEnabled;
		Math::tObbf					mBox;
		Math::tSpheref				mSphere;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tShapeEntity );

}

#endif//__tShapeEntity__
