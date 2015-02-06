# Grovolve

Grovolve is an artificial life simulator that demonstrates evolution by natural selection.  It is intended as an educational tool to improve the user's understanding of the evolutionary process.

Grovolve simulates plants growing, reproducing and dying in an environment with sunlight that shines from above.  Plants acquire energy by absorbing light with their leaves and expend their energy on growth, maintenance and generating spores.  When a plant runs out of energy, it dies.  Each plant has a genome that determines its growth and shape, and this genome is inherited, from parent to offspring, with the chance of mutations.

These ingredients are sufficient for natural selection to occur.  Plants with forms that increase their chance of survival will be more likely to have offspring, and plants with less advantageous forms tend to leave fewer offspring.  Over time, small advantageous changes can accumulate and lead to large evolutionary change.  It should be emphasised that while the plants and environment are simulated, the evolution they undertake is very much real – this program does not _simulate_ evolution, it _demonstrates_ it.

The primary selection pressure on the plants is light competition – shorter plants are more likely to be shaded by their neighbours and receive less light, and taller plants are likely to receive more light.  This encourages plants to evolve to be taller and more tree-like.

## Motivation

Grovolve was created because other programs that carry out evolution by natural selection are too enigmatic to be appropriate as an educational tool.  Grovolve is easy to use, and by simulating a real world scenario, plants competing for light, it is accessible to users learning about natural selection.

Other educational evolution programs are easy to use but fail to demonstrate a key concept in evolution – that the process can be constructive and creative.  Evolution does not just change the colour of moths or the beak size of birds, evolution made the moths and the birds in the first place, from less complex ancestors!  Grovolve allows users to watch this, as small grass-like plants evolve into larger and more complex tree-like forms.

## Usage

#### Basic usage
Running Grovolve is mostly hands-off, as natural selection works on its own without external interaction.  Simply start the simulation and periodically check back to see how the plants are changing.  By moving the speed slider fully to the right, you can hide the simulation to maximise the speed.  The population will continue to evolve indefinitely, but you can expect interesting changes in a matter of hours to days.

Toggling the 'Show shadows' button will illustrate the path of light in the simulation, so it is clear which areas are receiving sunlight and which are not.  Clicking on a plant will show its genome.  Genomes are very simple at the start of the simulation, but becoming increasingly complex as it progresses.

#### Advanced usage

Users can switch the program into advanced mode by using the 'Mode' menu.  This exposes more information and options, so the user can study and alter the simulation.  The environment parameters are changeable, which define the amount of light, the strength of gravity and the mutation rate.  The simulation information is accessible, including stats, plots over time and the population history.  The 'Advanced settings' dialog allows users to change the low-level parameters of the simulation.

#### Save and load

Simulations can be saved to file and resumed later.  At regular intervals, the program will automatically save the simulation to a temporary folder, to guard against lost progress in the event of a power outage or unplanned restart.  'Recover autosave files' is available under the 'Tools' menu when the program is in advanced mode.

## Installation

Grovolve users are encouraged to download the ready-to-use executable files available in the 'Releases' section of GitHub.  Users that wish to modify the program or compile it themselves can do so using the following instructions.

#### Building on Linux

The following instructions successfully build Grovolve on a fresh installation of Ubuntu 14.04:

1. Ensure the package lists are up-to-date: `sudo apt-get update`
2. Install the prerequisite packages: `sudo apt-get install build-essential qtbase5-dev libboost-all-dev libtbb-dev`
3. Download the Grovolve code from GitHub: `git clone https://github.com/rrwick/Grovolve.git`
4. Open a terminal in the Grovolve directory.
5. Set the environment variable to specify that you will be using Qt 5, not Qt 4: `export QT_SELECT=5`
6. Run qmake to generate a Makefile: `qmake`
7. Build the program: `make`
8. `Grovolve` should now be an executable file.
9. Optionally, copy the program into /usr/local/bin: `sudo make install`

#### Building on Mac

The following instructions successfully build Grovolve on OS X 10.9 Mavericks:

1. Install Xcode using the App Store.
2. Install Homebrew: [brew.sh](http://brew.sh/)
3. Install the prerequisite packages: `brew install qt5 boost tbb`
4. Download the Grovolve code from GitHub: `git clone https://github.com/rrwick/Grovolve.git`
5. Open a terminal in the Grovolve directory.
6. Run qmake to generate a Makefile: `/usr/local/opt/qt5/bin/qmake`
7. Build the program: `make`
8. `Grovolve` should now be an application bundle.
9. Optionally, copy the program into the 'Applications' folder.

#### Building on Windows

Building on a PC is a bit tougher due to the lack of a standard package manager that can take care of all dependencies.  It's easiest to downloaded each dependency manually and the Qt SDK, and then build the program using the included IDE, Qt Creator.  The following instructions successfully build Grovolve on Windows 7:

1. Install Visual Studio Express 2013: [www.visualstudio.com](http://www.visualstudio.com/)
2. Install Qt SDK: [qt-project.org](http://qt-project.org/)
3. Download Boost, compiled by Visual C++ 2013 (MSVC 12.0): [sourceforge.net/projects/boost/files/boost-binaries](http://sourceforge.net/projects/boost/files/boost-binaries/)
4. Download a compiled copy of Intel TBB: [www.threadingbuildingblocks.org/download](https://www.threadingbuildingblocks.org/download/)
5. Download the Grovolve code from GitHub: `git clone https://github.com/rrwick/Grovolve.git`
6. Ensure that the Grovolve directory, the Boost directory and the TBB directory are in the same parent directory.
7. If you have different versions of Boost or TBB than are specified in the Grovolve.pro file, it will be necessary to adjust the filepaths to those library in the Grovolve.pro file.
8. Open Qt Creator, load Grovolve.pro and configure the project.
9. Switch to a Release configuration and build the project.

## Contributing

New contributors are welcome!  If you're interested or have ideas, please contact me (Ryan) at rrwick@gmail.com.

## History

Version 1.0.0 – initial release

## License

GNU General Public License, version 3
