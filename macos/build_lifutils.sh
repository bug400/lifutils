rm -rf lifutils.dst/*
rm -rf ../cmake-tmp
mkdir ../cmake-tmp
cd ../cmake-tmp
cmake .. 
make DESTDIR=../macos/lifutils.dst install
make clean
cd ../macos
rm -rf build_products
mkdir build_products
pkgbuild --identifier org.bug400.lifutils --version=1.7.2 --install-location="/" --root lifutils.dst --component-plist lifutils.plist build_products/lifutils.pkg 
productbuild --distribution lifutils.xml --package-path=build_products/ --resources=lifutils.resources lifutils.pkg
rm build_products/lifutils.pkg
rm -rf lifutils.dst/*
rm -rf ../cmake-tmp
