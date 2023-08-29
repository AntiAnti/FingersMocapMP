// (c) Yuri Kalinin 2020, All Rights Reserved (ykasczc@gmail.com)

#include "WindowsHelpersBPLibrary.h"
#include "WindowsHelpers.h"
#include "Windows/WindowsHWrapper.h"

#include "Misc/Paths.h"
#include "Windows/COMPointer.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/LocalTimestampDirectoryVisitor.h"
#include "Misc/FileHelper.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SWindow.h"
#include "GenericPlatform/GenericWindow.h"
#include "GenericPlatform/GenericPlatformApplicationMisc.h"

// Winodws API includes
#include "Windows/AllowWindowsPlatformTypes.h"
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Winver.h>
#include <LM.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include "Windows/HideWindowsPlatformTypes.h"

#pragma comment( lib, "version.lib" )

#ifndef MAX_FILETYPES_STR
#define MAX_FILETYPES_STR 4096
#endif

#ifndef MAX_FILENAME_STR
#define MAX_FILENAME_STR 65536 // This buffer has to be big enough to contain the names of all the selected files as well as the null characters between them and the null character at the end
#endif

namespace EWindowsFileDialogFlags
{
	enum Type
	{
		None = 0x00,		// No flags
		Multiple = 0x01		// Allow multiple file selections
	};
}

CA_SUPPRESS(6262)

UWindowsHelpersBPLibrary::UWindowsHelpersBPLibrary(const FObjectInitializer& ObjectInitializer)
{
}

FString UWindowsHelpersBPLibrary::GetUserAppDataFolder()
{
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		return FString(szPath);
	}
	else
	{
		return TEXT("");
	}
}

void UWindowsHelpersBPLibrary::DeleteFile(const FString& FileName)
{
	IFileManager::Get().Delete(*FileName);
}

bool UWindowsHelpersBPLibrary::SaveByteArrayToFile(const FString& FileName, const TArray<uint8>& Data, bool bOverrideExisting)
{
	if (FPaths::FileExists(FileName))
	{
		if (bOverrideExisting)
		{
			IFileManager::Get().Delete(*FileName);
		}
		else
		{
			return false;
		}
	}

	FFileHelper::SaveArrayToFile(Data, *FileName);
	return true;
}

void UWindowsHelpersBPLibrary::ListDirectory(const FString& DirectoryPath, TArray<FString>& FilesList)
{
	// Get all files in directory
	TArray<FString> directoriesToSkip;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
	PlatformFile.IterateDirectory(*DirectoryPath, Visitor);

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString FilePath = TimestampIt.Key();
		FString FileName = FPaths::GetCleanFilename(FilePath);
		bool shouldAddFile = true;

		if (FPaths::DirectoryExists(FilePath))
		{
			FileName.Append(TEXT("/"));
		}
		
		FilesList.Add(FileName);
	}
}

