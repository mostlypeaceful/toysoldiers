#ifndef __tEditableShapeEntity__
#define __tEditableShapeEntity__
#include "tEditableObject.hpp"

namespace Sig { namespace Sigml { class tShapeObject; }}

namespace Sig
{
	class tools_export tEditableShapeEntity : public tEditableObject
	{
		define_dynamic_cast( tEditableShapeEntity, tEditableObject );
	private:
		Gfx::tRenderState mRenderState;
	public:
		tEditableShapeEntity( tEditableObjectContainer& container );
		tEditableShapeEntity( tEditableObjectContainer& container, const Sigml::tShapeObject& ao );
		~tEditableShapeEntity( );
		virtual std::string fGetToolTip( ) const;
		virtual b32 fUniformScaleOnly( );
	private:
		void fAddEditableProperties( );
		void fCommonCtor( );
	protected:
		virtual void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;
		virtual void fNotifyPropertyChanged( tEditableProperty& property );
	};

}

#endif//__tEditableShapeEntity__
