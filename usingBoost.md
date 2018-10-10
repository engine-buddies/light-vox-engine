# Using boost for a first Time User
## and using it with Visual Studio 2017

Download the ZIP file from their [site](https://www.boost.org/doc/libs/1_68_0/more/getting_started/windows.html#get-boost)

Create a folder in `C:/Program Files/` called `boost`

extract the contents of the zip folder to there.
	The file structure should look like this:
	`C:/Program Files/boost/boost_1_XY_Z/ < Other folders and some setup scripts >`

If you are on Windows 10

Open up PowerShell / command line **_AS ADMIN_**
```
./boostrap.bat	# some setup to build boost
./b2		# More setup
./bjam		# Creates a folder called staging with a bunch of libs
```
Create a new project in Visual Studio

Open Project settings and ensure your configuration settings are set to All
Configs and All Platforms

---

Project Settings > C/C++ > General

Set an additional Include directory to be `C:\Program Files]boost\boost_1_XY_Z`

---

Project Settings > Linker > General

Add an additional library directory to
`C:\Program Files\boost\boost_1_XY_Z\stage\lib`
