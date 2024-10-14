#include <iostream>
#include <stdio.h>
#include <windows.h>


SYSTEMTIME stUTC, stLocal;

bool moveFileToFolder(const char* sourcePath, const char* destinationPath)
{
    if (MoveFile(sourcePath, destinationPath)) {
        std::cout << "File moved successfully." << std::endl;
        return true;
    }
    else {
        std::cerr << "Failed to move file. Error: " << GetLastError() << std::endl;
        return false;
    }
}

bool createFolderIfNotExists(const char* folderPath) 
{
    DWORD fileAttributes = GetFileAttributesA(folderPath);

    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        if (CreateDirectoryA(folderPath, NULL))
        {
            printf("Folder created: \"%s\". \n", folderPath);
            return true;
        } else {
            printf("Error creating folder: \"%s\". \n", folderPath);
            std::cerr<< "Error code: " << GetLastError() << std::endl;
            return false;
        }
    } else if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return true;
    } else {
        std::cerr << "Error: Path exists but is not a directory: " << folderPath << std::endl;
        return false;
    }
}

std::string getFileCreationYearInfo(const char* filePath)
{
    HANDLE hFile; 
    hFile = CreateFile(filePath,                     // name of the write
                    GENERIC_WRITE,                   // open for write
                    FILE_SHARE_WRITE,                // share write
                    NULL,                            // default security
                    OPEN_EXISTING,                   // create new file only
                    FILE_ATTRIBUTE_NORMAL,           // normal file
                    NULL);                           // no attr. template
    
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        printf("Terminal failure: Unable to open file \"%s\".\n", filePath);
        return NULL;
    }   

    FILETIME creationTime, accessTime, writeTime;
    if (GetFileTime(hFile, &creationTime, &accessTime, &writeTime)) {
        FileTimeToSystemTime(&creationTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        auto creationYearWORD = stLocal.wYear;
        std::string creationYear = std::to_string(creationYearWORD);
        std::cout << "File " << std::string(filePath) << "has the year of creation--> " << creationYear << std::endl;
        CloseHandle(hFile);
        return creationYear;
    } else {
        std::cerr << "Error retrieving file time!" << std::endl;
    }

    CloseHandle(hFile);
    return NULL;
}

void sortFilesByYearOfCreationWindows(const char* folderPathSource, const char* folderPathDestination)
{
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;

    std::string searchPattern = std::string(folderPathSource) + "\\*";

    if ((hFind = FindFirstFile(searchPattern.c_str(), &FindFileData)) != INVALID_HANDLE_VALUE)
    {
        do {
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // Skip the "." and ".." directories
                if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0) {
                    std::cout << "Directory spotted: " << FindFileData.cFileName << "\n";
                }
                continue; // Skip directories
            }
            printf("\"%s\"\n", folderPathSource);
            auto fullFilePathSource = std::string(folderPathSource) + "\\" + FindFileData.cFileName;
            std::cout << "Processing file: \"" << fullFilePathSource << "\"\n";

            auto yearOfFileCreation = getFileCreationYearInfo(fullFilePathSource.c_str());
            auto fullFolderPathDestination = std::string(folderPathDestination) + "\\" + yearOfFileCreation;

            std::cout << "Destination folder: " << fullFolderPathDestination << "\"\n";
            
            if (createFolderIfNotExists(fullFolderPathDestination.c_str()))
            {
                moveFileToFolder(fullFilePathSource.c_str(), fullFolderPathDestination.c_str());
            }
        } while (FindNextFile(hFind, &FindFileData));
        FindClose(hFind);
    }
    else {
        printf("Terminal failure: Unable to open folder \"%s\".\n", folderPathSource);
    }
}

int main(int argc, char *argv[]) 
{
    printf("Start\n");
    sortFilesByYearOfCreationWindows(argv[1], argv[2]);
    return 0;
}

