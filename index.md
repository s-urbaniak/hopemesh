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

# Documentation
Currently the documentation consists mainly of my thesis and a presentation:

* [hopemesh.pdf][presentation]

# Software
The aim was to create the project using Open Source software only. You will need the following software in order to get up and running:

* avrdude: For flashing the USB

[presentation]: doc/hopemesh.pdf
