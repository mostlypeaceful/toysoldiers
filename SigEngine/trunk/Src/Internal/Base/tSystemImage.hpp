#ifndef __tSystemImage__
#define __tSystemImage__

#include "Gfx/tTextureFile.hpp"

namespace Sig
{
	/// Currently only implemented on metro.
	/// PC should probably use DXGI
	/// 360 might not have this concept at all
	///
	/// The idea is to support all the "standard" image formats provided by the system

	/// \brief Represents a (likely still compressed -- e.g. .png/.jpeg/whatever) image
	class base_export tSystemImage : public tRefCounter, tUncopyable
	{
	private:
		void* mRawPlatformHandle;
		tSystemImage() {}
	public:
		typedef Gfx::tTextureFile::tFormat tFormat;

		static tRefCounterPtr<tSystemImage> fFromMemory( byte* begin, u32 size );
		static tRefCounterPtr<tSystemImage> fFromFile( const tFilePathPtr& file );

		u32 fWidth() const;
		u32 fHeight() const;

		void fCopyTo( byte* buffer, tFormat format, u32 w, u32 h ) const;
		void fCopyTo( byte* buffer, tFormat format, u32 w, u32 h, u32 stride ) const;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tSystemImage );
}

#endif //ndef __tSystemImage__
