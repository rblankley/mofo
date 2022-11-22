# Project mofo

Money4Options is a cross-platform stock options evaluation tool.

## About

Are you a fan of Θ ThetaGang Θ stock option trading strategies as found on [r/thetagang](https://www.reddit.com/r/thetagang/)? Are you looking for an evaluation tool that can scan down and filter options on equities that you are interested in trading? If so this may be the tool for you...

This software allows you to build up watchlists and continuously scan down options on a configurable time basis. Options that pass your filter criteria are then shown in a list so you can have a closer look and decide if your interested in selling puts or not. You can also scan down stocks you own and determine the best call to sell.

This program uses TD Ameritrade API for fetching option and equity data. To make use of this software you will need to setup an account on their platform. Technically I don't believe you need to fund the account but their ToS platform is top notch and I highly recommend it.

## Usage

After building (see build instructions below) and before running you will need to copy over the config files manually. Copy the following files to `/etc/mofo` (Linux) or your build folder (Windows):
~~~~
endpoints.config
logging.config
~~~~

If you installed from the debian package or ran `make install` (Linux Only) then you can skip this manual copy process.

If you are not interested in building source yourself there is some limited binary packages available under the releases section. I will try to keep this reasonably up to date but it takes time and may lag behind the sources. You will need to follow the usage instructions below just the same as if you built from source.

For TDA API access you need to create a development account. Signup and add your app here:
https://developer.tdameritrade.com/user/me/apps/add

When you run the application enter your API credentials into the TDA API > Credentials dialog and save. Optionally you may simply update the `credentials.json` file with your app information. On Linux this is in `~/.config/mofo` or in your build folder on Windows.

![TDA Credentials Dialog](./doc/credentials.png?raw=true)

After doing this you need to sign into your trading account using the TDA API > Authenticate option. Follow the instructions to login to your TDA trading account. After logging in you will eventually get an error about localhost (or whatever your redirect URL is) not being found.

This is due to TDA OAuth requiring HTTPS domains for callbacks. Since HTTPS is not supported by Qt it will fail and you are shown something similar to this:

![HTTPS Callbacks are not supported in Qt](./doc/redirecturi.png?raw=true)

To resolve this simply change the URL you get redirected to HTTP instead of HTTPS to workaround this issue. After doing this it will say your good to go and to close the window.

### Watchlists

You will need to setup some watchlists of equities you are interested in. There are some common indices provided already but you should really setup your own. Navigate to View > Watchlists to get started.

![Watchlists Dialog](./doc/watchlists.png?raw=true)

### Filters

You should also setup some filters for the results output windows. You don't have to do this, but if you don't you will get everything possible which includes Calls, Puts, Verticals, etc. It's a ton of data and will bog down your system if you don't filter it. I recommend you setup something simple like showing OTM Puts only for Singles (no Verticals or other strategies). Keep in mind this software is a work in progress some trading strategies don't work yet but Singles and Verticals should. Navigate to View > Filters to set this up.

You can create as many filters as you like.

![Filters Dialog](./doc/filters.png?raw=true)

Here is an example filter that evaluates OTM Puts only with a reasonable spread percentage.

![Filter Edit Dialog](./doc/filtereditor.png?raw=true)

### Configuration

There are a number of configuration options found in View > Configuration.

![Configuration](./doc/options.png?raw=true)

Here you can mess with a number of things that control how TDA API is used and also constants used for pricing calculations. Be careful messing with things you don't understand! Feel free to go wild with color schemes though.

The really important ones are:

| Config | Description |
| ------ | ----------- |
| Option Chain Expiration End | This controls the maximum DTE that will be fetched by the daemon. By default this is set to 28 days which means options with a DTE greater than 28 days will not be evaluated. This is enforced no matter what your filter is set for! |
| Option Chain Watchlists | Controls what symbols are scanned down by the daemon background scanning process. |
| Option Pricing Evaluation Method | Pricing methods other than CRR Binomial Tree are a work in progress. Use with caution. |
| Option Analysis Filtering Method | Filter used for background scanning process. |

### Scanning your first option chain

After setting up a few basic filters you can scan single option chains by using TDA API > View Option Chain. The first time you lookup an equity it will take a little bit longer as it needs to fetch market history and fundamental data. Furthermore, you will also see a slight performance hit the first time you use it each day (though not nearly as bad as the first time you lookup an equity) as well because it will update this information.

![Price History](./doc/chart.png?raw=true)

![Option Chains](./doc/optionchains.png?raw=true)

The first tab contains a price history graph for the underlying. The remaining tabs contain option chains. Select an expiration date you are interested in and then press the Analysis button. If you setup filters you will be asked to choose one. You can analyze just the one expiration or all expirations by which button you press.

After analysis the results will show up down in the bottom.

![Option Chain Analysis Results](./doc/optionchainanalysis.png?raw=true)

For both tables (the chains up top and the analysis results below) you will probably want to customize look and feel. You can drag column headers around to re-order as you please. If you right click the header you can also modify what is shown and hidden from view.

![Layout Customization](./doc/layoutcustomization.png?raw=true)

The ability to save/restore layouts is provided. You can rename/delete/copy these from the View > Layouts dialog.

You can also sort results by whatever column you want.

### Background scanning

Scanning option chains in a singular fashion is nice and all but this tool was really designed to be run in the background constantly scanning down juicy option premiums for you to execute on. To startup the background daemon use the TDA API > Start Daemon option. By default the daemon will only actively scan during market hours but you can override this behavior with the Allow When Markets Closed option.

![Start Daemon](./doc/startdaemon.png?raw=true)

Once the daemon is running it will periodically scan down the watchlist(s) you selected in configuration.

You can view scanner results by navigating to Analysis > View Results.

![Scan Results](./doc/scanner.png?raw=true)

### On demand scanning

Sometimes you may want to scan down a watchlist using some new filter you just setup and see results instantly. This is possible using the Analysis > Custom Scan option.

![Custom Scan](./doc/customscan.png?raw=true)

You will be asked to choose a watchlist and a filter. After doing so you can view results the same way as background scanning.

![Custom Scan Dialog](./doc/customscanfilter.png?raw=true)

### Charts and Graphs

After running analysis you can view basic information on the underlying or the option trade through the right click menu.

![Option Trade Menu](./doc/optionmenu.png?raw=true)

For the underlying you can view basic fundamentals, implied volatility skew for all analyzed expirations, and implied versus historical estimated movement.

I've noticed TDA API gives back very mixed results regarding fundamental data. Some symbols appear to have lots of detail where others do not.

![Underlying Details](./doc/symboldetails.png?raw=true)

![Underlying Implied Volatility](./doc/symbolimplvol.png?raw=true)

![Underlying Estimated Movement](./doc/symbolestmove.png?raw=true)

For the analyzed option trade you can view estimated returns along with estimated movement information (the red background indicates three sigmas worth of estimated movement), implied volatility skew, probability curve, and open interest at each strike.

The volatility and estimated movement that I compute always seems to be a little less than ToS. I'm not sure if this is due to computation method (I tend to use binomial) or something else. I guess it goes without saying that you should do your own homework before making any trade!

![Option Trade Details](./doc/optiondetails.png?raw=true)

![Option Implied Volatility](./doc/optionimplvol.png?raw=true)

![Option Probability](./doc/optionprob.png?raw=true)

![Option Open Interest](./doc/optionopenint.png?raw=true)

## Building

This project requires Qt libraries for the user interface. For Windows I have used versions 5.15.2, 6.2.1, and 6.4.1. For Linux I have used whatever comes with Ubuntu 20.04 which I believe is 5.12.8.

### Windows

Qt supports several different compilers. I personally use the Community version of Visual Studio 2019 (free). You can download MSVC from the [Microsoft](https://visualstudio.microsoft.com/downloads/) site. When installing make sure to install the C++ package.

If your not using MSVC 2019 then I'm not sure... but it should be possible I believe (i.e. MinGW or other compiler).

If you just pulled down the binary package you may need the MSVC 2019 Redistributable package. You can find that at the [Microsoft](https://docs.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170) site.

Next you will need to install Qt. If you've never used Qt before you're in for a treat, it's an excellent cross platform package that does just about everything you need out of box. You can download Qt open source edition [here](https://www.qt.io/download-open-source).  Just look for the Download Qt Installer button and press it. They really, really want you to purchase a license and throw up confusing adverts but the open source (free) edition works just fine.

When installing make sure you choose the appropriate packages. You will need the Qt Network Authorization package. You may want to hunt down SSL toolkit (towards the bottom) and install that as well.

![Qt Installer](./doc/qtcomponents.png?raw=true)

A Qt solution file is provided in the `src` folder. Simply open it up in Qt Creator and build.

Lastly you may need to install Open SSL as a separate package. I've had mixed results with SSL, sometimes it works fine with the Qt installer version and other times not. If you get weird Failed to Initialize SSL errors when running the software you know you will need a separate package.

I've used the Open SSL package from [Shining Light](https://slproweb.com/products/Win32OpenSSL.html). The light version works just fine. You will probably need to reboot afterward though to get the DLLs into your PATH.

![Open SSL](./doc/openssl.png?raw=true)

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

## Code Documentation

For documentation you will need the following packages:
~~~~
$ sudo apt-get install doxygen graphviz
~~~~

Then run the following to generate all doxygen documentation:
~~~~
$ make doxygen-doc
~~~~

By default you get html documentation placed in the `doc` folder.

If you want PDF/PS support install these packages too (its quite large):
~~~~
$ sudo apt-get install texlive texlive-latex-extra
~~~~

And you will also need to configure them before running make.
~~~~
$ ./configure --enable-doxygen-pdf --enable-doxygen-ps
$ make doxygen-doc 
~~~~

## Final Note and a Word of Caution

This tool is provided in the hopes that it brings you some ease in finding your next option trade. Even though I personally use this tool quite often keep in mind it is a hobby project for me and probably doesn't even work correctly. I'm not a financial expert by any means and anything the program outputs is certainly not financial advice. Use with caution!

My eventual goal for this project is to have some sort of algo in place for auto-trading juicy targets. I have a long way to go however... I'd like to implement some sort of backtesting first.

Cheers!
