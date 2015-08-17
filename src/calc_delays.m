
load MIDI_FB_ellip_pitch_60_96_22050_Q25.mat;

dirac = zeros(50000, 1);
dirac(1) = 1.0;

delays = zeros(120, 1);

for n = 21:120
    f = filter(h(n).b, h(n).a, dirac);
    [~,pos] = max(f(2:10000));
    [~,neg] = max(-f(2:10000));
    delays(n) = 2 + (pos + neg) / 2;
end

csvwrite('delays.csv', round(delays));
