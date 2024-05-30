#ifndef __tLenseFlareData__
#define __tLenseFlareData__

namespace Sig { namespace Gfx
{

	struct base_export tLenseFlareData
	{
		struct tFlare
		{
			tFilePathPtr mTexture;
			f32 mPosition;
			Math::tVec2f mScale;
			Math::tVec4f mColor;

			tFlare( )
			{ }

			tFlare( const tFilePathPtr& path, f32 position, const Math::tVec2f& scale, const Math::tVec4f& color )
				: mTexture( path )
				, mPosition( position )
				, mScale( scale )
				, mColor( color )
			{ }

			// Serialization
			template< typename tSerializer >
			void fSerializeXml( tSerializer& s )
			{
				s( "t", mTexture );
				s( "p", mPosition );
				s( "s", mScale );
				s( "c", mColor );
			}

			b32 operator < ( const tFlare& other ) const
			{
				return mPosition < other.mPosition;
			}
		};

		tDynamicArray< tFlare > mFlares;

		// Serialization
		template< typename tSerializer >
		void fSerializeXml( tSerializer& s )
		{
			s( "f", mFlares );
		}
	};

}}

#endif //__tLenseFlareData__

