# fileunlocker
Application that terminates all processes that lock files inside a folder recursively

This application is useful when you need to delete a folder but can't be deleted because some unknown application is locking files in the folder and you don't know which process it is. This happens often with Visual Studio. When you close VS and try to delete for example node_modules but Windows won't let you because some process is holding to some file somewhere or you run c++ tests and make edits to source code and try to build but the test is still hanging and you can't rewrite it. In my setup I added this to file explorer right click context menu options so I can quickly run this application in file explorer.

What it does:
1) Read the first argument from cli arguments which is the path to the folder
2) List files recursively
3) Go trough the list and ask Windows is the file locked and if it is then ask what is the locking process and terminates the process

Output:


<img width="470" height="188" alt="image" src="https://github.com/user-attachments/assets/eab45a55-792f-42d4-bce6-5016bd416269" />


## To add right click menu to Windows Explorer

* regedit.exe
* add a new key under shell in Computer\HKEY_CLASSES_ROOT\Directory\shell\ naming it as you want to be displayed in the context menu item
* add a new key inside this key, for example command (mandatory name)
* edit the default property in UnlockFiles to "C:\path\path\myfileunlocker.exe" "%1" to pass the file path and name of the selected file to your custom program

should look like so:

<img width="691" height="125" alt="image" src="https://github.com/user-attachments/assets/daf241fe-51f1-48c7-9c97-cb569d35ea76" />


