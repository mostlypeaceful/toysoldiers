#ifndef __CodeStandards__
#define __CodeStandards__

// all engine code should be wrapped in the 'Sig' namespace;
// all other namespaces should similarly follow the UpperCamelCase convention
namespace Sig { namespace CodeStandards
{

	///
	/// \brief All macros should be all lower-case, with underscores separating each word.
	#define standard_macro(x) x

	///
	/// \brief tExampleClass is the example class demonstrating all relevant coding conventions and standards.
	///
	/// All classes should begin with the letter 't' (for 'type'), and be followed by UpperCamelCase style naming.
	/// Additionally, all classes should begin with a doxygen style comment block (such as this one)
	class tExampleClass
	{
	public:

		// All types defined within the class scope should be declared first.

		///
		/// The name of typedefs, like classes and structs, should begin with lowercase t (for 'type')
		typedef b32 tResult;

		///
		/// \brief tConfig is an example of a simple data-only struct defined within the class tExampleClass.
		///
		/// The name of structs, like classes and typedefs, should begin with lowercase t (for 'type').
		struct tConfig
		{
			// Follows the standard member variable convention (see below).
			u32 mMode;
		};

		///
		/// \brief Enums follow the same naming rules as classes, structs, and typedefs.
		///
		/// Namely, they should begin with a lower-case 't' (for 'type') and continue with UpperCamelCase.
		/// Additionally, the values of an enum should follow the rules for constant values, using the
		/// lower-case 'c' (for 'constant'), followed by UpperCamelCase.
		enum tState
		{
			cState0, cState1, cState2, cStateCount
		};

	public:

		// All member data should be declared immediately following.  It is very important to be aware 
		// of the data layout of your class, and front loading the class declaration with all the data 
		// helps keep this explicit.

		///
		/// Member variables, whether public or private, whether members of structs or classes, should
		/// begin with a lower-case 'm' (for 'member'), and be followed by UpperCamelCase style naming.
		tResult		mResult;

		///
		/// The built-in type typedefs (f32,u32,etc) should be used over names like float or int
		f32			mVelocity;	///< this is an alternative, short-hand doxygen style for the brief statement

	public:

		// All static data should be declared next.

		///
		/// Non-constant globals should begin with a lower-case 'g' (for 'global'), and proceed with UpperCamelCase.
		static u32			gGlobal;

		///
		/// Constants (whether global or local) should begin with a lower-case 'c' (for 'constant'), and
		/// proceed with UpperCamelCase.
		static const u32	cConstantFlag = 0x0001;

	public:

		///
		/// \brief Example of a simple inline method defined in the header file.
		///
		/// All functions and methods should begin with a lower-case 'f' (for 'function'), and be followed
		/// by UpperCamelCase style naming.  Additionally, all functions should be written using const-correctness.
		inline tResult fInitialize( ) const { return mResult; }
	};


}}


#endif//__CodeStandards__
