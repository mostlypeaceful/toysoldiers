/**
 * @file Math.cpp
 * @brief Implementation file for the Math namespace.
 *
 * This file contains the implementation of various mathematical functions and classes
 * used in the SigEngine project. It includes vector, matrix, and geometry classes,
 * as well as utility functions for mathematical operations.
 *
 * The Math namespace provides functionality for performing common mathematical operations,
 * such as vector and matrix operations, interpolation, trigonometric functions, and more.
 * It also includes classes for representing geometric shapes, such as spheres, AABBs, and OBBs.
 *
 * The code in this file is part of the SigEngine project and is located in the
 * /c:/ToySoldiers/SigEngine/branches/TS2_SteamV2/Src/Internal/Base/Math/ directory.
 */
#include "BasePch.hpp"

namespace Sig
{
	namespace Math
	{
#pragma warning(disable : 4723)
		static const f32 cZero = 0.f;
		const f32 cInvalidFloat = 1.f / cZero;
#pragma warning(default : 4723)

		const f32 cAntiPodalQuatThreshold = -0.333f;

		sig_static_assert(sizeof(tDebugF32) == sizeof(f32));

		template class tVector1<s32>;
		template class tVector2<s32>;
		template class tVector3<s32>;
		template class tVector4<s32>;

		template class tVector1<u32>;
		template class tVector2<u32>;
		template class tVector3<u32>;
		template class tVector4<u32>;

		template class tVector1<f32>;
		template class tVector2<f32>;
		template class tVector3<f32>;
		template class tVector4<f32>;

		template class tVector1<f64>;
		template class tVector2<f64>;
		template class tVector3<f64>;
		template class tVector4<f64>;

		template class tAxisAngle<f32>;
		template class tAxisAngle<f64>;

		template class tQuaternion<f32>;
		template class tQuaternion<f64>;

		template class tMatrix3<f32>;
		template class tMatrix3<f64>;

		template class tMatrix4<f32>;
		template class tMatrix4<f64>;

		template class tSphere<f32>;
		template class tSphere<f64>;

		template class tAabb<f32>;
		template class tAabb<f64>;

		template class tObb<f32>;
		template class tObb<f64>;

		template class tPlane<f32>;
		template class tPlane<f64>;

		template class tFrustum<f32>;
		template class tFrustum<f64>;

		template class tRay<f32>;
		template class tRay<f64>;

		template class tTriangle<f32>;
		template class tTriangle<f64>;
	}
}

namespace Sig
{
	namespace Math
	{
		namespace
		{
#define fExportVecCommon(tVecType, classDesc)                                                                              \
	{                                                                                                                      \
		classDesc                                                                                                          \
			.StaticFunc<tVecType (*)(const tVecType &, const tVecType &, const f32 &)>(_SC("Lerp"), &fLerp<tVecType, f32>) \
			.Func(_SC("_add"), &tVecType::operator+)                                                                       \
			.Func<tVecType (tVecType::*)(const tVecType &) const>(_SC("_sub"), &tVecType::operator-)                       \
			.Func<tVecType (tVecType::*)(f32) const>(_SC("_mul"), &tVecType::operator*)                                    \
			.Func<tVecType (tVecType::*)(f32) const>(_SC("_div"), &tVecType::operator/)                                    \
			.Func<tVecType (tVecType::*)() const>(_SC("_unm"), &tVecType::operator-)                                       \
			.Func(_SC("Length"), &tVecType::fLength)                                                                       \
			.Func(_SC("LengthSquared"), &tVecType::fLengthSquared)                                                         \
			.Func(_SC("Dot"), &tVecType::fDot)                                                                             \
			.Func<tVecType &(tVecType::*)(void)>(_SC("Normalize"), &tVecType::fNormalizeSafe)                              \
			.Func(_SC("IsZero"), &tVecType::fIsZero)                                                                       \
			.Func(_SC("Clone"), &tVecType::fClone);                                                                        \
	}

