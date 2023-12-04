// WindowSizing.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <stdio.h>
#include <conio.h>
#include <Windows.h>

#include <string>


std::string GetFolderBasePath();
char* LoadExistingFile(const char* szFileName);
void PrintLastErrorMessage();

HWND GetTheLastChildWindowHandleFromClassName(const wchar_t* szClassName);
void PrintWindowInfo(HWND hWnd);

bool SaveWindowSize(const char* szFileName, RECT rect);
bool LoadWindowSize(const char* szFileName, RECT& rect);
bool ReadWindowSize(char* src, RECT& rect);

void AdjustWindow(HWND hWnd, RECT rect);

int main()
{
	char key{};
	for (; !_kbhit();)
	{
		printf("Saves/Loads the last child Microsoft Edge window position.\r\n");
		printf("  s: Save...\r\n  l: Load...\r\n");
		key = _getch();
		if (key == 's' || key == 'l')break;
	}

	/*Microsoft Edgeのクラス名*/
	HWND hWnd = GetTheLastChildWindowHandleFromClassName(L"Chrome_WidgetWin_1");
	if (hWnd != nullptr)
	{
		RECT rect{};
		std::string strFile = GetFolderBasePath() + "rect.txt";

		if (key == 's')
		{
			BOOL iRet = ::GetWindowRect(hWnd, &rect);
			if (iRet)
			{
				if (!SaveWindowSize(strFile.c_str(), rect))
				{
					printf_s("Failed to save file.\r\n");
					PrintLastErrorMessage();
				}
			}
			else
			{
				printf_s("Failed to get position.\r\n");
				PrintLastErrorMessage();
			}
		}
		else
		{
			if (LoadWindowSize(strFile.c_str(), rect))
			{
				AdjustWindow(hWnd, rect);
			}
			else
			{
				printf_s("Failed to load file.\r\n");
				PrintLastErrorMessage();
			}
		}

	}
	else
	{
		printf_s("Cannot find window.\r\n");
		PrintLastErrorMessage();
	}

    return 0;
}

/*実行プロセスの階層取得*/
std::string GetFolderBasePath()
{
    char application_path[MAX_PATH]{};
    GetModuleFileNameA(nullptr, application_path, MAX_PATH);
    std::string::size_type pos = std::string(application_path).find_last_of("\\/");
    return std::string(application_path).substr(0, pos) + "\\/";
}
/*ファイルのメモリ展開*/
char* LoadExistingFile(const char* szFileName)
{
	HANDLE hFile = ::CreateFileA(szFileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = ::GetFileSize(hFile, nullptr);
		if (dwSize != INVALID_FILE_SIZE)
		{
			char* buffer = static_cast<char*>(malloc(static_cast<size_t>(dwSize + 1ULL)));
			if (buffer != nullptr)
			{
				DWORD dwRead = 0;
				BOOL iRet = ::ReadFile(hFile, buffer, dwSize, &dwRead, nullptr);
				if (iRet)
				{
					::CloseHandle(hFile);
					*(buffer + dwRead) = '\0';
					return buffer;
				}
				else
				{
					free(buffer);
				}
			}
		}
		::CloseHandle(hFile);
	}

	return nullptr;
}
/*最終エラーコード内容の表示*/
void PrintLastErrorMessage()
{
	DWORD dwCode = ::GetLastError();
	char* pMsg = nullptr;

	::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, dwCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&pMsg, 0, nullptr);

	printf("LastError code: %d, message: %s\r\n", dwCode, pMsg);

	::LocalFree(pMsg);

	system("pause");
}

/*最後の子ウィンドウを探索*/
HWND GetTheLastChildWindowHandleFromClassName(const wchar_t* szClassName)
{
	HWND hWnd = nullptr;

	for (;;)
	{
		hWnd = ::FindWindowEx(0, hWnd, szClassName, 0);
		if (!hWnd) break;
		if (!::IsWindowVisible(hWnd)) continue;

		return hWnd;
	}

	return nullptr;
}
/*ウィンドウ情報表示*/
void PrintWindowInfo(HWND hWnd)
{
	if (hWnd != nullptr)
	{
		RECT rect{};
		BOOL iRet = ::GetWindowRect(hWnd, &rect);
		if (iRet)
		{
			int iLength = ::GetWindowTextLength(hWnd);
			char* buffer = static_cast<char*>(malloc(iLength + 1LL));
			if (buffer != nullptr)
			{
				::GetWindowTextA(hWnd, buffer, iLength);
				printf_s("Caption: %s, Left: %ld, Top: %ld, Right: %ld, Bottom:%ld\r\n", buffer, rect.left, rect.top, rect.right, rect.bottom);
				free(buffer);
			}
		}
	}

}

/*RECT構造体保存*/
bool SaveWindowSize(const char* szFileName, RECT rect)
{
	if (szFileName != nullptr)
	{
		HANDLE hFile = ::CreateFileA(szFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::SetFilePointer(hFile, NULL, nullptr, FILE_END);
			char buffer[256]{};
			sprintf_s(buffer, "Left:%ld, Top:%ld, Right:%ld, Bottom:%ld;\r\n", rect.left, rect.top, rect.right, rect.bottom);
			DWORD bytesWrite = 0;
			BOOL bRet = ::WriteFile(hFile, buffer, static_cast<DWORD>(strlen(buffer)), &bytesWrite, nullptr);
			if (!bRet)
			{
				::CloseHandle(hFile);
				return false;
			}
			::CloseHandle(hFile);
			return true;
		}
	}
	return false;
}
/*RECT保存ファイル読み込み*/
bool LoadWindowSize(const char* szFileName, RECT& rect)
{
	bool bRet = false;

	char* buffer = LoadExistingFile(szFileName);
	if (buffer != nullptr)
	{
		bRet = ReadWindowSize(buffer, rect);
		free(buffer);
	}

	return bRet;
}
/*RECT復元*/
bool ReadWindowSize(char* src, RECT& rect)
{
	char* p = nullptr;
	char* pp = src;
	size_t len = 0;
	char buffer[128]{};

	int iCount = 0;
	for (;; ++iCount)
	{
		p = strstr(pp, "Left:");
		if (p == nullptr)break;
		p += strlen("Left:");
		pp = strstr(p, ",");
		if (pp == nullptr)break;
		len = pp - p;
		if (len > sizeof(buffer))break;
		memcpy(buffer, p, len);
		*(buffer + len) = '\0';
		rect.left = atol(buffer);

		p = strstr(pp, "Top:");
		if (p == nullptr)break;
		p += strlen("Top:");
		pp = strstr(p, ",");
		if (pp == nullptr)break;
		len = pp - p;
		if (len > sizeof(buffer))break;
		memcpy(buffer, p, len);
		*(buffer + len) = '\0';
		rect.top = atol(buffer);

		p = strstr(pp, "Right:");
		if (p == nullptr)break;
		p += strlen("Right:");
		pp = strstr(p, ",");
		if (pp == nullptr)break;
		len = pp - p;
		if (len > sizeof(buffer))break;
		memcpy(buffer, p, len);
		*(buffer + len) = '\0';
		rect.right = atol(buffer);

		p = strstr(pp, "Bottom:");
		if (p == nullptr)break;
		p += strlen("Bottom:");
		pp = strstr(p, ";");
		if (pp == nullptr)break;
		len = pp - p;
		if (len > sizeof(buffer))break;
		memcpy(buffer, p, len);
		*(buffer + len) = '\0';
		rect.bottom = atol(buffer);

	}

	return iCount > 0;
}
/*指定ウィンドウの移動*/
void AdjustWindow(HWND hWnd, RECT rect)
{
    ::MoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}