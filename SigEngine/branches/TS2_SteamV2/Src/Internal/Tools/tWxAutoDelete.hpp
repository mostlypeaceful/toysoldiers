#ifndef __tWxAutoDelete__
#define __tWxAutoDelete__

namespace Sig
{
	///
	/// \brief Helper class for incorporating custom types into a wxWidgets
	/// window hierarchy. If your type doesn't inherit from wxWindow, but instead
	/// contains controls, you can inherit from this type so that your object
	/// will get auto deleted like the rest of the wxWidgets controls.
	class tools_export tWxAutoDelete : tUncopyable
	{
	protected:
		wxWindow* mParent;
	public:
		virtual ~tWxAutoDelete( );
		inline wxWindow* fParent( ) const { return mParent; }
	protected:
		tWxAutoDelete( wxWindow* parent );
	};

}


#endif//__tWxAutoDelete__
