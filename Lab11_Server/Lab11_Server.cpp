#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
using namespace std;
HANDLE hMutex;
HANDLE myH = CreateMutex(0, FALSE, (LPCWSTR)"Globa\\myH");
int counter = 0;
string prohibitedWords[] = { "NULP", "OS", "Some", "KRUK", "other" };
int prohibitedAmount = 3;
struct pr {
    vector <string> words;
    string wordToCheck;
};
DWORD WINAPI MassageChecker(__in LPVOID params) {
    
    pr parameters = *(pr*)params;
    for (string word : parameters.words) {
        if (word == parameters.wordToCheck)
            WaitForSingleObject(myH, INFINITE);
            counter++;
            ReleaseMutex(myH);
    }
    return 0;
}
int main(int argc, const char** argv) {
    wcout << "Creating an instance of a named pipe..." << endl;

    // Create a pipe to send data
    HANDLE pipe = CreateNamedPipe(
        L"\\\\.\\pipe\\my_pipe", // name of the pipe
        PIPE_ACCESS_DUPLEX, 
        PIPE_TYPE_BYTE, // send data as a byte stream
        1, // only allow 1 instance of this pipe
        0, // no outbound buffer
        0, // no inbound buffer
        0, // use default wait time
        NULL // use default security attributes
    );

    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
        wcout << "Failed to create outbound pipe instance.";
        // look up error code here using GetLastError()
        system("pause");
        return 1;
    }

    wcout << "Waiting for a client to connect to the pipe..." << endl;

    // This call blocks until a client process connects to the pipe
    BOOL result = ConnectNamedPipe(pipe, NULL);
    if (!result) {
        wcout << "Failed to make connection on named pipe." << endl;
        // look up error code here using GetLastError()
        CloseHandle(pipe); // close the pipe
        system("pause");
        return 1;
    }

    wcout << "Reading name from client..." << endl;
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
    const wchar_t* data = L"Your name successfully added";
    if (result) {
        buffer[numBytesRead / sizeof(wchar_t)] = '\0'; // null terminate the string
        wstring ws(buffer);
        string name(ws.begin(), ws.end());
        ifstream myfileRead;
        myfileRead.open("Users.txt");
        string fileData;
        myfileRead >> fileData;
        myfileRead.close();
        
        if (fileData.find(name) == std::string::npos) {
            ofstream myfile;
            myfile.open("Users.txt", ios_base::app);
            myfile << name + "\n";
            myfile.close();
        }
        else
            data = L"You are already registered:)";
    }
    else {
        wcout << "Failed to read name from the pipe." << endl;
    }

    wcout << "Sending data to pipe..." << endl; 
    // This call blocks until a client process reads all the data
    DWORD numBytesWritten = 0;
    result = WriteFile(
        pipe, // handle to our outbound pipe
        data, // data to send
        wcslen(data) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten, // will store actual amount of data sent
        NULL // not using overlapped IO
    );

    if (result) {
        wcout << "Number of bytes sent: " << numBytesWritten << endl;
    }
    else {
        wcout << "Failed to send data." << endl;
        // look up error code here using GetLastError()
    }

    wcout << "Reading client message..." << endl;
    // The read operation will block until there is data to read
    buffer[128];
    numBytesRead = 0;
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
    wstring ws1(buffer);
    string message(ws1.begin(), ws1.end());
    vector<string> words{};
    string delimiter = " ";
    size_t pos = 0;
    string token;
    while ((pos = message.find(delimiter)) != string::npos) {
        token = message.substr(0, pos);
        words.push_back(token);
        message.erase(0, pos + delimiter.length());
    }
    HANDLE* h = new HANDLE[prohibitedAmount];
    pr* par = new pr[prohibitedAmount];
    int i = 0;
    for (string w : prohibitedWords) {
        DWORD threadID;
        par[i].words = words;
        par[i].wordToCheck = w;
        h[i] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MassageChecker, (LPVOID)&par[i], NULL, &threadID);
    }
    WaitForMultipleObjects(prohibitedAmount, h, FALSE, INFINITE);


    wcout << "Sending data to pipe..." << endl;
    // This call blocks until a client process reads all the data
    
    if (counter < prohibitedAmount)
        data = buffer;
    else
        data = L"Too many prohibited words in your message!";

    numBytesWritten = 0;
    result = WriteFile(
        pipe, // handle to our outbound pipe
        data, // data to send
        wcslen(data) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten, // will store actual amount of data sent
        NULL // not using overlapped IO
    );

    if (result) {
        wcout << "Number of bytes sent: " << numBytesWritten << endl;
    }
    else {
        wcout << "Failed to send data." << endl;
        // look up error code here using GetLastError()
    }
    // Close the pipe (automatically disconnects client too)
    CloseHandle(pipe);

    wcout << "Done." << endl;

    system("pause");
    return 0;
}
