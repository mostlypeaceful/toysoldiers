#ifndef __tFxmlReferenceEntity__
#define __tFxmlReferenceEntity__
#include "tEditableObject.hpp"
#include "tFxFileRefEntity.hpp"

namespace Sig
{
	namespace Sigml { class tFxmlReferenceObject; }
	class tools_export tFxmlReferenceEntity : public tEditableObject
	{
		define_dynamic_cast( tFxmlReferenceEntity, tEditableObject );
	private:
		tResource::tOnLoadComplete::tObserver	mOnResourceLoaded;
		FX::tFxFileRefEntityPtr			mFxSystem;
		std::string								mToolTip;
	public:
		tFxmlReferenceEntity( tEditableObjectContainer& container, const tResourcePtr& fxbResource );
		tFxmlReferenceEntity( tEditableObjectContainer& container, const Sigml::tFxmlReferenceObject& sigmlObject );
		~tFxmlReferenceEntity( );
		tFilePathPtr fResourcePath( ) const;
		void fResetReference( const tFilePathPtr& newRefPath );
		void fRefreshDependents( const tResourcePtr& reloadedResource );
		std::string fGetToolTip( ) const;
		std::string fGetAssetPath( ) const;
	private:
		void fAddEditableProperties( );
		void fCommonCtor( const tResourcePtr& fxbResource );
		void fOnResourceLoaded( tResource& theResource, b32 success );
	protected:
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
	};

}

#endif//__tFxmlReferenceEntity__
