## Introduction 

Finite Difference Time Domain (FDTD) is a simulation method where Maxwell's equations are made discrete in both time and space. FDTD can be used to simulate the propragation of electromagnetic radiation.

This repo showcases a study of FDTD method that I developed in Qt 5 for Photonics course of my Master's degree in Physics Engineering, at the Faculty of Sciences of 
Univeristy of Lisbon.

## Requirements

- Qt 5.12.12 - Bare features are used.
- Qwt 6.1.2 - Used in data visualization.

> Qwt source is available for compilation. On the other hand, Qwt provides a plugin for Qt Desginer, `qwt_designer_plugin.dll`. This dll needs to be compiled with the same compiler that Qt Designer was compiled with. So you will need to compile Qwt two times: Once using the final compiler you'll be using to compile each project (e.g. MinGW), and a second time to generate the `qwt_designer_plugin.dll` (e.g. with MSVC).

> Also, attempting to compile in Debug mode was not working. I was getting the error below. This is likelly because the wrong Qwt dlls were linked. Debug versions should be used. 

```
QWidget: Must construct a QApplication before a QWidget
20:42:20: The program has unexpectedly finished.
```



## Contents

### OneD
This project illustrated the one dimension case.
A Perfectly Matched Layer (PML) is implemented 


### 2D - TODO