			static void fExportVec2(tScriptVm &vm)
			{
				Sqrat::Class<tVec2f, Sqrat::DefaultAllocator<tVec2f>> classDesc(vm.fSq());
				classDesc
					.StaticFunc(_SC("Construct"), &tVec2f::fConstruct)
					.Var(_SC("x"), &tVec2f::x)
					.Var(_SC("y"), &tVec2f::y)
					.Func(_SC("Angle"), &tVec2f::fAngle);
				fExportVecCommon(tVec2f, classDesc);
				vm.fNamespace(_SC("Math")).Bind(_SC("Vec2"), classDesc);
			}
			static void fExportVec3(tScriptVm &vm)
			{
				Sqrat::Class<tVec3f, Sqrat::DefaultAllocator<tVec3f>> classDesc(vm.fSq());
				classDesc
					.StaticFunc(_SC("Construct"), &tVec3f::fConstruct)
					.Var(_SC("x"), &tVec3f::x)
					.Var(_SC("y"), &tVec3f::y)
					.Var(_SC("z"), &tVec3f::z)
					.Func(_SC("Cross"), &tVec3f::fCross);
				fExportVecCommon(tVec3f, classDesc);
				vm.fNamespace(_SC("Math")).Bind(_SC("Vec3"), classDesc);
			}
			static void fExportVec4(tScriptVm &vm)
			{
				Sqrat::Class<tVec4f, Sqrat::DefaultAllocator<tVec4f>> classDesc(vm.fSq());
				classDesc
					.StaticFunc(_SC("Construct"), &tVec4f::fConstruct)
					.Var(_SC("x"), &tVec4f::x)
					.Var(_SC("y"), &tVec4f::y)
					.Var(_SC("z"), &tVec4f::z)
					.Var(_SC("w"), &tVec4f::w);
				fExportVecCommon(tVec4f, classDesc);
				vm.fNamespace(_SC("Math")).Bind(_SC("Vec4"), classDesc);
			}
			static void fExportEulerAngles(tScriptVm &vm)
			{
				Sqrat::Class<tEulerAnglesf, Sqrat::DefaultAllocator<tEulerAnglesf>> classDesc(vm.fSq());
				classDesc
					.StaticFunc(_SC("Construct"), &tEulerAnglesf::fConstruct)
					.Var(_SC("x"), &tEulerAnglesf::x)
					.Var(_SC("y"), &tEulerAnglesf::y)
					.Var(_SC("z"), &tEulerAnglesf::z);
				fExportVecCommon(tEulerAnglesf, classDesc);
				vm.fNamespace(_SC("Math")).Bind(_SC("EulerAngles"), classDesc);
			}
			static tMat3f cIdentityMat3 = tMat3f::cIdentity;
			static tMat3f *fIdentityMat3()
			{
				return &cIdentityMat3;
			}
			static void fExportMat3(tScriptVm &vm)
			{
				Sqrat::Class<tMat3f, Sqrat::DefaultAllocator<tMat3f>> classDesc(vm.fSq());
				classDesc
					.Prop(_SC("Translation"), &tMat3f::fGetTranslation, &tMat3f::fSetTranslation)
					.Prop(_SC("Determinant"), &tMat3f::fDeterminant)
					.StaticFunc(_SC("Identity"), &fIdentityMat3);
				vm.fNamespace(_SC("Math")).Bind(_SC("Mat3"), classDesc);
			}
			static tMat4f cIdentityMat4 = tMat4f::cIdentity;
			static tMat4f *fIdentityMat4()
			{
				return &cIdentityMat4;
			}
			static void fExportMat4(tScriptVm &vm)
			{
				Sqrat::Class<tMat4f, Sqrat::DefaultAllocator<tMat4f>> classDesc(vm.fSq());
				classDesc
					.Prop(_SC("Translation"), &tMat4f::fGetTranslation, &tMat4f::fSetTranslation)
					.Prop(_SC("Determinant"), &tMat4f::fDeterminant)
					.StaticFunc(_SC("Identity"), &fIdentityMat4);
				vm.fNamespace(_SC("Math")).Bind(_SC("Mat4"), classDesc);
			}
			static void fExportRect(tScriptVm &vm)
			{
				Sqrat::Class<tRect, Sqrat::DefaultAllocator<tRect>> classDesc(vm.fSq());

				classDesc
					.StaticFunc(_SC("Construct"), &tRect::fConstruct)
					.StaticFunc(_SC("ConstructEx"), &tRect::fConstructEx)
					.Prop(_SC("Width"), &tRect::fWidth)
					.Prop(_SC("Height"), &tRect::fHeight)
					.Prop(_SC("WidthHeight"), &tRect::fWidthHeight)
					.Func(_SC("Barycentric"), &tRect::fBarycentric)
					.Prop(_SC("Center"), &tRect::fCenter)
					.Prop(_SC("AspectRatio"), &tRect::fAspectRatio)
					.Prop(_SC("TopLeft"), &tRect::fTopLeft)
					.Prop(_SC("TopRight"), &tRect::fTopRight)
					.Prop(_SC("BottomLeft"), &tRect::fBottomLeft)
					.Prop(_SC("BottomRight"), &tRect::fBottomRight)
					.Var(_SC("Top"), &tRect::mT)
					.Var(_SC("Left"), &tRect::mL)
					.Var(_SC("Bottom"), &tRect::mB)
					.Var(_SC("Right"), &tRect::mR)
					.Func(_SC("Contains"), &tRect::fContains);

				vm.fNamespace(_SC("Math")).Bind(_SC("Rect"), classDesc);
			}
			static void fExportSphere(tScriptVm &vm)
			{
				Sqrat::Class<tSpheref, Sqrat::DefaultAllocator<tSpheref>> classDesc(vm.fSq());
				classDesc
					.StaticFunc(_SC("Construct"), &tSpheref::fConstruct);
				vm.fNamespace(_SC("Math")).Bind(_SC("Sphere"), classDesc);
			}

