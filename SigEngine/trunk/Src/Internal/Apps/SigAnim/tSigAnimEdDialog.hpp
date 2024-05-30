#ifndef __tSigAnimEdDialog__
#define __tSigAnimEdDialog__
#include "tWxSlapOnDialog.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tKeyFrameAnimation.hpp"
#include "Anipk.hpp"


namespace Sig
{
	class tSigAnimMainWindow;

	class tSigAnimEdDialog : public tWxSlapOnDialog
	{
	public:

		class tOwner
		{
		public:
			virtual void fMarkCurrentAnimPackDirty( ) = 0;
			virtual void fSaveAllAnimPacks( ) = 0;
			virtual f32 fGetAnimTime( const std::string& animName ) const = 0;

		};

		class tCheckBox : public tWxSlapOnCheckBox
		{
			tSigAnimEdDialog& mOwner;
			b32* mDataPtr;
		public:
			tCheckBox( tSigAnimEdDialog& owner, wxWindow* parent, const char* label )
				: tWxSlapOnCheckBox( parent, label ), mOwner( owner ), mDataPtr( 0 ) { }
			virtual void fOnControlUpdated( ) { if( mDataPtr && *mDataPtr != fGetValueBool( ) ) { *mDataPtr = fGetValueBool( ); mOwner.fOnAnimValueModified( ); } }
			void fSetDataPtr( b32* dataPtr ) { mDataPtr = dataPtr; if( dataPtr ) fSetValue( *dataPtr ? tWxSlapOnCheckBox::cTrue : tWxSlapOnCheckBox::cFalse ); }
		};
		class tSpinner : public tWxSlapOnSpinner
		{
			tSigAnimEdDialog& mOwner;
			f32* mDataPtr;
		public:
			tSpinner( tSigAnimEdDialog& owner, wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision )
				: tWxSlapOnSpinner( parent, label, min, max, increment, precision ), mOwner( owner ), mDataPtr( 0 ) { }
			virtual void fOnControlUpdated( ) { if( mDataPtr && *mDataPtr != fGetValue( ) ) { *mDataPtr = fGetValue( ); mOwner.fOnAnimValueModified( );  } }
			void fSetDataPtr( f32* dataPtr ) { mDataPtr = dataPtr; if( dataPtr ) fSetValueNoEvent( *dataPtr ); }
		};
	private:
		tOwner & mOwner;
		wxScrolledWindow* mMainPanel;
		wxStaticText* mCurrentAnimPackText;
		wxStaticText* mCurrentAnimText;
		wxStaticText* mCurrentAnimFlagsText;
		wxStaticText* mCurrentAnimLengthText;
		tCheckBox* mDisableCompression;
		tSpinner* mCompressionErrorP;
		tSpinner* mCompressionErrorR;
		tSpinner* mCompressionErrorS;
		wxListBox* mEventTagListBox;
		Anipk::tAnimationMetaData* mAnimMetaData;
	public:
		tSigAnimEdDialog( tOwner & owner, wxWindow * parent );
		void fClearAnim( );
		void fSetAnim( const std::string& animPkLabel, const std::string& animName, Anipk::tFile& anipkFile, const tKeyFrameAnimation* kfAnim );
		void fOnAnimValueModified( );
		void fClearDirty( );
		void fOnTick( );
	private:
		void fOnAction( wxCommandEvent& );
		void fOnAddEventTagPressed( wxCommandEvent& );
		void fOnSubEventTagPressed( wxCommandEvent& );
		void fOnModifyEventTag( wxCommandEvent& );
		void fAddKeyframeEventToListBox( f32 time, const std::string& type, const std::string& tag );
		b32  fExtractKeyFrameTag( s32 ithListBoxItem, Anipk::tKeyFrameTag& kft ) const;
		void fSyncCurrentAnimToEventListBox( );
		void fSyncEventListBoxFromMetaData( );
		void fBuildGui( );
	};
}

#endif//__tSigAnimEdDialog__
