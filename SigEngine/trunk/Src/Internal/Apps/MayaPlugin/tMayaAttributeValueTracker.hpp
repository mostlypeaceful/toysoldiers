#ifndef __tMayaAttributeValueTracker__
#define __tMayaAttributeValueTracker__

namespace Sig
{

	template<class tValue>
	class tMayaAttributeValueTracker
	{
		b32		mConflict;
		b32		mFound;
		tValue	mValue;
		tValue  mDefValue;

	public:

		tMayaAttributeValueTracker( const tValue& defValue = tValue(0) )
			: mConflict( false ), mFound( false ), mValue( defValue ), mDefValue( defValue )
		{
		}

		void fStartTracking( )
		{
			mConflict = false;
			mFound = false;
			mValue = mDefValue;
		}

		b32 fUpdate( b32 found, const tValue& value )
		{
			if( !found )
			{
				// this object doesn't have a value set;
				// that automatically marks it as a conflict
				mConflict = true;
				return false;
			}

			if( !mFound )
			{
				// this is the first object we've visited
				mFound = true;
				mValue = value;
				return true;
			}

			if( !fEqual( value, mValue ) )
			{
				// we've now visited multiple objects,
				// and this one's value is different from
				// the previous ones
				mConflict = true;
				return false;
			}

			return true;
		}

		b32 fIsValueConsistent( ) const
		{
			return !mConflict && mFound;
		}

		b32 fIsValueFound( ) const
		{
			return mFound;
		}

		const tValue& fGetValue( ) const
		{
			return mValue;
		}

	};
}


#endif//__tMayaAttributeValueTracker__
