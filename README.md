# Project mofo

Money4Options is a cross-platform stock options evaluation tool.

## Building

### Linux

You will need the following packages:
~~~~
$ sudo apt-get install build-essential autoconf pandoc
~~~~

This project uses Qt5 for the user interface. There are a number of development packages needed but the following package seems to pull everything you need (at least in Ubuntu 20.04):
~~~~
$ sudo apt-get install libqt5networkauth5-dev
~~~~

Then run the following to build:
~~~~
$ autoreconf -vfi
$ ./configure
$ make
~~~~

If you have the clio logging library installed you can use that, too:
~~~~
$ ./configure --with-clio
~~~~

You can also build a debian package if you would prefer. You will need the following packages:
~~~~
$ sudo apt-get install fakeroot devscripts dh-autoreconf
~~~~

Then you can build a dpkg directly:
~~~~
$ dpkg-buildpackage -uc -us
~~~~

### Windows

A Qt solution file is provided in the `src` folder. Simply open it up and build.

I use Visual Studio 2019 (the free one) and it works fine.

## Documentation

For documentation you will need the following packages:
~~~~
$ sudo apt-get install doxygen
~~~~

If you want PDF/PS support install these packages too (its quite large):
~~~~
$ sudo apt-get install texlive texlive-latex-extra
~~~~

Then run the following to generate all doxygen documentation:
~~~~
$ make doxygen-doc
~~~~

If you only want HTML output instead run the following:
~~~~
$ make doxygen-html
~~~~



