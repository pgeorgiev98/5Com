* Tue Sep 22 2020 - Petko Georgiev <petko.vas.georgiev@gmail.com> - v0.5.1
    - Made the plain text view put new lines after CR
    - Fix issues with saving the window size when maximixed/fullscreen
    - Fix issues with the changelog window

* Sat Nov 16 2019 - Petko Georgiev <petko.vas.georgiev@gmail.com> - v0.5.0
    - Added optimizations to Text View which lead to huge performance gains
      In certain workloads the application could be over 50x faster
    - Added an option to save/load sequences
    - Added an option to move sequences up/down in the list
    - Add option to save inputs to a list of favorite inputs
    - Make the keyboard shortcuts user-configurable
    - Other various bug fixes and enhancements

* Sun Oct 06 2019 - Petko Georgiev <petko.vas.georgiev@gmail.com> - v0.4.1
    - Fix a bug with the input field history
    - Fix a bug with restarting the app after an auto-update [Windows]

* Fri Aug 02 2019 - Petko Georgiev <petko.vas.georgiev@gmail.com> - v0.4.0
    - Text View was fully rewritten which will greatly improve performance
    - Fixed bug where not all port settings were saved
    - Tied the selections in Text View and Hex View together
    - Added an option to change the used monospace font
    - Bundled DejaVu Sans Mono as the default monospace font
    - Added an option to customize the main window size on startup

* Fri Jul 05 2019 - Petko Georgiev <petko.vas.georgiev@gmail.com> - v0.3.0
    - Added settings window
    - Added read buffer limit to prevent high memory usage and crashes
    - Now ports from /dev/pts/ can also be used as serial ports [Unix]
    - Added an option to export the 'byte receive times' as a csv table
    - Added input field history
    - Added 'Sequences'
    - Added an option to automatically update from within the application [Windows]
    - Major improvements to the HexView interface and performance
    - Added color and hints to the 'Plain Text View'
    - Improved the Export dialog
    - Added new shortcuts
    - And other minor features that are probably not worth noting here

* Thu Feb 28 2019 - Petko Georgiev <petko.vas.georgiev@gmail.com> - v0.2.0
    - Optimized the HexView for larger datasets
    - Added checking for updates
    - Various minor improvements and bugfixes

* Thu Feb 21 2019 - Petko Georgiev <petko.vas.georgiev@gmail.com> - v0.1.0
    - Initial release
