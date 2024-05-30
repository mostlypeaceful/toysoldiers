#ifndef __Metaprogramming_Conditionals__
#define __Metaprogramming_Conditionals__

namespace Sig { namespace Metaprogramming
{
	/// \brief SelectIfC.
	/// Has a single typedef member, tResult, which will correspond to tN for the first condN that was true.

	template<
		bool cond1, typename t1,
		bool cond2 = false, typename t2 = void,
		bool cond3 = false, typename t3 = void,
		bool cond4 = false, typename t4 = void,
		bool cond5 = false, typename t5 = void>
	struct SelectIfC;
	
	template<
		typename t1,
		bool cond2, typename t2,
		bool cond3, typename t3,
		bool cond4, typename t4,
		bool cond5, typename t5>
	struct SelectIfC<
		true , t1,
		cond2, t2,
		cond3, t3,
		cond4, t4,
		cond5, t5>
	{
		typedef t1 tResult;
	};
	
	template<
		typename t1,
		typename t2,
		bool cond3, typename t3,
		bool cond4, typename t4,
		bool cond5, typename t5>
	struct SelectIfC<
		false, t1,
		true , t2,
		cond3, t3,
		cond4, t4,
		cond5, t5>
	{
		typedef t2 tResult;
	};
	
	template<
		typename t1,
		typename t2,
		typename t3,
		bool cond4, typename t4,
		bool cond5, typename t5>
	struct SelectIfC<
		false, t1,
		false, t2,
		true , t3,
		cond4, t4,
		cond5, t5>
	{
		typedef t3 tResult;
	};
	
	template<
		typename t1,
		typename t2,
		typename t3,
		typename t4,
		bool cond5, typename t5>
	struct SelectIfC<
		false, t1,
		false, t2,
		false, t3,
		true , t4,
		cond5, t5>
	{
		typedef t4 tResult;
	};
	
	template<
		typename t1,
		typename t2,
		typename t3,
		typename t4,
		typename t5>
	struct SelectIfC<
		false, t1,
		false, t2,
		false, t3,
		false, t4,
		true , t5>
	{
		typedef t5 tResult;
	};
}} // namespace Sig::Metaprogramming

#endif //ndef __Metaprogramming_Conditionals__
