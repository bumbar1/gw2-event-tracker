Guild Wars 2 event tracker (overlay app)
====================

Usage
---------------------
> on first run click options and select langauge, server


Todo list
---------------------
> remove events from list
>
> track entire zones?
>
> options: add colors pickers for event states
>
> customizable wishlist
>
> Mac build

Dependicies
---------------------
App creates ini file in DOCUMENTS/Guild Wars 2 (folder should be created by GW2 itself)

- Windows
	- Qt5Core.dll
	- Qt5Gui.dll
	- Qt5Network.dll
	- Qt5Widgets.dll
	- ssleay32.dll
	- libeay32.dll
	
Bugs
---------------------
> `updateEvents` : check if key exists (to prevent crash)
>
> when options window is closed, main window doesn't become borderless (fixed)
>
> (probably) tons of memory leaks
