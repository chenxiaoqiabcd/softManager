#include "track.h"

#include "log_helper.h"

#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp")

void DumpCallStack(LPCONTEXT context, HANDLE process_handle) {
	STACKFRAME stack_frame;
	memset(&stack_frame, 0, sizeof(stack_frame));

	stack_frame.AddrPC.Offset = context->Eip;
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = context->Esp;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = context->Ebp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;

	while(true) {
		if(!StackWalk(IMAGE_FILE_MACHINE_I386, process_handle, GetCurrentThread(), &stack_frame, context,
					  nullptr, SymFunctionTableAccess, SymGetModuleBase, 0)) {
			break;
		}

		if(stack_frame.AddrFrame.Offset == 0) {
			break;
		}

		BYTE symbol_buffer[sizeof(SYMBOL_INFO) + 1024];
		PSYMBOL_INFO symbol_info = (PSYMBOL_INFO)symbol_buffer;

		symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol_info->MaxNameLen = 1024;

		if(SymFromAddr(process_handle, stack_frame.AddrPC.Offset, 0, symbol_info)) {
			KF_INFO("function: %s", symbol_info->Name);
		}
		else {
			KF_ERROR("sym from add failed");
		}

		IMAGEHLP_LINE line_info = { sizeof(IMAGEHLP_LINE) };
		DWORD dw_line_displacement = 0;

		if(SymGetLineFromAddr(process_handle, stack_frame.AddrPC.Offset, &dw_line_displacement, &line_info)) {
			KF_INFO("file: %s, line: %d", line_info.FileName, line_info.LineNumber);
		}
		else {
			KF_ERROR("sym get line from addr failed");
		}
 	}
}

DWORD ExceptionFilter(LPEXCEPTION_POINTERS exception_pointers) {
	HANDLE process_handle = GetCurrentProcess();

	if(!SymInitialize(process_handle, nullptr, TRUE)) {
		KF_ERROR("failed init Sym");
		return 0;
	}

	DumpCallStack(exception_pointers->ContextRecord, process_handle);

	SymCleanup(process_handle);

	return 0;
}
