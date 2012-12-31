---
layout: index
title: HopeMesh
tagline: Build a mesh network using Arduino like hardware
repository_url: https://github.com/s-urbaniak/hopemesh
---

# Overview
The HopeMesh project aims to build a fail-safe mesh network prototype using Arduino like hardware. The following key components were used to build a working module:

* CPU: ATmega162 / ATTiny2123
* RAM: 62256 32kB SRAM
* Radio module: RFM12B

# Status
We have two working prototypes communication with each other. You can see a video of the working modules here:

<iframe width="560" height="315" src="http://www.youtube.com/embed/JDviZbN_kvU" frameborder="0" allowfullscreen="true">_</iframe>

Currently you can only connect the modules via USB to an existing laptop/desktop machine using a terminal application. In the future a PS/2 keyboard and HD4780 LCD display is planned for autonomous working.

# Documentation
Currently the documentation consists mainly of my thesis and a presentation:

* [hopemesh.pdf][presentation]
* [hopemesh-thesis.pdf][thesis]

# Software
The aim was to create the project using Open Source software only. You will need the following software in order to get up and running:

* avrdude: For flashing the USB
* avr-gcc: For compiling the software
* gcc: For compiling and executing unit tests on a x86 machine

[presentation]: doc/hopemesh.pdf
[thesis]: doc/hopemesh-thesis.pdf
