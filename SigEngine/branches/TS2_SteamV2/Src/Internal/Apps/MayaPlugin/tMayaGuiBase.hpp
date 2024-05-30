#ifndef __tMayaGuiBase__
#define __tMayaGuiBase__
#include "tMayaEvent.hpp"
#include "tMayaAttributeValueTracker.hpp"

namespace Sig
{

	class tMayaGuiBase
	{
	private:
		MFn::Type		mMayaObjectType;

	protected:
		tMayaGuiBase( );
		inline void fSetMayaObjectType( MFn::Type t ) { mMayaObjectType = t; }

	public:
		inline MFn::Type fGetMayaObjectType( ) const { return mMayaObjectType; }
		b32 fIsNodeMyType( MDagPath& path ) const;
	};

	class tMayaContainerBase : public tMayaGuiBase
	{
	};

	class tMayaControlBase : public tMayaGuiBase
	{
	};

	class tMayaAttributeControlBase : public tMayaControlBase
	{
	private:
		tMayaEventPtr	mOnMayaSelChanged;
	protected:
		tMayaAttributeControlBase( tMayaGuiBase* parent );
		virtual void fOnMayaSelChanged( );
	};

}

#endif//__tMayaGuiBase__
