# fileunlocker
Application that terminates all processes that lock files in a folder

This application is usefull when you need to delete a folder but can't because some unknown application is locking files in the folder and you don't know which process that is. This happens often with Visual Studio. You close VS and try to delete for example node_modules but Windows won't let because some process is holding to some file somewhere. In my setup I added this to file explorer right click so I can quickly run this application.

What it does:
1) Read the first argument from cli arguments which is the path to the folder
2) List files recursively
3) Go trough the list and ask windows is the file locked and if it is then ask who is the locking process and terminate it

## To add right click menu to Windows Explorer

* regedit.exe
* add a new key under shell in Computer\HKEY_CLASSES_ROOT\Directory\shell\, naming it as you want to name the context menu item
* add a new key inside this key, for example UnlockFiles (mandatory name)
* edit the default property in UnlockFiles to myprogrampath\path\path\myfileunlocker.exe "%1" to pass the file path and name of the selected file to your custom program
