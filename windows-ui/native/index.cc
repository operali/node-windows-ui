#include <node.h>

using namespace std;
using v8::Boolean;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

#ifndef WIN32
#ifdef _WIN32
#define WIN32 1
#endif /* _WIN32 */
#endif /* WIN32 */
#ifdef WIN32
#include <windows.h>
#include <shlobj_core.h>
#include <sstream>
#include <iostream>
#define HAVE_MMAP 1
#define HAVE_MORECORE 0
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#define MALLOC_FAILURE_ACTION
#define MMAP_CLEARS 0 /* WINCE and some others apparently don't clear */

std::string replace_all(std::string str, std::string old_value, std::string new_value)
{
    while (true)
    {
        std::string::size_type pos(0);
        if ((pos = str.find(old_value)) != std::string::npos)
        {
            str.replace(pos, old_value.length(), new_value);
        }
        else
        {
            break;
        }
    }
    return str;
}

std::string selectFile(bool openOrSave, std::string defaultPath, std::string filter)
{
    defaultPath = replace_all(defaultPath, "/", "\\");
    printf("openOrSave:%d, defaultPath:%s, filter:%s", openOrSave, defaultPath.c_str(), filter.c_str());
    std::string path(MAX_PATH, 0);
    OPENFILENAME ofn = {sizeof(OPENFILENAME)};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr; //父窗口句柄

    std::string opfilter(MAX_PATH, 0);
    std::string strFiles = "files(" + filter + ")";
    int len = 0;
    memcpy(&opfilter[0], &strFiles[0], strFiles.length());
    len += strFiles.length();
    memset(&opfilter[0] + len, '\0', 1);
    len += 1;
    memcpy(&opfilter[0] + len, &filter[0], filter.length());
    len += filter.length();
    memset(&opfilter[0] + len, '\0', 1);
    len += 1;
    const char *strAll = "all files(*.*)";
    memcpy(&opfilter[0] + len, strAll, strlen(strAll));
    len += strlen(strAll);
    memset(&opfilter[0] + len, '\0', 1);
    len += 1;
    const char *allType = "*.*";
    memcpy(&opfilter[0] + len, allType, strlen(allType));
    len += strlen(allType);
    memset(&opfilter[0] + len, '\0', 1);
    len += 1;
    ofn.lpstrFilter = &opfilter[0]; //打开的文件类型，这里以xml和所有文件为例
    ofn.lpstrFile = &path[0];
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = ".*";                    //默认的打开的文件类型
    ofn.lpstrInitialDir = defaultPath.c_str(); //默认的打开的文件路径，这里以当前目录为例
    if (openOrSave)
    {
        if (GetOpenFileName(&ofn))
        {
            return path;
        }
    }
    else
    {
        if (GetSaveFileName(&ofn))
        {
            return path;
        }
    }

    return "";
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    LPITEMIDLIST pidlNavigate;
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
    {
        pidlNavigate = (LPITEMIDLIST)lpData;
        if (pidlNavigate != NULL)
            SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)FALSE, (LPARAM)pidlNavigate);
    }
    break;
    }
    return 0;
}

std::string BrowseForFolder(std::string title, std::string folder)
{
    folder = replace_all(folder, "/", "\\");
    BROWSEINFOA bi = {0};
    LPITEMIDLIST pidlStart, pidlSelected;
    pidlStart = ILCreateFromPathA(folder.c_str());
    bi.hwndOwner = nullptr;
    bi.pszDisplayName = 0;
    bi.lpszTitle = title.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)pidlStart;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    char DIRPATH[MAX_PATH];
    if (pidl != 0)
    {
        //get the name of the folder and put it in path
        SHGetPathFromIDList(pidl, DIRPATH);

        //free memory used
        IMalloc *imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }

        return DIRPATH;
    }
    return "";
}

void v8MessageBox(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    Local<String> type = Local<String>::Cast(args[0]);
    v8::String::Value valueType(isolate, type);

    Local<String> title = Local<String>::Cast(args[1]);
    v8::String::Value valTitle(isolate, title);
    Local<String> info = Local<String>::Cast(args[2]);
    v8::String::Value valInfo(isolate, info);
    Local<Function> cb = Local<Function>::Cast(args[3]);
    int result = 1;
    if (std::wstring(reinterpret_cast<wchar_t *>(*valueType)) == L"OKCANCEL")
    {
        result = ::MessageBoxW(nullptr, reinterpret_cast<LPCWSTR>(*valInfo), reinterpret_cast<LPCWSTR>(*valTitle), MB_OKCANCEL | MB_DEFBUTTON2);
    }
    else
    {
        result = ::MessageBoxW(nullptr, reinterpret_cast<LPCWSTR>(*valInfo), reinterpret_cast<LPCWSTR>(*valTitle), 0);
    }
    bool bRes = result == 1;

    const unsigned argc = 1;
    Local<Value> argv[argc] = {
        Boolean::New(isolate, bRes)};
    cb->Call(context, Null(isolate), argc, argv).ToLocalChecked();
}

