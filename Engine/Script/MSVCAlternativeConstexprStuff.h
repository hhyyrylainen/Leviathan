// Alternative implementations of the complex constexpr stuff in ScriptExecutor
// Because Microsoft can't fix their compiler to not choke on some constexpr if expressions
// NOTE: IF THIS OR THE GCC / NON-MSVC CODE IS CHANGED THE OTHER MUST ALSO BE CHANGED!

template<bool convertible>
struct _MSVCHelperDoConv {
	static bool f() {
		static_assert(convertible == false && convertible == true);
	}
};

template<>
struct _MSVCHelperDoConv<true> {
	template<class T, class CurrentT, class... Args>
	static bool f(ScriptExecutor* exec, asUINT parameterc, asUINT& i, asIScriptContext* scriptcontext,
		ScriptRunningSetup& setup, ScriptModule* module, asIScriptFunction* func,
		const CurrentT& current, Args&&... args)
	{
		return exec->_DoPassEachParameter(parameterc, i, scriptcontext, setup, module,
			func, static_cast<T>(current), std::forward<Args>(args)...);
	}
};

template<>
struct _MSVCHelperDoConv<false> {
	template<class T, class CurrentT, class... Args>
	static bool f(ScriptExecutor* exec, asUINT parameterc, asUINT& i, asIScriptContext* scriptcontext,
		ScriptRunningSetup& setup, ScriptModule* module, asIScriptFunction* func,
		const CurrentT& current, Args&&... args)
	{
		LOG_FATAL("Shouldn't get here");
		return false;
	}
};
