#ifndef __tEditableGroupEntity__
#define __tEditableGroupEntity__
#include "tEditableObject.hpp"


namespace Sig
{

	class tools_export tEditableGroupEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableGroupEntity, tEditableObject );

		tGrowableArray< tEntityPtr > mHeldEntities;

	public:
		tEditableGroupEntity( tEditableObjectContainer& container );
		// No serialization constructor. Not serializable.
		virtual ~tEditableGroupEntity( );
		virtual std::string fGetToolTip( ) const;

		Sigml::tObjectPtr fSerialize( b32 clone ) const { return Sigml::tObjectPtr(); }

		void fAddObjects( tEditorSelectionList& entities );
		void fEmptyObjects( );
		tEditorSelectionList fGetObjects( ) const;

	protected:
		void fCommonCtor( );
	};

}

#endif//__tEditableGroupEntity__