std::wstring s2ws(const std::string &s)
{
    /*using namespace boost::locale::conv;
	wstring wide_string = to_utf<wchar_t>(s, "utf8");
	return wide_string;*/

    int requiredSize = MultiByteToWideChar(CP_ACP,
                                           0, s.c_str(), s.length(), 0, 0);

    if (!requiredSize)
    {
        return 0;
    }

    wchar_t *w = new wchar_t[requiredSize + 1];
    w[requiredSize] = 0;

    int retval = MultiByteToWideChar(CP_ACP,
                                     0, s.c_str(), s.length(), w, requiredSize);

    std::wstring r = w;
    delete[] w;
    return r;
}

std::string ws2s(const std::wstring &s)
{
    /*using namespace boost::locale::conv;
	std::string str = from_utf(s, "utf8");
	return str;*/

    int requiredSize = WideCharToMultiByte(CP_ACP,
                                           0, s.c_str(), s.length(), 0, 0, 0, 0);

    if (!requiredSize)
    {
        return 0;
    }

    char *w = new char[requiredSize + 1];
    w[requiredSize] = 0;

    int retval = WideCharToMultiByte(CP_ACP,
                                     0, s.c_str(), s.length(), w, requiredSize, 0, 0);

    std::string r = w;
    delete[] w;
    return r;
}
// selectfile(openOrSave, initPath, filter, dest=>{})
void v8selectfile(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    Local<Boolean> type = Local<Boolean>::Cast(args[0]);
    v8::String::Value valueType(isolate, type);

    Local<String> initPath = Local<String>::Cast(args[1]);
    v8::String::Value valInitPath(isolate, initPath);
    Local<String> filter = Local<String>::Cast(args[2]);
    v8::String::Value valFilter(isolate, filter);
    Local<Function> cb = Local<Function>::Cast(args[3]);
    std::string result;
    if (type->Value())
    {
        std::wstring wInitPath = reinterpret_cast<wchar_t *>(*valInitPath);
        std::wstring wFilterPath = reinterpret_cast<wchar_t *>(*valFilter);
        result = selectFile(true, ws2s(wInitPath).c_str(), ws2s(wFilterPath).c_str());
    }
    else
    {
        std::wstring wInitPath = reinterpret_cast<wchar_t *>(*valInitPath);
        std::wstring wFilterPath = reinterpret_cast<wchar_t *>(*valFilter);
        result = selectFile(false, ws2s(wInitPath).c_str(), ws2s(wFilterPath).c_str());
    }
    Local<String> r = String::NewFromUtf8(
                          isolate, result.c_str(), NewStringType::kNormal)
                          .ToLocalChecked();

    const unsigned argc = 1;
    Local<Value> argv[argc] = {r};
    cb->Call(context, Null(isolate), argc, argv).ToLocalChecked();
}

// selectdir(desc, initPath, dest=>{})
void v8selectdir(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<String> desc = Local<String>::Cast(args[0]);
    v8::String::Value valDesc(isolate, desc);
    Local<String> initPath = Local<String>::Cast(args[1]);
    v8::String::Value valInitPath(isolate, initPath);
    Local<Function> cb = Local<Function>::Cast(args[2]);
    std::string result;
    std::wstring wDesc = reinterpret_cast<wchar_t *>(*valDesc);
    std::wstring wInitPath = reinterpret_cast<wchar_t *>(*valInitPath);
    result = BrowseForFolder(ws2s(wDesc).c_str(), ws2s(wInitPath).c_str());

    Local<String> r = String::NewFromUtf8(
                          isolate, result.c_str(), NewStringType::kNormal)
                          .ToLocalChecked();
    const unsigned argc = 1;
    Local<Value> argv[argc] = {r};
    cb->Call(context, Null(isolate), argc, argv).ToLocalChecked();
}

void Initialize(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "messagebox", v8MessageBox);
    NODE_SET_METHOD(exports, "selectfile", v8selectfile);
    NODE_SET_METHOD(exports, "selectdir", v8selectdir);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

#endif /* WIN32 */