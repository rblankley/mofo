# Project mofo

Money4Options is a cross-platform stock options evaluation tool.

## Building

This project requires Qt libraries for the user interface. For Windows I have used both 5.15.2 and 6.2.1. For Linux I have used whatever comes with Ubuntu 20.04 which I believe is 5.12.8.

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

When you run the application enter your credentials into the TDA API > Credentials dialog and save. Optionally you may simply update the `credentials.json` file with your app information. On Linux this is in `~/.config/mofo` or in your build folder on Windows.

![alt text](./docs/credentials.png?raw=true)

After doing this you need to sign into your account using the TDA API > Authenticate option. Follow the instructions to login to your TDA account. After logging in you will eventually get an error about localhost (or whatever your redirect URL is) not being found.

This is due to TDA OAuth requiring HTTPS domains for callbacks. Since HTTPS is not supported by Qt it will fail and you are shown something similar to this:

![alt text](./docs/redirecturi.png?raw=true)

To resolve this simply change the URL you get redirected to HTTP instead of HTTPS to workaround this issue. After doing this it will say your good to go and to close the window.

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

