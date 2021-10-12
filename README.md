# System G

System G provides a graphical interface to the UNIX file system.  The on-line help explains all the features of the program.

Since we use System G extensively, we guarantee that it will continue to improve.  We have a very long list of features that we intend to implement, so please tell us what features you need, so we will know which ones on our list to implement first.


## Building from source

1. Install the [JX Application Framework](https://github.com/jafl/jx_application_framework),  and set the `JX_ROOT` environment variable to point to its `include` directory.
1. `./configure`
1. `makemake`
1. `make`


## Installing a binary

http://sourceforge.net/p/nps-systemg/

The easiest way to install from the binary package is to run `sudo ./install`

Otherwise, the program (systemg) should be placed in a directory that is on your execution path.  `/usr/bin` or `~/bin` are usually good choices.  Copy the libraries in `lib/` to a directory on your `LD_LIBRARY_PATH`.

### Requirements

On MacOS, this program requires XQuartz, the X11 server.  Before running this program, you must first start XQuartz.


## FAQ

For general questions, refer to the [Programs FAQ](https://github.com/jafl/jx_application_framework/blob/master/APPS.md).