			static b32 fAABBFContains(tAabbf *owner, const Math::tVec3f &pt)
			{
				return owner->fContains(pt);
			}
			static void fExportAabb(tScriptVm &vm)
			{
				Sqrat::Class<tAabbf, Sqrat::DefaultAllocator<tAabbf>> classDesc(vm.fSq());
				classDesc
					.StaticFunc(_SC("Construct"), &tAabbf::fConstruct)
					.Prop(_SC("ComputeCenter"), &tAabbf::fComputeCenter)
					.Prop(_SC("ComputeDiagonal"), &tAabbf::fComputeDiagonal)
					.GlobalFunc(_SC("Contains"), &fAABBFContains);
				vm.fNamespace(_SC("Math")).Bind(_SC("Aabb"), classDesc);
			}
			static void fExportObb(tScriptVm &vm)
			{
				Sqrat::Class<tObbf, Sqrat::DefaultAllocator<tObbf>> classDesc(vm.fSq());
				classDesc
					.StaticFunc(_SC("Construct"), &tObbf::fConstruct)
					.Func(_SC("ToAabb"), &tObbf::fToAabb);
				vm.fNamespace(_SC("Math")).Bind(_SC("Obb"), classDesc);
			}
			static f32 fMaxFloat(f32 a, f32 b)
			{
				return fMax(a, b);
			}
			static f32 fMinFloat(f32 a, f32 b)
			{
				return fMin(a, b);
			}
			static f32 fClampFloat(f32 toClamp, f32 min, f32 max)
			{
				return fClamp(toClamp, min, max);
			}
			static f32 fWrapFloat(f32 toWrap, f32 min, f32 max)
			{
				return fWrap(toWrap, min, max);
			}
			static u32 fQuantizeCircle(f32 angle, u32 numBuckets)
			{
				angle = fWrap(angle, 0.f, Math::c2Pi);
				const f32 dtheta = Math::c2Pi / numBuckets;
				const f32 intAngle = fRound<f32>(angle / dtheta);
				return (u32)intAngle % numBuckets;
			}
		}
		void fExportScriptInterface(tScriptVm &vm)
		{
			fExportVec2(vm);
			fExportVec3(vm);
			fExportVec4(vm);
			fExportEulerAngles(vm);
			fExportMat3(vm);
			fExportMat4(vm);
			fExportRect(vm);
			fExportSphere(vm);
			fExportAabb(vm);
			fExportObb(vm);

			vm.fNamespace(_SC("Math"))
				.Func(_SC("Max"), &fMaxFloat)
				.Func(_SC("Min"), &fMinFloat)
				.Func(_SC("Abs"), &fAbs<f32>)
				.Func(_SC("Clamp"), &fClampFloat)
				.Func(_SC("Wrap"), &fWrapFloat)
				.Func(_SC("Sin"), &fSin<f32>)
				.Func(_SC("Cos"), &fCos<f32>)
				.Func(_SC("Tan"), &fTan<f32>)
				.Func(_SC("QuantizeCircle"), &fQuantizeCircle)
				.Func<f32 (*)(const f32 &, const f32 &, const f32 &)>(_SC("Lerp"), &Math::fLerp<f32, f32>)
				.Func<Math::tVec2f (*)(const Math::tVec2f &, const Math::tVec2f &, const f32 &)>(_SC("Lerp2"), &Math::fLerp<Math::tVec2f, f32>)
				.Func<Math::tVec3f (*)(const Math::tVec3f &, const Math::tVec3f &, const f32 &)>(_SC("Lerp3"), &Math::fLerp<Math::tVec3f, f32>)
				.Func<Math::tVec4f (*)(const Math::tVec4f &, const Math::tVec4f &, const f32 &)>(_SC("Lerp4"), &Math::fLerp<Math::tVec4f, f32>)
				.Func<f32 (*)(const f32 &, const f32 &, const f32 &)>(_SC("Remap"), &Math::fRemapZeroToOne<f32>)
				.Func<f32 (*)(const f32 &, const f32 &)>(_SC("RemapMinimum"), &Math::fRemapMinimum<f32>)
				.Func<f32 (*)(const f32)>(_SC("Round"), &fRound<f32>)
				.Func<f32 (*)(const f32)>(_SC("RoundUp"), &fRoundUp<f32>)
				.Func<f32 (*)(const f32)>(_SC("RoundDown"), &fRoundDown<f32>)
				.Func<b32 (*)(s32)>(_SC("IsOdd"), &fIsOdd<s32>)
				.Func<b32 (*)(s32)>(_SC("IsEven"), &fIsEven<s32>)
				.Func(_SC("Pow"), &fPow<f32>);

			vm.fConstTable().Const(_SC("MATH_INFINITY"), cInfinity);
			vm.fConstTable().Const(_SC("MATH_PI"), cPi);
			vm.fConstTable().Const(_SC("MATH_2_PI"), c2Pi);
			vm.fConstTable().Const(_SC("MATH_PI_OVER_2"), cPiOver2);
			vm.fConstTable().Const(_SC("MATH_PI_OVER_4"), cPiOver4);
			vm.fConstTable().Const(_SC("MATH_PI_OVER_8"), cPiOver8);
			vm.fConstTable().Const(_SC("MATH_3_PI_OVER_2"), c3PiOver2);
		}
	}
}
