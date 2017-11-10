export version="1.7.7b1"
rm -rf lifutils.dst/*
rm -rf ../cmake-tmp
mkdir ../cmake-tmp
pushd ../cmake-tmp
cmake .. 
make DESTDIR=../macos/lifutils.dst install
make clean
popd
rm -rf lifutils.resources
mkdir lifutils.resources
cp ../LICENSE lifutils.resources
echo "LIFUTILS ${version}"  > lifutils.resources/welcome.txt
echo "" >> lifutils.resources/welcome.txt
echo "This package installs the LIFUTILS in the location /usr/local." >> lifutils.resources/welcome.txt
echo "Please uninstall any previously installed version first." >> lifutils.resources/welcome.txt
echo "LIFUTILS ${version} was installed on your system"  > lifutils.resources/conclusion.txt
echo "" >> lifutils.resources/conclusion.txt
echo "To uninstall the LIFUTILS do the following:" >> lifutils.resources/conclusion.txt
echo "" >> lifutils.resources/conclusion.txt
echo "Run the script create_lifutils_removescript.sh to create an uninstall script:" >> lifutils.resources/conclusion.txt
echo "bash /usr/local/share/lifutils/create_lifutils_removescript.sh > uninstall.sh" >> lifutils.resources/conclusion.txt
echo "" >> lifutils.resources/conclusion.txt
echo "Run the script uninstall.sh as administrator:" >> lifutils.resources/conclusion.txt
echo "sudo bash ./uninstall.sh" >> lifutils.resources/conclusion.txt
echo "" >> lifutils.resources/conclusion.txt
echo "Due to security reasons each delete operation requires confirmation." >> lifutils.resources/conclusion.txt
cp create_lifutils_removescript.sh lifutils.dst/usr/local/share/lifutils
rm -rf build_products
mkdir build_products
pkgbuild --identifier org.bug400.lifutils --version=$version --install-location="/" --root lifutils.dst --component-plist lifutils.plist build_products/lifutils.pkg 
productbuild --distribution lifutils.xml --package-path=build_products/ --resources=lifutils.resources lifutils.pkg
rm build_products/lifutils.pkg
rm -rf lifutils.dst/*
rm -rf ../cmake-tmp
