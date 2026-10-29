#include <Research/NativeProbe/Probe.h>
#include <Research/HudCaptain/HudCaptain.h>
#include <Research/HudAssembly/HudAssembly.h>
#include <d3d.h>
#include <tuple>
#include <intrin.h>
#include <Research/FlightRecorder/Hooks.h>
#pragma once

#include <string>
#include <typeinfo>
#include <type_traits>

#include <Common/Hook.h>
#include <Common/Log.h>

template <typename Interface>
using Vtable = typename std::remove_pointer<decltype(Interface::lpVtbl)>::type;

template <typename Vtable>
class CompatVtable;

template <typename Vtable, typename Interface>
const Vtable& getOrigVtable(Interface* /*This*/)
{
	return CompatVtable<Vtable>::s_origVtable;
}

template <>
inline const IUnknownVtbl& getOrigVtable(IUnknown* This)
{
	return *This->lpVtbl;
}

template <typename Interface>
const Vtable<Interface>& getOrigVtable(Interface* This)
{
	return getOrigVtable<Vtable<Interface>, Interface>(This);
}

template <typename Vtable>
const Vtable& getOrigVtable(HANDLE);

namespace
{
	template <typename Vtable>
	constexpr void setCompatVtable(Vtable&);
}

template <typename Vtable, auto memberPtr, typename Interface, typename... Params>
auto __stdcall callOrigFunc(Interface This, Params... params)
{
	return (getOrigVtable<Vtable>(This).*memberPtr)(This, params...);
}

template <typename Vtable>
constexpr auto getCompatVtable()
{
	Vtable vtable = {};
	setCompatVtable(vtable);
	return vtable;
}

template <auto memberPtr, typename Vtable>
constexpr auto getCompatFunc(Vtable*)
{
	auto func = getCompatVtable<Vtable>().*memberPtr;
	if (!func)
	{
		func = &callOrigFunc<Vtable, memberPtr>;
	}
	return func;
}

template <auto memberPtr, typename Vtable>
constexpr auto getCompatFunc()
{
	return getCompatFunc<memberPtr>(static_cast<Vtable*>(nullptr));
}

template <typename Vtable, typename Lock>
class VtableHookVisitor
{
public:
	VtableHookVisitor(const Vtable& vtable)
		: m_vtable(const_cast<Vtable&>(vtable))
	{
	}

