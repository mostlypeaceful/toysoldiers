#ifndef __tEnum__
#define __tEnum__

namespace Sig
{
	/// \brief Enum wrapper, so that you can control the storage size.
	template<class tRealEnum, class tStorage>
	class tEnum
	{
		declare_reflector( );
	private:
		tStorage mValue;
	public:
		inline tEnum( ) { }
		inline tEnum( tRealEnum value ) : mValue( ( tStorage )value ) { }
		inline operator tRealEnum( ) const { return ( tRealEnum )mValue; }
	};
}

#endif //ndef __tEnum__
