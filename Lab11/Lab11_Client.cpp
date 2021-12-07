#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <tchar.h>
using namespace std;
int main()
{
    wcout << "Connecting to pipe..." << endl;

    // Open the named pipe
    // Most of these parameters aren't very relevant for pipes.
    HANDLE pipe = CreateFile(
        L"\\\\.\\pipe\\my_pipe",
        GENERIC_READ | GENERIC_WRITE, // only need read access
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (pipe == INVALID_HANDLE_VALUE) {
        wcout << "Failed to connect to pipe." << endl;
        // look up error code here using GetLastError()
        system("pause");
        return 1;
    }

    // Sending name to server 
    wcout << "Enter your name:" << endl;
    string name;
    getline(cin, name);
    wstring widestr = wstring(name.begin(), name.end());
    const wchar_t* data = widestr.c_str();
    DWORD numBytesWritten = 0;
    BOOL result = WriteFile(
        pipe, // handle to our outbound pipe
        data, // data to send
        wcslen(data) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten, // will store actual amount of data sent
        NULL // not using overlapped IO
    );

    wcout << "Reading data from pipe..." << endl;
    // The read operation will block until there is data to read
    wchar_t buffer[128];
    DWORD numBytesRead = 0;
    result = ReadFile(
        pipe,
        buffer, // the data from the pipe will be put here
        127 * sizeof(wchar_t), // number of bytes allocated
        &numBytesRead, // this will store number of bytes actually read
        NULL // not using overlapped IO
    );

    if (result) {
        buffer[numBytesRead / sizeof(wchar_t)] = '\0'; // null terminate the string
        wcout << "Number of bytes read: " << numBytesRead << endl;
        wcout << "Message: " << buffer << endl;
    }
    else {
        wcout << "Failed to read data from the pipe." << endl;
    }
    wcout << "Sending data to server..." << endl;

    // This call blocks until a client process reads all the data
    wcout << "Enter any message:" << endl;
    string data1;
    getline(cin, data1);
    widestr = wstring(data1.begin(), data1.end());
    data = widestr.c_str();
    numBytesWritten = 0;
    result = WriteFile(
        pipe, // handle to our outbound pipe
        data, // data to send
        wcslen(data) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten, // will store actual amount of data sent
        NULL // not using overlapped IO
    );
    // Close our pipe handle
    CloseHandle(pipe);

    wcout << "Done." << endl;

    system("pause");
    return 0;
}