#ifndef __Fxml__
#define __Fxml__

#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "Sigml.hpp"
#include "FX/tParticleSystem.hpp"
#include "FX/tParticleAttractor.hpp"
#include "FX/tMeshSystem.hpp"
#include "Editor/tEditableLightEntity.hpp"
#include "FxEditor/tToolAnimatedLightData.hpp"

namespace Sig
{
	class tResourceId;
	class tLoadInPlaceFileBase;
	class tSceneGraphFile;
	
	class tEntity;
	class tEditableObject;
	class tEntityDef;
	class tEntityDefProperties;
	class tFxmlConverter;
	class tEditableObjectContainer;

	class tToolAnimatedLightData;
	typedef tRefCounterPtr< tToolAnimatedLightData > tToolAnimatedLightDataPtr;
}

namespace Sig { namespace Gfx
{
	class tMaterial;
}}

namespace Sig { namespace Fxml
{
	class tFile;

	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsFxmlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fFxmlPathToFxb( const tFilePathPtr& path );
	tools_export tFilePathPtr fFxbPathToFxml( const tFilePathPtr& path );

	
	///
	/// \class tFxLightObject
	/// \brief Serialization object for an editable FX light
	class tools_export tFxLightObject : public Sigml::tPointLightObject
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tFxLightObject, 0xC281E771 );

	public:
		tToolAnimatedLightDataPtr mData;

	public:

		tFxLightObject( ) { }

		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEntityDef* fCreateEntityDef( tFxmlConverter& fxmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	///
	/// \class tFxLightObject
	/// \brief Serialization object for a mesh system
	class tools_export tFxMeshSystemObject : public Sigml::tObject
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tFxMeshSystemObject, 0x89A0AB57 );

	public:

		tFilePathPtr									mReferencePath;		// don't know if we need this or not...
		tStringPtr										mFxMeshSystemName;
		tStringPtr										mParticleSystemToSyncWith;
		tFilePathPtr									mMeshResourceFile;

		FX::FxMeshSystem::tMeshSystemDataPtr		mFxMeshSystemData;

	public:

		tFxMeshSystemObject( ) {	}

		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEntityDef* fCreateEntityDef( tFxmlConverter& fxmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};



	class tools_export tFxAttractorObject : public Sigml::tObject
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tFxAttractorObject, 0x2B3DC4C1 );

	public:

		tFilePathPtr				mReferencePath;		// don't know if we need this or not...
		tStringPtr					mAttractorName;
		FX::tToolAttractorDataPtr	mToolData;
		FX::tAttractorDataPtr		mAttractorData; // Deprecated but supported
		b32 mOpened;

	public:

		tFxAttractorObject( )
			: mOpened( false )
		{ }

		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEntityDef* fCreateEntityDef( tFxmlConverter& fxmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	///
	/// \brief Object representing a reference to another fxml file.
	class tools_export tFxParticleSystemObject : public Sigml::tObject
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tFxParticleSystemObject, 0xB8FEA61F );

	public:
		tFilePathPtr mReferencePath;		// don't know if we need this or not...

		// particle system informations here!
		FX::tToolParticleSystemStatePtr	mToolState;
		tGrowableArray< FX::tStatePtr > mStates;   // Deprecated

		tStringPtr						mParticleSystemName;
		b32								mLocalSpace;
		f32								mCameraDepthOffset;
		f32								mUpdateSpeedMultiplier;
		f32								mLodFactor;
		f32								mGhostParticleFrequency;
		f32								mGhostParticleLifetime;
		FX::tEmitterType				mEmitterType;

		u32	mBlendOp;
		u32 mSrcBlend;
		u32 mDstBlend;
		FX::tParticleSortMode	mSortMode;

		Sigml::tMaterialPtr mMaterial;
		tFilePathPtr mMeshResourcePath;

	public:
		tFxParticleSystemObject( );

		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEntityDef* fCreateEntityDef( tFxmlConverter& fxmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};


	///
	/// \brief Encapsulates all the objects that make up a Fxml file.
	/// Purpose: allow building up of a file object using normal code
	/// data structures, then serializing to xml; or, alternatively,
	/// the opposite.
	class tools_export tFile
	{
	public:
		u32								mVersion;
		Sig::Sigml::tObjectPtrArray		mObjects;
		f32								mLifetime;
		u32								mFlags;
		
	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		
	};

}}

#endif//__Fxml__
