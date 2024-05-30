#ifndef __tEditable3dGridEntity__
#define __tEditable3dGridEntity__
#include "tEditableObject.hpp"
#include "Gfx/tWorldSpaceLines.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig { namespace Sigml { class t3dGridObject; }}

namespace Sig
{
	class tools_export tEditable3dGridEntity : public tEditableObject
	{
		define_dynamic_cast( tEditable3dGridEntity, tEditableObject );
	private:
		Gfx::tRenderState mRenderState;
		Gfx::tWorldSpaceLinesPtr mGridLines;
		Math::tVec4f mCurrentTint;
	public:
		tEditable3dGridEntity( tEditableObjectContainer& container );
		tEditable3dGridEntity( tEditableObjectContainer& container, const Sigml::t3dGridObject& ao );
		~tEditable3dGridEntity( );
		virtual std::string fGetToolTip( ) const;
		virtual b32 fSupportsRotation( ) { return false; }
		virtual void fOnMoved( b32 recomputeParentRelative );
	private:
		void fAddEditableProperties( );
		void fCommonCtor( );
		void fRefreshLines( );
		void fAddPlane( tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, u32 vtxColor, const Math::tVec3f& center, const Math::tVec3f& e0, const Math::tVec3f& e1 );
	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
	};

}

#endif//__tEditable3dGridEntity__
