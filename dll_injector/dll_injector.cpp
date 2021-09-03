// dll_injector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include<string>

using namespace std;

bool inject_method_CreateRemoteThread(DWORD process_id, string dll_path)
{
  // 1. open the target process

  HANDLE hp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);

  // 2. allocate a memory block in the target process to store dll path

  auto ptr_store_dll_path = VirtualAllocEx(hp, nullptr, dll_path.length(), MEM_COMMIT, PAGE_READWRITE);

  // 3. write dll path to allocated memory in the target process

  SIZE_T written_bytes = 0;
  WriteProcessMemory(hp, ptr_store_dll_path, dll_path.c_str(), dll_path.length(), &written_bytes);

  // 4. get address of the function LoadLibraryA(...) in the target process

  auto hmodule = GetModuleHandleA("kernel32.dll");
  auto fn_LoadLibraryA = GetProcAddress(hmodule, "LoadLibraryA");
  // this can be ignored, cuz a system dll is almost the same base address in all processes
  // so in this case we can use direct `LoadLibraryA` instead of get its address

  // 5. create a remote thread to execute the function LoadLibraryA(...)

  auto thread = CreateRemoteThread(hp, nullptr, 0, LPTHREAD_START_ROUTINE(fn_LoadLibraryA), ptr_store_dll_path, 0, nullptr);
  WaitForSingleObject(thread, INFINITE);

  // 6. free allocated memory at step 2

  VirtualFreeEx(hp, ptr_store_dll_path, dll_path.length(), MEM_RELEASE);

  // 7. finish and close the target process

  CloseHandle(hp);

  return true;
}


int main()
{
  // 1. process id of the target
	string process_name;
	cout << " Input process name you want to inject dll (Example: notepad, zoom, chorme, ...) \n";
	cin >> process_name;
  HWND hwnd = FindWindowA(process_name.c_str(), nullptr);

  DWORD process_id = 0;
  GetWindowThreadProcessId(hwnd, &process_id);

  cout << "Process ID = " << process_id << endl;

  // 2. dll path to inject
  string get_dll_path;
	cout << " Input your dll path \n";
	cin >> get_dll_path;

	const string dll_path = get_dll_path.c_str();

  // 3. go to inject here

  bool succeed = inject_method_CreateRemoteThread(process_id, dll_path);
  // bool succeed = inject_method_WindowMessageHooking(process_id, dll_path);

  cout << (succeed ? "Inject -> SUCCEED" : "Inject -> FAILED") << endl;
  system("pause");
	
  return 0;
}