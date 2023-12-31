[![Github CI](https://github.com/jafl/system-g/actions/workflows/ci.yml/badge.svg)](https://github.com/jafl/system-g/actions/workflows/ci.yml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_system-g&metric=alert_status)](https://sonarcloud.io/dashboard?id=jafl_system-g&branch=main)

[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_system-g&metric=sqale_rating)](https://sonarcloud.io/dashboard?id=jafl_system-g&branch=main)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_system-g&metric=security_rating)](https://sonarcloud.io/dashboard?id=jafl_system-g&branch=main)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_system-g&metric=vulnerabilities)](https://sonarcloud.io/dashboard?id=jafl_system-g&branch=main)

[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_system-g&metric=ncloc)](https://sonarcloud.io/dashboard?id=jafl_system-g&branch=main)

# System G

System G provides a graphical interface to the UNIX file system.  The on-line help explains all the features of the program.

Since we use System G extensively, we guarantee that it will continue to improve.  We have a very long list of features that we intend to implement, so please tell us what features you need, so we will know which ones on our list to implement first.


## Requirements

On MacOS, this program requires XQuartz, the X11 server.  Before running this program, you must first start XQuartz.


## Installing a binary

For macOS, the easiest way to install is via [Homebrew](https://brew.sh):

    brew install --cask xquartz
    brew tap jafl/jx
    brew install jafl/jx/systemg

For all other systems, download a package:

[![Download from Github](http://libjx.sourceforge.net/github.png)](https://github.com/jafl/system-g/releases/latest)

<a href="https://sourceforge.net/p/nps-systemg/"><img alt="Download System G" src="https://sourceforge.net/sflogo.php?type=17&group_id=170488" width="250"></a>

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-white.svg)](https://snapcraft.io/systemg)

If you download a tar, unpack it, cd into the directory, and run `sudo ./install`.

Otherwise, put the program (`systemg`) in a directory that is on your execution path.  `~/bin` is typically a good choice.


## Building from source

1. Install the [JX Application Framework](https://github.com/jafl/jx_application_framework),  and set the `JX_ROOT` environment variable to point to its `include` directory.
1. `./configure`
1. `make`


## FAQ

For general questions, refer to the [Programs FAQ](https://github.com/jafl/jx_application_framework/blob/master/APPS.md).

### Ubuntu snap

#### Why don't some of the features work?

Canonical refused to allow the program to run in "classic" mode, so it has very limited permissions.

If you want to use the git features, run the following in a terminal:

    sudo snap connect systemg:ssh-keys
