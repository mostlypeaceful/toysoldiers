#include "ToolsPch.hpp"
#include "iSigEdPlugin.hpp"

namespace Sig
{

	tEditorPluginDataContainer& tEditorPluginDataContainer::operator = ( const tEditorPluginDataContainer& other )
	{
		fSetCount( other.fCount( ) );

		for( u32 i = 0; i < other.fCount( ); ++i )
			operator[]( i ).fReset( other[ i ]->fClone( ) );

		return *this;	
	}

	void tEditorPluginDataContainer::fUnion( const tEditorPluginDataContainer& other )
	{
		fJoin( other );
	}

	tEditorPluginDataContainer tEditorPluginDataContainer::fClone( ) const
	{
		tEditorPluginDataContainer result;
		result = *this;
		return result;
	}

}
