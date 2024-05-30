#ifndef __tPathEntity__
#define __tPathEntity__
#include "tEntityDef.hpp"

namespace Sig
{
	class tPathEntity;

	class base_export tPathEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPathEntityDef, 0x935DC54F );
	public:
		typedef tDynamicArray< u32 > tConnectionList;
	public:
		u32 mGuid;
		tConnectionList mNextPoints;
	public:
		tPathEntityDef( );
		tPathEntityDef( tNoOpTag );
		~tPathEntityDef( );
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};

	typedef tRefCounterPtr< tPathEntity > tPathEntityPtr;

	class base_export tPathEntity : public tEntity
	{
		define_dynamic_cast( tPathEntity, tEntity );
	public:
		typedef tGrowableArray<tPathEntity*> tConnectionsList;
	private:
		const tPathEntityDef*	mEntityDef;
		tConnectionsList		mPrevPoints;
		tConnectionsList		mNextPoints;

	public:
		tPathEntity( ) : mEntityDef( NULL ) { }
		tPathEntity( const tPathEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld );
		const tPathEntityDef* fEntityDef( ) const { return mEntityDef; }
		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }
		virtual void fAfterSiblingsHaveBeenCreated( );
		virtual void fPropagateSkeleton( tAnimatedSkeleton& skeleton );
		virtual b32 fIsHelper( ) const { return true; }

		u32 fPrevPointCount( ) const { return mPrevPoints.fCount( ); }
		tPathEntity* fPrevPoint( u32 index ) { return mPrevPoints[ index ]; }
		const tConnectionsList& fPrevPathPoints( ) const { return mPrevPoints; }

		u32 fNextPointCount( ) const { return mNextPoints.fCount( ); }
		tPathEntity* fNextPoint( u32 index ) const { return mNextPoints[ index ]; }
		const tConnectionsList& fNextPathPoints( ) const { return mNextPoints; }

		// will take the first child and interpolate up to distance along the path
		// return false if you've reached the end. out will then contain the end position
		b32 fTraversePath( f32 distance, Math::tVec3f& out ) const;

		static void fExportScriptInterface( tScriptVm& vm );
	};


}

#endif//__tPathEntity__
