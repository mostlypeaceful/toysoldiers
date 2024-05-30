#ifndef __tSigFxMeshSystem__
#define __tSigFxMeshSystem__

#include "Editor/tEditableObject.hpp"
#include "FX/tMeshSystem.hpp"
#include "Fxml.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tGeometryBufferVRamSlice.hpp"
#include "Gfx/tIndexBufferVRamSlice.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig
{
	
	class tools_export tSigFxMeshSystem: public tEditableObject
	{
		define_dynamic_cast( tSigFxMeshSystem, tEditableObject );
	public:

		tSigFxMeshSystem( tEditableObjectContainer& container );
		tSigFxMeshSystem( tEditableObjectContainer& container, const Fxml::tFxMeshSystemObject& fxmso);

		virtual ~tSigFxMeshSystem( );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		void fClone( const tSigFxMeshSystem& meshSystem );

		FX::tMeshSystemPtr fFxMeshSystem( ) const { return mFxMeshSystem; }

		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		virtual void fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max );

		void fSetMeshSystemName( const tStringPtr& name ) { mFxMeshSystem->fSetMeshSystemName( name ); }
		const tStringPtr& fFxMeshSystemName( ) const { return mFxMeshSystem->fFxMeshSystemName( ); }

		void fSetParticleSystemToSyncWith( const tStringPtr& syncWith ) { mFxMeshSystem->fSetParticleSystemToSyncWith( syncWith ); fSyncWithSystem( ); }
		const tStringPtr& fParticleSystemToSyncWith( ) const { return mFxMeshSystem->fParticleSystemToSyncWith( ); }

		void fSetMeshResourceFile( const tFilePathPtr& meshFile, b32 nameOnly = false );
		const tFilePathPtr& fMeshResourceFile( ) const;


		u32 mLastOpenGraphIdx;

		void fSetHidden( b32 hide ) { mHidden = hide; }
		b32 fHidden( ) const { return mHidden; }

	private:

		void fSyncWithSystem( );		//sync up the mesh system with any particle systems that need be.

	private:

		FX::tMeshSystemPtr mFxMeshSystem;

		b32	mHidden;
		void fCommonCtor( tResourceDepot& resDep );
	};


}

#endif // __tSigFxMeshSystem__