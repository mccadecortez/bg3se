#pragma once

#include <GameDefinitions/Enumerations.h>
#include <GameDefinitions/Osiris.h>
#include "Wrappers.h"
#include <detours.h>
#include <thread>

namespace bg3se {

uint8_t * ResolveRealFunctionAddress(uint8_t * Address);

#define PREHOOKABLE(ty, name) enum class name##Tag {}; \
	PreHookableFunction<name##Tag, ty> name

#define POSTHOOKABLE(ty, name) enum class name##Tag {}; \
	PostHookableFunction<name##Tag, ty> name

#define HOOKABLE(ty, name) enum class name##Tag {}; \
	HookableFunction<name##Tag, ty> name

#define WRAPPABLE(ty, name) enum class name##Tag {}; \
	WrappableFunction<name##Tag, ty> name

#define FASTWRAPPABLE(ty, name) enum class name##Tag {}; \
	FastWrappableFunction<name##Tag, ty> name

class OsirisWrappers
{
public:
	OsirisWrappers();

	void Initialize();
	void Shutdown();
	bool ResolveNodeVMTs();

	OsirisStaticGlobals Globals;

	void * OsirisDllStart{ nullptr };
	uint32_t OsirisDllSize{ 0 };

	HOOKABLE(int(void *, DivFunctions *), RegisterDivFunctions);
	PREHOOKABLE(int(void *), InitGame);
	PREHOOKABLE(int(void *, bool), DeleteAllData);
	POSTHOOKABLE(void(void *, MappingInfo **, uint32_t *), GetFunctionMappings);

	PREHOOKABLE(void(void * Osiris, wchar_t const * Path, std::ios_base::openmode Mode), OpenLogFile);
	PREHOOKABLE(void(void * Osiris), CloseLogFile);

	WRAPPABLE(bool(void *, wchar_t const *, wchar_t const *), Compile);
	HOOKABLE(int(void *, void *), Load);
	WRAPPABLE(bool(void *, wchar_t *), Merge);
	HOOKABLE(ReturnCode(void *, uint32_t, OsiArgumentDesc *), Event);

	RuleActionCallProc OriginalRuleActionCallProc;
	WRAPPABLE(void(RuleActionNode *, void *, void *, void *, void *), RuleActionCall);

	FASTWRAPPABLE(bool(uint32_t FunctionHandle, OsiArgumentDesc * Params), Call);
	FASTWRAPPABLE(bool(uint32_t FunctionHandle, OsiArgumentDesc * Params), Query);
	PREHOOKABLE(void(char const * Message), Error);
	PREHOOKABLE(void(bool Successful, char const * Message, bool Unknown2), Assert);

	POSTHOOKABLE(HANDLE(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE), CreateFileW);
	POSTHOOKABLE(BOOL(HANDLE), CloseHandle);

	DivFunctions::CallProc CallOriginal;
	DivFunctions::CallProc QueryOriginal;
	DivFunctions::ErrorMessageProc ErrorOriginal;
	DivFunctions::AssertProc AssertOriginal;

	NodeVMT* VMTs[(unsigned)NodeType::Max + 1];

private:
	HMODULE Kernel32Module{ NULL };
	HMODULE OsirisModule{ NULL };

	std::optional<bool> resolvedVMTs_;

	void * FindRuleActionCallProc();
	void FindOsirisGlobals(FARPROC CtorProc);
	void FindDebugFlags(FARPROC SetOptionProc);
	void RegisterDIVFunctionsPreHook(void * Osiris, DivFunctions * Functions);
	bool ResolveNodeVMTsInternal();

	void SaveNodeVMT(NodeType type, NodeVMT* vmt);

	static bool CallWrapper(uint32_t FunctionHandle, OsiArgumentDesc * Params);
	static bool QueryWrapper(uint32_t FunctionHandle, OsiArgumentDesc * Params);
	static void ErrorWrapper(char const * Message);
	static void AssertWrapper(bool Successful, char const * Message, bool Unknown2);
};

}