bool UWindowsHelpersBPLibrary::ShowFileOpenDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, bool bMultiSelect, TArray<FString>& OutFilenames, int32& OutFilterIndex)
{
	bool bSuccess = false;
	uint8 Flags = 0;
	HWND ParentWindowHandle = (HWND)GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();

	TComPtr<IFileDialog> FileDialog;
	if (SUCCEEDED(::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, IID_PPV_ARGS_Helper(&FileDialog))))
	{
		// Set this up as a multi-select picker
		if (bMultiSelect)
		{
			DWORD dwFlags = 0;
			FileDialog->GetOptions(&dwFlags);
			FileDialog->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);
		}

		// Set up common settings
		FileDialog->SetTitle(*DialogTitle);
		if (!DefaultPath.IsEmpty())
		{
			// SHCreateItemFromParsingName requires the given path be absolute and use \ rather than / as our normalized paths do
			FString DefaultWindowsPath = FPaths::ConvertRelativePathToFull(DefaultPath);
			DefaultWindowsPath.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);

			TComPtr<IShellItem> DefaultPathItem;
			if (SUCCEEDED(::SHCreateItemFromParsingName(*DefaultWindowsPath, nullptr, IID_PPV_ARGS(&DefaultPathItem))))
			{
				FileDialog->SetFolder(DefaultPathItem);
			}
		}

		// Set-up the file type filters
		TArray<FString> UnformattedExtensions;
		TArray<COMDLG_FILTERSPEC> FileDialogFilters;
		{
			// Split the given filter string (formatted as "Pair1String1|Pair1String2|Pair2String1|Pair2String2") into the Windows specific filter struct
			FileTypes.ParseIntoArray(UnformattedExtensions, TEXT("|"), true);

			if (UnformattedExtensions.Num() % 2 == 0)
			{
				FileDialogFilters.Reserve(UnformattedExtensions.Num() / 2);
				for (int32 ExtensionIndex = 0; ExtensionIndex < UnformattedExtensions.Num();)
				{
					COMDLG_FILTERSPEC& NewFilterSpec = FileDialogFilters[FileDialogFilters.AddDefaulted()];
					NewFilterSpec.pszName = *UnformattedExtensions[ExtensionIndex++];
					NewFilterSpec.pszSpec = *UnformattedExtensions[ExtensionIndex++];
				}
			}
		}
		FileDialog->SetFileTypes(FileDialogFilters.Num(), FileDialogFilters.GetData());

		// Show the picker
		if (SUCCEEDED(FileDialog->Show(ParentWindowHandle)))
		{
			OutFilterIndex = 0;
			if (SUCCEEDED(FileDialog->GetFileTypeIndex((UINT*)&OutFilterIndex)))
			{
				OutFilterIndex -= 1; // GetFileTypeIndex returns a 1-based index
			}

			auto AddOutFilename = [&OutFilenames](const FString& InFilename)
			{
				FString& OutFilename = OutFilenames[OutFilenames.Add(FPaths::ConvertRelativePathToFull(InFilename))];
				OutFilename = IFileManager::Get().ConvertToRelativePath(*OutFilename);
				FPaths::NormalizeFilename(OutFilename);
			};

			IFileOpenDialog* FileOpenDialog = static_cast<IFileOpenDialog*>(FileDialog.Get());

			TComPtr<IShellItemArray> Results;
			if (SUCCEEDED(FileOpenDialog->GetResults(&Results)))
			{
				DWORD NumResults = 0;
				Results->GetCount(&NumResults);
				for (DWORD ResultIndex = 0; ResultIndex < NumResults; ++ResultIndex)
				{
					TComPtr<IShellItem> Result;
					if (SUCCEEDED(Results->GetItemAt(ResultIndex, &Result)))
					{
						PWSTR pFilePath = nullptr;
						if (SUCCEEDED(Result->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
						{
							bSuccess = true;
							AddOutFilename(pFilePath);
							::CoTaskMemFree(pFilePath);
						}
					}
				}
			}
		}
	}

	return bSuccess;
}

bool UWindowsHelpersBPLibrary::OpenDirectoryDialog(const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName)
{
	bool bSuccess = false;

	HWND ParentWindowHandle = (HWND)GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();

	TComPtr<IFileOpenDialog> FileDialog;
	if (SUCCEEDED(::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileDialog))))
	{
		// Set this up as a folder picker
		{
			DWORD dwFlags = 0;
			FileDialog->GetOptions(&dwFlags);
			FileDialog->SetOptions(dwFlags | FOS_PICKFOLDERS);
		}

		// Set up common settings
		FileDialog->SetTitle(*DialogTitle);
		if (!DefaultPath.IsEmpty())
		{
			// SHCreateItemFromParsingName requires the given path be absolute and use \ rather than / as our normalized paths do
			FString DefaultWindowsPath = FPaths::ConvertRelativePathToFull(DefaultPath);
			DefaultWindowsPath.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);

			TComPtr<IShellItem> DefaultPathItem;
			if (SUCCEEDED(::SHCreateItemFromParsingName(*DefaultWindowsPath, nullptr, IID_PPV_ARGS(&DefaultPathItem))))
			{
				FileDialog->SetFolder(DefaultPathItem);
			}
		}

		// Show the picker
		if (SUCCEEDED(FileDialog->Show((HWND)ParentWindowHandle)))
		{
			TComPtr<IShellItem> Result;
			if (SUCCEEDED(FileDialog->GetResult(&Result)))
			{
				PWSTR pFilePath = nullptr;
				if (SUCCEEDED(Result->GetDisplayName(SIGDN_FILESYSPATH, &pFilePath)))
				{
					bSuccess = true;

					OutFolderName = pFilePath;
					FPaths::NormalizeDirectoryName(OutFolderName);

					::CoTaskMemFree(pFilePath);
				}
			}
		}
	}

	return bSuccess;
}

void UWindowsHelpersBPLibrary::CopyTextToClipboard(const FString& InText)
{
	FGenericPlatformApplicationMisc::ClipboardCopy(*InText);
}

FString UWindowsHelpersBPLibrary::GetTextFromClipboard()
{
	FString OutString;
	FGenericPlatformApplicationMisc::ClipboardPaste(OutString);
	return OutString;
}
