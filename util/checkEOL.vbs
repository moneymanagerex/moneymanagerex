Set fso = CreateObject("Scripting.FileSystemObject")
Set Win = New RegExp : Win.Pattern = "\r\n"
Set Unix = New RegExp : Unix.Pattern = "[^\r]\n"

For Each arg In WScript.Arguments
  file = fso.OpenTextFile(arg).ReadAll
  If Win.Test(file) Then
    EOL = "=Win32"
  ElseIf Unix.Test(file) Then
    EOL = "=UNIX"
  Else
    EOL = "=Unknown"
  End If
  WScript.Echo fso.GetFileName(arg) & EOL
Next 'arg
