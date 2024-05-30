#ifndef __tSigmlReferenceEntity__
#define __tSigmlReferenceEntity__
#include "tEditableObject.hpp"
#include "Sigml.hpp"
#include "tSgFileRefEntity.hpp"

namespace Sig
{
	class tools_export tEditableSgFileRefEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableSgFileRefEntity, tEditableObject );
	public:
		tEditableSgFileRefEntity( tEditableObjectContainer& container, const tResourcePtr& sigbResource );
		tEditableSgFileRefEntity( tEditableObjectContainer& container, const Sigml::tSigmlReferenceObject& sigmlObject );
		~tEditableSgFileRefEntity( );
		tFilePathPtr fResourcePath( ) const;
		const tResourcePtr & fResource( ) const;
		b32 fIsEditable( ) const;
		b32 fIsRedBox( ) const;
		void fResetReference( const tFilePathPtr& newRefPath );
		void fRefreshDependents( const tResourcePtr& reloadedResource, b32 unloadOnly );
		std::string fGetToolTip( ) const;

	protected:
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
		s32 fGetDisplayIndex( ) const;
		void fUpdateDisplayedStates( u32 displayIndex );

	protected:
		tResourcePtr							mSgResource;
		tSgFileRefEntityPtr						mSgFileRefEntity;

	private:

		void fAddEditableProperties( );
		void fCommonCtor( const tResourcePtr& sigbResource );
		void fOnResourceLoaded( tResource& theResource, b32 success );

	private:

		tGrowableArray<Gfx::tRenderableEntityPtr>	mGameDebugGeom;
		tResource::tOnLoadComplete::tObserver		mOnResourceLoaded;
		std::string									mToolTip;
		s32											mLastDisplayIndex;
	};

}

#endif//__tSigmlReferenceEntity__
