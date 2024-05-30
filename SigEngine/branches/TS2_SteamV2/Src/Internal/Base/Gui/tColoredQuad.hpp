#ifndef __tColoredQuad__
#define __tColoredQuad__
#include "tRenderableCanvas.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tDynamicGeometry.hpp"

namespace Sig { namespace Gfx
{
	class tDevice;
	struct tDefaultAllocators;
}}

namespace Sig { namespace Gui
{
	class tColoredQuad : public tRenderableCanvas
	{
		define_dynamic_cast( tColoredQuad, tRenderableCanvas );
	public:
		tColoredQuad( );
		explicit tColoredQuad( Gfx::tDefaultAllocators& allocators );
		virtual ~tColoredQuad( );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
		void fSetRect( const Math::tVec2f& topLeft, const Math::tVec2f& widthHeight );
		void fSetRect( const Math::tVec2f& widthHeight );
		void fSetRect( const tRect& rect );
		void fSetVertColors( const Math::tVec4f& topLeft, const Math::tVec4f& topRight, const Math::tVec4f& botLeft, const Math::tVec4f& botRight );

		const Gfx::tRenderState&	fRenderState( ) const { return mRenderState; }
		void						fSetRenderState( const Gfx::tRenderState& state ) { mRenderState = state; }

	protected:

		void fCommonCtor( );
		void fResetDeviceObjects( Gfx::tDevice& device );
		void fUpdateGeometry( );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		Gfx::tSolidColorMaterialPtr	mMaterial;
		tResourcePtr				mColorMap;
		Gfx::tDynamicGeometry		mGeometry;
		Gfx::tRenderState			mRenderState;
		Gfx::tDefaultAllocators&	mAllocators;
		u32							mVertColors[4];
	};

}}

#endif//__tColoredQuad__
