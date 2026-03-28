# 🎛️ BasicCompressor

A basic audio compressor plugin developed in C++ using the JUCE framework. This processor reduces the dynamic range of the incoming audio signal based on standard dynamic compression parameters.
<p align="center">
  <img src="https://github.com/user-attachments/assets/d2e501cd-50b5-4d1d-9035-aaf207ea4c1c" width="600">
</p>

## 🎚️ Features and Controls

The plugin exposes the following control parameters:

* **Gain:** Adjusts the overall gain of the audio signal (range from -50 dB to +10 dB).
* **Threshold:** Sets the level in decibels above which the compressor begins to attenuate the signal (range from -60 dB to 0 dB).
* **Ratio:** Determines the amount of gain reduction applied once the signal exceeds the threshold (range from 1.0 to 10.0).
* **Attack:** The time in milliseconds it takes for the compressor to apply gain reduction (range from 0.1 ms to 100 ms).
* **Release:** The time in milliseconds it takes for the compressor to stop acting once the signal falls below the threshold (range from 10 ms to 500 ms).

## 🖥️ Graphical User Interface (GUI)

The interface includes visual monitoring elements:

* **Transfer Curve:** A static graph that updates to reflect the compression curve resulting from the *Threshold* and *Ratio* parameters.
* **Level Meters:** Two meters (Input and Output) that display real-time signal levels, calibrated from -60 dB to 0 dB.

## 🛠️ Technologies

* C++
* [JUCE Framework](https://juce.com/)

## 🚀 How to Build

To compile this project on Windows:

1. Make sure you have **JUCE** and **Visual Studio** (2019 or 2022) installed with C++ desktop development tools.
2. Open the `.jucer` project file using the **Projucer** application.
3. Click the Visual Studio icon in Projucer ("Save and Open in IDE") to generate the solution files.
4. In Visual Studio, select the configuration you need (`Debug` or `Release`) and build the project (`Build > Build Solution`).
5. You will find the compiled plugin (typically in VST3 or Standalone format) inside the `Builds/VisualStudio...` folder within your project directory.
