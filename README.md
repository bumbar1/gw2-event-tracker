Guild Wars 2 event tracker (overlay app)
====================

Usage
---------------------
> on first run click options and select langauge, server (and position)

Todo list
---------------------
- remove events from list; stop tracking (can be hidden, but are still tracked)
- track entire zones?
- options: add colors pickers for event states
- customizable wishlist
- Mac build

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
	- libglesv2.dll
	- libEGL.dll
	- why won't these link statically is beyond me....
		- libwinpthread-1.dll
		- libstdc++-6.dll
		- libgcc_s_seh-1.dll

Bugs
---------------------
> `updateEvents` : check if key exists (to prevent crash)
>
> (probably) tons of memory leaks