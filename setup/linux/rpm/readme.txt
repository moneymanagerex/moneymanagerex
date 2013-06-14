Firstly rename source directory mmex to mmex-X.X.X.X and compress it as mmex-X.X.X.X.tar.gz

#rpmdev-setuptree

cp mmex.spec ~/rpmbuild/SPECS
cp mmex-X.X.X.X.tar.gz ~/rpmbuild/SOURCES
cp mmex\resources\mmex.xpm ~/rpmbuild/SOURCES

cd ~/rpmbuild/SPECS
rpmbuild -ba mmex.spec

# rpmdev-wipetree

P.S. OpenSUSE uses /usr/src/packages instead of ~/rpmbuild.


                                REMINDER

The "Release" tag can be thought of as the package's version. The release is 
traditionally an integer — for example, when a specific piece of software 
at a particular version is first packaged, the release should be "1". 
If it is necessary to repackage that software at the same version, the release
should be incremented. When a new version of the software becomes available, 
the release should drop back to "1" when it is first packaged. 

The "Icon" tag is used to name a file containing an icon representing the packaged 
software. The file may be in either GIF or XPM format, although XPM is preferred. 
In either case, the background of the icon should be transparent. The file should 
be placed in RPM's SOURCES directory prior to performing a build, so no path is 
needed.
