#ifndef __tFuiMemoryOptions__
#define __tFuiMemoryOptions__


namespace Sig { namespace Fui
{

	struct tFuiMemoryOptions
	{
		struct
		{
			u32 mNumHandles;
			u32 mNumBytes;
		} mRenderTargets, mTextures, mVertexBuffers;
		
		tFuiMemoryOptions( )
		{
			mRenderTargets.mNumHandles = 1;
			mRenderTargets.mNumBytes = Memory::fFromMB<u32>( 16 );

			mTextures.mNumHandles = 200;
			mTextures.mNumBytes = Memory::fFromMB<u32>( 25 );

			mVertexBuffers.mNumHandles = 1000;
			mVertexBuffers.mNumBytes = Memory::fFromMB<u32>( 1 );
		}
	};

} }//Sig::Fui

#endif//__tFuiMemoryOptions__
