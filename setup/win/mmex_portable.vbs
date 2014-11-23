'--------------------------------------------------------------------------
'Author: gabriele-v            (c) 2014
'
'Purpose: Automate creation of release files to upload
'
'--------------------------------------------------------------------------
Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objShell = CreateObject("Wscript.Shell")
Set MmexFolder = objFSO.GetFolder(GetReleaseDir())
Set MmexSFolders = MmexFolder.SubFolders
For Each MmexSFolder in MmexSFolders
	If Instr(MmexSFolder.Name,"mmex_") = 1 Then
        Set RelFolder = objFSO.GetFolder(MmexSFolder.Path)
        Set RelSFolders = RelFolder.SubFolders
        For Each RelSFolder in RelSFolders
            FileName = RelSFolder.path&".zip"
	        zip_folder RelSFolder.path, FileName
            If objFSO.FileExists(FileName) Then
                MsgBox "ZIP created successfully:"&vbNewLine&vbNewLine&FileName,"0","MMEX Release"
            Else
                MsgBox "ZIP creation error:"&vbNewLine&vbNewLine&FileName,"0","MMEX Release"
            End If
        Next
    End If
Next
'Build InnoSetup
'objShell.Run Chr(34)&"%programfiles(x86)%\Inno Setup 5\iscc.exe"&Chr(34)&" "&Chr(34)&GetCurrentDir()&"\mmex_win32_setup.iss"&Chr(34), 1, true
'objShell.Run Chr(34)&"%programfiles(x86)%\Inno Setup 5\iscc.exe"&Chr(34)&" "&Chr(34)&GetCurrentDir()&"\mmex_x64_setup.iss"&Chr(34), 1, true



Function GetCurrentDir()
    Set objShell = CreateObject("Wscript.Shell")
    strPath = Wscript.ScriptFullName
    Set objFSO = CreateObject("Scripting.FileSystemObject")
    Set objFile = objFSO.GetFile(strPath)
    GetCurrentDir = objFSO.GetParentFolderName(objFile) 
End Function

Function GetReleaseDir()
    CurrFolder = GetCurrentDir()
    For i=0 to 1
        Set ObjFolder = objFSO.GetFolder(CurrFolder)
        CurrFolder = objFSO.GetParentFolderName(CurrFolder)
    Next
    GetReleaseDir = CurrFolder
End Function

Sub zip_folder(InputFolder,ZipFile)
   Set objFSO=Wscript.CreateObject("Scripting.FileSystemObject")

   If objFSO.FileExists(ZipFile) Then
       objFSO.DeleteFile ZipFile
   End If

   If Not objFSO.FolderExists(InputFolder) Then
       Exit Sub
   End If

   Set file = objFSO.CreateTextFile(ZipFile)
   file.Write Chr(80) & Chr(75) & Chr(5) & Chr(6) & String(18, 0)
   file.Close
   Set file = Nothing
   WScript.Sleep 500

   set sa = CreateObject("Shell.Application")
   Set zip = sa.NameSpace(ZipFile)
   Set d = sa.NameSpace(InputFolder)

   zip.CopyHere d.items

   Do Until d.Items.Count <= zip.Items.Count
       Wscript.Sleep(200)
   Loop
End Sub