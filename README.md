# Spectral Panner

Spectral Panner is an audio plugin that does stereo frequency-based panning. It pans each frequency bin, providing more control over the stereo image than regular stereo panning at the cost of latency (which is required for a high-resolution FFT). Because of the high latency, it is not adequate for real-time processing but works well in post-processing.

There are two panning modes:
- **Stereo Balance**, which reduces the volume of the left channel when panning right and the other way around. In this mode, there is a slider for side gain compensation because the signal energy will naturally be reduced when panned.
- **Stereo Pan**, which mixes the two channels, moving information from one channel to the other.

## Screenshots
<img width="898" alt="1" src="https://github.com/user-attachments/assets/fbc3e86d-e234-4cf0-8c22-4f5072587d3d" />

## Demo
Watch a demo [here](https://drive.google.com/file/d/1M4CAxXv6wRsevz7Pfj9xH_59teBQdTn4/view?usp=drive_link).
