#ifndef __Animap__
#define __Animap__

#include "tAniMapFile.hpp"
#include "Editor/tEditableProperty.hpp"

namespace Sig { namespace Animap
{
	/*
		
		The animap associates animations from their anipks with anim track inputs from Momaps.

	*/

	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsAnimapFile( const tFilePathPtr& path );

	class tools_export tFile
	{
	public:
		struct tools_export tAnimRef
		{
			std::string mAnipack;
			std::string mAnimName;
			tEditablePropertyTable mProperties;

			tAnimRef( const std::string& pack = "", const std::string& anim = "" )
				: mAnipack( pack )
				, mAnimName( anim )
			{
				fAddCommonProps( );
			}

			b32 operator == ( const tAnimRef& other ) const { return mAnipack == other.mAnipack && mAnimName == other.mAnimName; }

			f32 fTimeScale( ) const;
		
		private:
			void fAddCommonProps( );
		};

		struct tContextSwitch
		{
			u32 mContextKey;
			tGrowableArray< tContextSwitch > mBranches;
			tGrowableArray< tAnimRef > mLeaves;

			tContextSwitch( )
				: mContextKey( ~0 )
			{ }
		};

		struct tMapping
		{
			std::string		mName; // name of anim track input from momap.
			tContextSwitch	mRoot; // the beginning of a context tree

			tMapping( const std::string& name = "" )
				: mName( name )
			{ }

			b32 operator == ( const std::string& name ) const { return mName == name; }
		};

		tGrowableArray< tMapping > mMappings;

	public:
		tFile( );

		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );

		tAniMapFile* fMakeAnimapFile( ) const;

		void fGatherAnimPacks( tFilePathPtrList& output );

	};

}}

#endif//__Animap__
