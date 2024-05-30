#ifndef __tSigmlReferenceEntity__
#define __tSigmlReferenceEntity__
#include "tEditableObject.hpp"
#include "Sigml.hpp"
#include "tSgFileRefEntity.hpp"
#include "Gfx\tWorldSpaceLines.hpp"

namespace Sig
{
	class tools_export tEditableSgFileRefEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableSgFileRefEntity, tEditableObject );
	public:
		// The resource ptr might be null so now we need a resource id to fall back on in such a case.
		tEditableSgFileRefEntity( tEditableObjectContainer& container, const tResourceId& resourceId, const tResourcePtr& sigbResource );
		tEditableSgFileRefEntity( tEditableObjectContainer& container, const Sigml::tSigmlReferenceObject& sigmlObject );
		~tEditableSgFileRefEntity( );
		tFilePathPtr fResourcePath( ) const;
		const tResourcePtr & fResource( ) const;

		b32 fIsSigml( ) const;
		b32 fIsEditable( ) const;
		b32 fIsRedBox( ) const;
		void fResetReference( const tFilePathPtr& newRefPath );
		void fRefreshDependents( const tResourcePtr& reloadedResource, b32 unloadOnly );
		std::string fGetToolTip( ) const;
		std::string fGetAssetPath( ) const;

		const tSgFileRefEntityPtr& fEntity( ) const { return mSgFileRefEntity; }

		virtual void fOnDeviceLost( Gfx::tDevice* device );
		virtual void fOnDeviceReset( Gfx::tDevice* device );

	protected:
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
		s32 fGetDisplayIndex( ) const;
		void fUpdateDisplayedStates( u32 displayIndex );
		virtual std::string fGetSoftName( ) const;

	protected:
		tResourceId				mSgId;				
		tResourcePtr			mSgResource;
		tSgFileRefEntityPtr		mSgFileRefEntity;
		tGrowableArray< Gfx::tWorldSpaceLinesPtr > mRaycastSubmeshes;

	private:
		void fAddEditableProperties( );
		void fCommonCtor( const tResourceId& resourceId, const tResourcePtr& sigbResource );
		void fOnResourceLoaded( tResource& theResource, b32 success );

		f32 fMediumDistance( ) const;
		f32 fFarDistance( ) const;
		b32 fShowLODInEditor( ) const;

		void fBuildRaycastVisualization( );
		void fSetRaycastVisOn( b32 visOn );
		void fSetFreezeRaycastVis( b32 freeze );

	private:
		tGrowableArray<Gfx::tRenderableEntityPtr>	mGameDebugGeom;
		tResource::tOnLoadComplete::tObserver		mOnResourceLoaded;
		std::string									mToolTip;
		s32											mLastDisplayIndex;
		Gfx::tRenderableEntityPtr mMediumDistSphere, mFarDistSphere;
	};

}

#endif//__tSigmlReferenceEntity__
