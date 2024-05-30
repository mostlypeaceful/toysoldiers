#ifndef __tRenderToTextureAgent__
#define __tRenderToTextureAgent__

#include "tLightEntity.hpp"
#include "tRenderToTexture.hpp"

namespace Sig { namespace Gfx
{

	class base_export tRenderToTextureAgent : public tRefCounter
	{
	public:
		tEntityPtr						mRoot; //what gets rendered
		tCamera							mCamera;
		tLightEntityPtr					mLight;

		tResourcePtr					mTextureResource; //if we are given a texture file then keep the resource around until rendering is complete
		tTextureFile::tPlatformHandle	mOutTexture; //What gets rendered to
		tRenderToTexturePtr				mRtt;
		Math::tVec4f					mClearColor;

		tRenderToTextureAgent( );
		~tRenderToTextureAgent( );

		void fFromSigml( tScreen& screen, tEntity& sigml, const Math::tMat3f& camera, const tFilePathPtr& texture );

		b32 operator = ( const tEntity* root ) const { return mRoot == root; }
	};
	typedef tRefCounterPtr< tRenderToTextureAgent > tRenderToTextureAgentPtr;

}}
#endif//__tRenderToTextureAgent__