	template <auto memberPtr>
	void visit([[maybe_unused]] const char* funcName)
	{
		if constexpr (!(getCompatVtable<Vtable>().*memberPtr))
		{
			if (Compat::Log::getLogLevel() < Config::Settings::LogLevel::DEBUG && !Qfr::requested())
			{
				return;
			}
		}

		if (m_vtable.*memberPtr)
		{
			s_funcName<memberPtr> = s_vtableTypeName + "::" + funcName;
			LOG_DEBUG << "Hooking function: " << s_funcName<memberPtr>
				<< " (" << Compat::funcPtrToStr(m_vtable.*memberPtr) << ')';
			m_vtable.*memberPtr = &hookFunc<memberPtr>;
		}
	}

private:
	template <auto memberPtr, typename Result, typename FirstParam, typename... Params>
	static Result STDMETHODCALLTYPE hookFunc(FirstParam firstParam, Params... params)
	{
		Qfr::Call qfrCall("api",s_funcName<memberPtr>.c_str(),static_cast<uintptr_t>(Qfr::number(firstParam)),[&]{return Qfr::parameters(params...);});
		LOG_FUNC(s_funcName<memberPtr>.c_str(), firstParam, params...);
		NativeProbe::ApiScope nativeScope(s_funcName<memberPtr>.c_str(),static_cast<uintptr_t>(Qfr::number(firstParam)),reinterpret_cast<uintptr_t>(_ReturnAddress()));
		nativeScope.arguments(params...);
		[[maybe_unused]] Lock lock;
		constexpr auto compatFunc = getCompatFunc<memberPtr, Vtable>();
		if constexpr (std::is_same_v<decltype(memberPtr),decltype(&IDirect3DViewport3Vtbl::Clear2)>)
		{
			if constexpr (memberPtr == &IDirect3DViewport3Vtbl::Clear2)
			{
				auto args=std::make_tuple(params...);
				HudAssembly::Clear assembly(reinterpret_cast<uintptr_t>(_ReturnAddress()),reinterpret_cast<uintptr_t>(firstParam),std::get<0>(args),std::get<1>(args),std::get<2>(args),std::get<3>(args),std::get<4>(args),std::get<5>(args));
				HudCaptain::StripClear adapted(reinterpret_cast<uintptr_t>(_ReturnAddress()),reinterpret_cast<uintptr_t>(firstParam),std::get<0>(args),assembly.data(std::get<1>(args)),std::get<2>(args),std::get<3>(args),std::get<4>(args),std::get<5>(args));
				const auto result=compatFunc(firstParam,std::get<0>(args),const_cast<D3DRECT*>(adapted.data(assembly.data(std::get<1>(args)))),std::get<2>(args),std::get<3>(args),std::get<4>(args),std::get<5>(args));
				assembly.result(Qfr::number(result));
				adapted.result(Qfr::number(result));nativeScope.result(Qfr::number(result));
				qfrCall.result(Qfr::number(result),[&]{return Qfr::parameters(params...);});return LOG_RESULT(result);
			}
		}
		if constexpr (std::is_same_v<decltype(memberPtr),decltype(&IDirect3DDevice3Vtbl::DrawPrimitive)>)
		{
			if constexpr (memberPtr == &IDirect3DDevice3Vtbl::DrawPrimitive)
			{
				auto args=std::make_tuple(params...);
				HudAssembly::Draw assembly(reinterpret_cast<uintptr_t>(_ReturnAddress()),std::get<1>(args),std::get<2>(args),std::get<3>(args));
				HudCaptain::Draw adapted(std::get<1>(args),assembly.data(std::get<2>(args)),std::get<3>(args));
				const auto result=compatFunc(firstParam,std::get<0>(args),std::get<1>(args),adapted.data(assembly.data(std::get<2>(args))),std::get<3>(args),std::get<4>(args));
				assembly.result(Qfr::number(result));
				adapted.result(Qfr::number(result));nativeScope.result(Qfr::number(result));
				qfrCall.result(Qfr::number(result),[&]{return Qfr::parameters(params...);});return LOG_RESULT(result);
			}
		}
		if constexpr (std::is_same_v<decltype(memberPtr),decltype(&IDirect3DDevice3Vtbl::DrawIndexedPrimitive)>)
		{
			if constexpr (memberPtr == &IDirect3DDevice3Vtbl::DrawIndexedPrimitive)
			{
				auto args=std::make_tuple(params...);
				HudAssembly::Draw assembly(reinterpret_cast<uintptr_t>(_ReturnAddress()),std::get<1>(args),std::get<2>(args),std::get<3>(args));
				const auto result=compatFunc(firstParam,std::get<0>(args),std::get<1>(args),assembly.data(std::get<2>(args)),std::get<3>(args),std::get<4>(args),std::get<5>(args),std::get<6>(args));
				assembly.result(Qfr::number(result));nativeScope.result(Qfr::number(result));
				qfrCall.result(Qfr::number(result),[&]{return Qfr::parameters(params...);});return LOG_RESULT(result);
			}
		}
		if constexpr (std::is_void_v<Result>)
		{
			compatFunc(firstParam, params...);
		}
		else
		{
			const auto qfrResult=compatFunc(firstParam, params...);
			nativeScope.result(Qfr::number(qfrResult));
			qfrCall.result(Qfr::number(qfrResult),[&]{return Qfr::parameters(params...);});
			return LOG_RESULT(qfrResult);
		}
	}

	Vtable& m_vtable;

	template <auto memberPtr>
	static std::string s_funcName;

	static std::string s_vtableTypeName;
};

template <typename Vtable, typename Lock>
template <auto memberPtr>
std::string VtableHookVisitor<Vtable, Lock>::s_funcName;

template <typename Vtable, typename Lock>
std::string VtableHookVisitor<Vtable, Lock>::s_vtableTypeName(Compat::getTypeName<Vtable>());
