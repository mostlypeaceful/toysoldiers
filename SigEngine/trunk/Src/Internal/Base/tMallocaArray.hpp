#ifndef __tMallocaArray__
#define __tMallocaArray__

#include "tArraySleeve.hpp"

namespace Sig
{
	#define malloca_array( type, name, count ) ::Sig::tMallocaArraySleeve<type> name( sigmalloca(type,count), count )

	/// \brief Owns a malloca-ed array.
	template<typename t>
	class tMallocaArraySleeve : public tArraySleeve<t>
	{
	private:
		using tArraySleeve<t>::fNewArray;
		using tArraySleeve<t>::fDeleteArray;
		using tArraySleeve<t>::fDisown;
	public:
		tMallocaArraySleeve(): tArraySleeve<t>() {}
		tMallocaArraySleeve( void* malloca_items, u32 count ): tArraySleeve<t>((t*)malloca_items,count) {}
		~tMallocaArraySleeve() { fDeleteArray(); }
		void fDeleteArray( ) { sigfreea(this->mItems); this->mItems = 0; this->mCount = 0; }
		void fAquire( t* malloca_items, u32 count ) { this->mItems = malloca_items; this->mCount = count; }
	};
}

#endif //ndef __tMallocaArray__
