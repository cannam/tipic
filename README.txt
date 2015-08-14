
Tipic - Tibre-Invariant Pitch Chroma
====================================

A Vamp plugin (and accompanying C++ library) implementing pitch-chroma
audio features approaching timbre invariance, after the paper "Towards
timbre-invariant audio features for harmony-based music" by Meinard
Müller and Sebastian Ewert.

    https://code.soundsoftware.ac.uk/projects/tipic

This plugin is intended to provide features extracted from a music
audio signal, containing a reduction of the pitch and harmonic content
of the audio which is relatively stable in the presence of timbral
differences and local variations such as vibrato.

See the Chroma Toolbox,

    http://resources.mpi-inf.mpg.de/MIR/chromatoolbox/

for the original MATLAB implementations of these methods.


Authors
-------

Tipic library and plugin code written by Chris Cannam, based on the
MATLAB Chroma Toolbox implementation from Meinard Müller and Sebastian
Ewert.

The C++ code in the Tipic plugin and accompanying library is
Copyright 2015 Queen Mary, University of London, except where specified
in individual source files.

Distributed under the GNU General Public License: see the file COPYING
for details.


Citation, License and Use
-------------------------

If you make use of this software for academic purposes, please cite:

  Meinard Müller and Sebastian Ewert, "Towards timbre-invariant audio
  features for harmony-based music". IEEE Transactions on Audio,
  Speech, and Language Processing, volume 18 no 3, 2010, pp 649-662.

(See the CITATION file for a BibTeX reference.)

