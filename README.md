# The SITS Timesharing System

SITS means "Small Incompatible Timesharing System".  It was a
timesharing operating system for a PDP-11/45, created in the mid-1970s
at MIT for running Logo.

AI memo 356 has this blurb: *During 1974-1975, our programming staff,
under the direction or R. Lebel, completed the design and
implementation of a general purpose multi-language timesharing system
for the PDP-11/45.  The SITS timesharing system was developed to
provide an environment suitable for running Logo and other PDP-11/45
programs.  It incorporates a Multics-like tree structured file system
including (potentially) full access control.  It also provides unique
capabilities for running programs as multiple process systems, rather
than the more common single process approach, and the ability for each
user to run many jobs simultaneously.  The system include provisions
for using both the older refreshed displays and our new raster
displays.*

### Installing SITS

Type `./build.sh` to install SITS from binaries in the bin directory.
The binaries were cross assembled from ITS.  The automated scripts
will format disks and install a basic system using paper tapes.

The SIMH emulator is compiled from source code; it may need some
dependencies installed first.

### Running SITS

Type `./run.sh` to run SITS.  Several windows will be opened which
represent teletypes, vector displays, and raster displays.

### Using SITS

Type ^Z to a teletype to log in, or Escape on a TV display.  Log in as
GUEST.  Type `: 0; . SLOGO` to run Logo.

Useful commands in DDT are `:HELP` to see a list of commands, `:SETD`
to set the current directory, `:LISTF` to list files, and `:LOGOUT` to
log out.  See the [DDTORD](doc/ddtord.6) file for more information.

### Using Logo

Useful commands in Logo include `CLEARSCREEN` to start turtle
graphics, and `GOODBYE` to log out.  The 1974 [AI memo 313](doc/AIM-313.pdf)
documents a slightly older version.

There are several different displays available:
- To use a TV display, log in on a TV and type `CS`.
- To use a vector display, log in on TTY 1 and type `CS`.
- To use the 2500, log in on the 2500 TV display and type `GTLDIS`.
