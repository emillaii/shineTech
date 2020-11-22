DeVaSys USB-I2C/IO - Software

Release Version: 5.00
Release Date: 02/14/2010.


*******************************************************************************************

LEGAL DISCLAIMER:

While this version of the USB-I2C/IO software has undergone significant in-house testing, it is a new release, it incorporates many changes, it has not undergone any field testing, and it may contain flaws.

DeVaSys makes ABSOLUTELY NO CLAIM to the suitability of this software for any use, whatsoever, and in fact, we DO NOT RECOMMEND that anyone install it on any system for which operation is critical.

We will not accept ANY RESPONSIBILITY for any damages or losses you might encounter as a result of attempting to use this software.


*******************************************************************************************

GENERAL INFORMATION ABOUT THIS RELEASE:


This release incorporates:

  1. Firmware update for old (Ax and Bx) USB-I2C/IO boards based on the Cypress AN2131QC micro-controller to Rev. 3.06.
  2. Firmware update for new (Cx) USB-I2C/IO boards based on the Silicon Labs C8051F340 micro-controller to Rev. 5.04.
  3. Major revision of usbi2cio.dll (API DLL) to Rev. 5.00.
  4. Major revision of dev03edb.sys driver (USB-I2C/IO driver) to Rev. 5.00.


This release is primarily intended as a "beta" release for customers that need immediate support for one or more new features, known bug fixes, or support for 64 bit versions of Windows.

Customers that are using older versions of the software without any issues may wish to delay upgrading.


*******************************************************************************************

COMPATIBILITY:

As always, we have put a lot of effort into maintaining backward compatibility with previous releases.

Customers should be able to run their existing applications without any changes to their code.

Executables should run as is, without the need to rebuild them.

We anticipate most compatibility issues that do arise will be the result of a mix of old and new revisions of the software (3.x dll with 5.0 driver, or 5.0 dll with 3.x driver).

Simply insuring that dll(s) and driver versions match should be enough to resolve most issues.


*******************************************************************************************

SUPPORTED PLATFORMS:

This version of the dll and driver were produced with Visual Studio 2008 and the Windows 7 WDK.

They will not work on Windows 98, ME, or 2000 (all of which are still supported via 3.x version driver and dll).

Targeted platforms (both 32 bit and 64 bit Windows platforms are now supported):
  1. Windows XP (x86 and x64).
  2. Windows Server 2003 (x86 and x64).
  3. Vista (x86 and x64).
  4. Windows Server 2008 R1 (x86 and x64).
  5. Windows 7 (x86 and x64).
  6. Windows Server 2008 R2 (x64 only, R2 does not come in x86 version).


*******************************************************************************************

WHO SHOULD INSTALL THIS UPGRADE:

Customers needing immediate support for any of the following new features:

  1. Support for x64 (64 bit version of Windows OS).
  2. Support of feature not offered in previous versions.
  3. Support of specific bug fix.


*******************************************************************************************

WHO SHOULD NOT INSTALL THIS UPGRADE:

If you are running a customized version of DeVaSys firmware, you should NOT attempt to use this software, please contact DeVaSys to obtain an updated version of your customized firmware.

Customers who are using an earlier version of the software, experiencing no problems, and that don't need any of the new features.


*******************************************************************************************

NEW FEATURES IN THIS RELEASE OF THE FIRMWARE (3.06, for Rev. Ax and Bx boards only):

Support for the GetSerialId vendor request which fetches the serial number from it's ram image, rather than performing an I2C eeprom read.

Support for 23 bit operations (users can get access to 2 I/O signals on the Debug header, 1 I/O on the I2C header).

Support for configurable "properties" which provide a mechanism to modify the way the board operates.  For example, this
version allows the user to configure I2C properties, providing the user the ability to turn off NAK detection, change the
number of retries on a failed I2C transaction, etc.


*******************************************************************************************

NEW FEATURES IN THIS RELEASE OF THE FIRMWARE (5.04, for Rev. Cx boards only):

Corrects problem with Cx boards not enumerating on Windows 98 and ME (5.04 firmware allows Cx boards to be used on 98 and ME with 3.x dll and driver).

Corrects problem with property reflection on port zero.


*******************************************************************************************

NEW FEATURES IN THIS RELEASE OF THE DRIVER (5.00):

Driver is now digitally signed with DeVaSys certificate issued by Verisign.

Driver is now available in both 32 bit and 64 bit versions.

Driver now has better embedded version information.  On most OS's, simply hovering the cursor over the driver file in explorer will display pop-up version information.


*******************************************************************************************

NEW FEATURES IN THIS RELEASE OF THE DLL (5.00):

DLL is now digitally signed with DeVaSys certificate issued by Verisign.

DLL is now available in both 32 bit and 64 bit versions.  On a 64 bit platform that supports WOW64 (32 bit apps on 64 bit platform), both versions of dll are installed.

This version of the DLL does more parameter checking, with an emphasis on checks for passed null pointers.

DLL now has better embedded version information.  On most OS's, simply hovering the cursor over the dll file in explorer will display pop-up version information.

*******************************************************************************************

KNOWN ISSUES:


Platform support.

This release does not support Windows 98, Windows ME, or Windows 2000.  User's needing support for those OS's should continue to use the 3.x version software.


Installers (msi files).

The current installers are not as good as we would like them to be, but they are functional, and much better than what we have had in the past.

We recently purchased a copy of InstallShield Professional and are still learning how to use it, in the interim, we appreciate your patience.

We plan to post improved installers on the web-site as they are developed, including msm files (merge files) to aid our customers in producing their own installs that incorporate our drivers and dlls.


*******************************************************************************************

CLOSING:

The staff of DeVaSys would like to thank all of our customers for their past, present, and future patronage!


*******************************************************************************************
