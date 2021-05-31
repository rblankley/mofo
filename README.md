# Project mofo

Money4Options is a cross-platform stock options evaluation tool.

## Building

### Windows

A Qt solution file is provided in the `src` folder. Simply open it up in Qt Creator and build.

I use Visual Studio 2019 (the free one) and it works fine.

### Linux

You will need the following packages:
~~~~
$ sudo apt-get install build-essential autoconf pandoc
~~~~

This project uses Qt5 for the user interface. There are a number of development packages needed but the following package seems to pull everything you need (at least in Ubuntu 20.04):
~~~~
$ sudo apt-get install libqt5networkauth5-dev
~~~~

You can open the Qt solution file up in creator (as per above) or use autotools:
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

## Usage

If you did not install from the debian package or run make install you will need to copy over the config files manually. Copy the following files to `/etc/mofo` (Linux) or your build folder (Windows):
~~~~
endpoints.config
logging.config
~~~~

For TDA api access you need to create a development account. Signup and add your app here:
https://developer.tdameritrade.com/user/me/apps/add

Then, after running the application, update the `credentials.json` file with your app information. On Linux this is in `~/.config/mofo` or in your build folder on Windows.

There is a bug with Qt Network Auth where after TDA OAuth you will get redirected to an https URL. Because https is not supported by Qt it will fail. Simply change the URL you get redirected to http versus https to workaround this issue.

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

