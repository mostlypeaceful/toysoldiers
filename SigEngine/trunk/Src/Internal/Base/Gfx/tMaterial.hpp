#ifndef __tMaterial__
#define __tMaterial__
#include "tVertexFormat.hpp"
#include "tRenderState.hpp"

namespace Sig { namespace Gfx
{
	class tDevicePtr;
	class tRenderContext;
	class tRenderBatchData;
	class tRenderInstance;
	class tDrawCall;
	class tMaterial;
	class tRimLightShaderConstants;
	class tLightShaderConstantsArray;
	struct tLightShaderConstants;

	define_smart_ptr( base_export, tRefCounterPtr, tMaterial );

	///
	/// TODO document
	class base_export tMaterial : public Rtti::tSerializableBaseClass, public tUncopyable, public tRefCounter
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tMaterial, 0x9EFC373);
		define_dynamic_cast_base(tMaterial);
	public:

		enum tMaterialFlags
		{
			cFaceX = (1<<0),
			cFaceY = (1<<1),
			cFaceZ = (1<<2),

			cFaceMask = cFaceX | cFaceY | cFaceZ, // haha, he said "face mask"

			cBehaviorOwnsMaterialFileResource = (1<<3),
			cBehaviorXparentDepthPrepass = (1<<4), // Material requires depth only prepass when rendering xparently.
		};

		static const u32 cMaxLights = 6;
		static const u32 cLightSlotCount = cMaxLights + 1;
		static const u32 cMaxShadowLayers = 3;
		static const u32 cMaxBoneCount = 70;

	protected:

		tLoadInPlaceResourcePtr*	mMaterialFile;
		tRenderState				mRenderState;
		u32							mMaterialFlags;

	public:

		tMaterial( );
		tMaterial( tNoOpTag );
		virtual ~tMaterial( );

		void						fSetMaterialFileResourcePtrUnOwned( tLoadInPlaceResourcePtr* mtlFileResPtr );
		void						fSetMaterialFileResourcePtrOwned( tResourceDepot& resDepot, const tFilePathPtr& materialFilePath );
		void						fSetMaterialFileResourcePtrOwned( const tResourcePtr& resourcePtr );
		void						fSetFacingFlags( b32 x, b32 y, b32 z );
		void						fSetXparentDepthPrepassFlag( b32 enabled );

		inline void					fSetRenderState( const tRenderState& rs ) { mRenderState = rs; }
		inline const tRenderState&  fGetRenderState( ) const { return mRenderState; }

		inline b32					fRequiresXparentDepthPrepass( ) const { return mMaterialFlags & cBehaviorXparentDepthPrepass; }

		///
		/// \brief This method might return a different format depending on the
		/// way the specific derived tMaterial instance is configured. Some material
		/// types might always return the same format, others might for example have
		/// options for skinned, static, etc.
		virtual const tVertexFormat& fVertexFormat( ) const = 0;

		///
		/// \brief Return true if your material uses lights.
		virtual b32 fIsLit( ) const { return false; }

		///
		/// \brief Return true if your material renders into the depth map.
		virtual b32 fRendersDepth( ) const { return false; }

		///
		/// \brief Return true if your material supports hardware instancing.
		virtual b32 fSupportsInstancing( ) const { return false; }

		///
		/// \brief Apply any non instance-specific attributes of the material.
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const = 0;

		///
		/// \brief Apply instance-specific attributes of the material.
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const = 0;

		///
		/// \brief Get the underlying material file path or tFilePathPtr::cNullPtr
		const tFilePathPtr& fGetPath( ) const;

	public:

		void fApplyObjectToWorldVS( const tDevicePtr& device, u32 posId, u32 normalId, const tDrawCall& drawCall, const tRenderContext& context ) const;

		void fApplyVector3VS( const tDevicePtr& device, u32 id, const Math::tVec3f& vector ) const;
		void fApplyVector4VS( const tDevicePtr& device, u32 id, const Math::tVec4f& vector, u32 count = 1 ) const;
		void fApplyMatrix3VS( const tDevicePtr& device, u32 id, const Math::tMat3f& matrix, u32 count = 1 ) const;
		void fApplyMatrix4VS( const tDevicePtr& device, u32 id, const Math::tMat4f& matrix, u32 count = 1 ) const;
		void fApplyMatrixPaletteVS( const tDevicePtr& device, u32 id, const Math::tMat3f* matrix, u32 numEntries ) const;

		void fApplyVector3PS( const tDevicePtr& device, u32 id, const Math::tVec3f& vector ) const;
		void fApplyVector4PS( const tDevicePtr& device, u32 id, const Math::tVec4f& vector, u32 count = 1 ) const;
		void fApplyMatrix3PS( const tDevicePtr& device, u32 id, const Math::tMat3f& matrix, u32 count = 1 ) const;
		void fApplyMatrix4PS( const tDevicePtr& device, u32 id, const Math::tMat4f& matrix, u32 count = 1 ) const;

		void fApplyRimLight( const tDevicePtr& device, u32 id, const tRimLightShaderConstants& lightConstants ) const;
		void fApplyLights( const tDevicePtr& device, u32 id, const tLightShaderConstantsArray& lightConstants ) const;
		void fApplyLight( const tDevicePtr& device, u32 id, const tLightShaderConstants& lightConstants ) const;

		void fApplyTexture( const tDevicePtr& device, u32& slot, tLoadInPlaceResourcePtr* texture ) const;

	private:

		void fCleanupOwnedMaterialFileResourcePtr( );
	};

}}

#endif//__tMaterial__
