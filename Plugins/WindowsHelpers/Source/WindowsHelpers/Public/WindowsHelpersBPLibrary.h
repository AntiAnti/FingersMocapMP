// (c) Yuri Kalinin 2020, All Rights Reserved (ykasczc@gmail.com)

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "WindowsHelpersBPLibrary.generated.h"

/* 
* Windows API functions: clipboard and file dialogs
*/
UCLASS()
class UWindowsHelpersBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:

	/* File Open Dialog window
	* @param DialogTitle In: Title for dialog window
	* @param DefaultPath In: Initial path to open
	* @param DefaultFile In: File selected by default (full path)
	* @param FileTypes In: Allowed file types seperated by vertical line. Ex.: Autodesc FBX|*.fbx|All Files|*.*
	* @param bMultiSelect In: Allow selection of multiple files
	* @param OutFilenames Out: array of selected files (full names with paths)
	* @param OutFilterIndex Out: index of the selected file type
	*/
	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static bool ShowFileOpenDialog(
		const FString& DialogTitle,
		const FString& DefaultPath,
		const FString& DefaultFile,
		const FString& FileTypes,
		bool bMultiSelect,
		TArray<FString>& OutFilenames,
		int32& OutFilterIndex);

	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static bool OpenDirectoryDialog(const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName);

	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static void CopyTextToClipboard(const FString& InText);

	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static FString GetTextFromClipboard();

	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static FString GetUserAppDataFolder();

	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static void DeleteFile(const FString& FileName);

	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static bool SaveByteArrayToFile(const FString& FileName, const TArray<uint8>& Data, bool bOverrideExisting);

	// Returns list of files and subdirectories in the specified directory
	// Subdirectories have "/" at the end
	UFUNCTION(BlueprintCallable, Category = "Windows Helpers")
	static void ListDirectory(const FString& DirectoryPath, TArray<FString>& FilesList);
};
