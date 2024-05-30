#ifndef __tLoadInPlaceSerializerBase__
#define __tLoadInPlaceSerializerBase__

namespace Sig
{

	class base_export tLoadInPlaceSerializerBase
	{
	public:

		static const u32 cPointerOwnershipFlag = 0x80000000u;

		static const Rtti::tSerializableBaseClass* fCastToSerializableBaseClass( const void* object, const Rtti::tReflector& reflector )
		{
			// TODO one day we may find we actually need to recurse the reflector hierarchy to compute proper offsets
			// however, for now, we resort to this more ghetto approach, which works as long as Rtti::tBaseClass is the
			// first type inherited from in whatever 'object's' class tree is.
			return static_cast<Rtti::tSerializableBaseClass*>( ( Rtti::tBaseClass* )object );
		}
	};

}


#endif//__tLoadInPlaceSerializerBase__

