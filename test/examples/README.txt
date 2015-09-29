
This directory contains some example data calculated using the MATLAB
Chroma Toolbox.

The file ylsf-30sec-tipic-pitch.csv contains our input data: 88-bin
pitch features from a 30-second clip, at a rate of 10 per second so
300 features total. These came from a test version of the Tipic
plugin. They do not match the pitch features produced by the Chroma
Toolbox, because Tipic uses a causal filterbank rather than the
zero-phase forward-backward filter of the Chroma Toolbox.

The remaining files contain the results of processing the
ylsf-30sec-tipic-pitch.csv data using the pitch_to_* functions of the
Chroma Toolbox, using MATLAB commands along the lines of

>> pitch = csvread('ylsf-30sec-tipic-pitch.csv');
>> chroma = pitch_to_chroma([zeros(20,300); pitch'; zeros(12,300)]);
>> csvwrite('ylsf-30sec-chroma-from-tipic-pitch-defaults.csv', chroma');

(The zeros are there because the Toolbox functions expect 120-bin
pitch features while Tipic only emits 88 bins.)

The files here are:

 * ylsf-30sec-chroma-from-tipic-pitch-defaults.csv - CP chroma features,
   default parameters

 * ylsf-30sec-clp-from-tipic-pitch-defaults.csv - CLP log-compressed
   chroma features, default parameters (apart from log compression)

 * ylsf-30sec-cens-from-tipic-pitch.csv - CENS features, default parameters

 * ylsf-30sec-crp-from-tipic-pitch-defaults.csv - CRP features, default
   parameters

 * ylsf-30sec-crp-from-tipic-pitch-downsample.csv - CRP features, 10x
   downsampling with 41-point window enabled

The proposition that these are intended to help test is: If Tipic
produces the pitch features found in ylsf-30sec-tipic-pitch.csv, and
if we assume the MATLAB implementations are correct, then Tipic should
also produce chroma features matching those in these files. (Up to
rounding error based on the limited precision with which the original
pitch values were written out to CSV.)

