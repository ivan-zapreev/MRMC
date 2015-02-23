#!/bin/sh -u

#This file is required to make all fonts in the resulting pdf embeded!
#This is the requirement of IEEE and QEST

dvips -t letter -Pdownload35 QEST2006.dvi	
rm -rf QEST2006~.ps
cat QEST2006.ps | sed 's+Times-Bold+NimbusSanL-Bold+g' | sed 's+Times-Roman+NimbusSanL-Regu+g' | sed 's+Times+NimbusSanL-Regu+g' | sed 's+Helvetica-BoldOblique+NimbusSanL-BoldItal+g' | sed 's+Helvetica-Oblique+NimbusSanL-ReguItal+g' | sed 's+Helvetica-Bold+NimbusSanL-Bold+g' | sed 's+Helvetica-Bold-iso+NimbusSanL-Bold+g' | sed 's+Helvetica+NimbusSanL-Regu+g' | sed 's+Helvetica-iso+NimbusSanL-Regu+g' | sed 's+Symbol+StandardSymL+g' > QEST2006~.ps
ps2pdf -dEmbedAllFonts=true QEST2006~.ps QEST2006.pdf
rm -rf QEST2006~.ps

