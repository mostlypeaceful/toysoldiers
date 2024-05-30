#ifndef __tIndexFormat__
#define __tIndexFormat__

namespace Sig { namespace Gfx
{

	///
	/// \brief Describes the format of an index buffer used for
	/// rendering primitives from a geometry buffer.
	class base_export tIndexFormat
	{
		declare_reflector( );
	public:

		enum tStorageType
		{
			cStorageU16,
			cStorageU32,

			// last
			cStorageTypeCount,
			cStorageInvalid = cStorageTypeCount
		};

		enum tPrimitiveType
		{
			cPrimitiveTriangleList,
			cPrimitiveTriangleStrip,
			cPrimitiveLineList,
			cPrimitiveLineStrip,

			// last
			cPrimitiveTypeCount,
			cPrimitiveInvalid = cPrimitiveTypeCount
		};

	public:
		tEnum<tStorageType,u8>		mStorageType;
		tEnum<tPrimitiveType,u8>	mPrimitiveType;
		u16							mSize; // computed from constructor
		u32							mMaxValue; // computed from constructor

	public:

		static u32 fGetIndexSize( tStorageType storageType );
		static u32 fGetMaxAllowableValue( tStorageType storageType );

		///
		/// \brief Returns an index format that is appropriate for the specified 'highestVertexIndex';
		/// in 99.9% of the cases this value will be equivalent to the number of vertices in your vertex buffer.
		static tIndexFormat fCreateAppropriateFormat( tPrimitiveType primType, u32 highestVertexIndex );

		inline tIndexFormat( )
			: mStorageType( cStorageInvalid )
			, mPrimitiveType( cPrimitiveInvalid )
			, mSize( 0 )
			, mMaxValue( 0 )
		{
		}

		inline tIndexFormat( tStorageType storageType, tPrimitiveType primType )
			: mStorageType( storageType )
			, mPrimitiveType( primType )
			, mSize( fGetIndexSize( storageType ) )
			, mMaxValue( fGetMaxAllowableValue( storageType ) )
		{
		}

		inline tIndexFormat( tNoOpTag )
		{
		}

		inline b32 fValid( ) const { return mStorageType != cStorageInvalid && mPrimitiveType != cPrimitiveInvalid && mSize > 0; }
	};

}}

#endif//__tIndexFormat__

