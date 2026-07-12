Added functionality to V1_5:
Added function key FD, needs to be connected to PortA,2. Requires additional Pullup.
Added Lenz V36 Mode
New dispatch behaviour:
ESTOP+FA: FA-F0, FB-F1, FC-F2, FD-F3
ESTOP+FB: FA-F0, FB-F1, FC-F2, FD-F3; During normal Fred operation pressing FB/FC sets F1/F2 bit. Releasing FB/DC releases F1/F2 bit (V36 mode).
ESTOP+FC: FA-F0, FB-F3, FC-F4, FD-F1
