# CPack magic to generate packages
# This file is used to configure CPack for packaging the ClamAV-GUI application.
# It sets the package name, version, and other metadata required for packaging.
# It also specifies the files to include in the package and the installation directories.
# The package will be created in the build directory and can be used to distribute the application.
set(CPACK_PACKAGE_CONTACT "Jörg Macedo da Costa Zopes")
set(CPACK_PACKAGE_NAME "ClamAV-GUI")
set(CPACK_PACKAGE_VERSION "1.1.4")
set(CPACK_PACKAGE_FILE_NAME "ClamAV-GUI-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_ARCHIVING_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jörg Macedo da Costa Zopes")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "clamav, clamav-daemon, clamav-freshclam")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "ClamAV GUI for Linux")

set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Utilities")
set(CPACK_RPM_PACKAGE_REQUIRES "clamav, clamav-daemon, clamav-freshclam")
set(CPACK_RPM_PACKAGE_RELEASE 1)
set(CPACK_RPM_PACKAGE_DESCRIPTION "ClamAV GUI for Linux")

include(CPack)
