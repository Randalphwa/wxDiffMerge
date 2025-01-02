This directory contains the docbook source for the DiffMerge manual.

This directory has everything set up and precompiled versions of all
the JARs and etc.

The entire manual is contained within the sgdm3-docbook.xml
source file.  It is translated by docbook into various formats
and then the generated .PDF, and etc are copied into
sgdm3/src/Installers for use (pre-packaged) by the various
installers.

Also, note that the sgdm3/src/Installers directory contains
other documentation (such as the Linux man page and the License
file) that may also need to be updated.

This directory also contains a zip file of examples.  This contains
all of the source files used in the screenshots.

================================================================

make -f Makefile.Unix

This should produce a tarball (./GENERATED/webhelp.tgz) 
of content for the website.  It ***IS NOT*** distributed with the 
product.

It also generates DiffMergeManual.pdf and copies it to
../../src/Installers

