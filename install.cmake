install(TARGETS clamav-gui    
    RUNTIME DESTINATION usr/bin/
)

install(FILES extra/icon16/clamav-gui.png
    DESTINATION usr/share/icons/hicolor/16x16/apps/
)

install(FILES extra/icon22/clamav-gui.png
    DESTINATION usr/share/icons/hicolor/22x22/apps/
)

install(FILES extra/icon32/clamav-gui.png
    DESTINATION usr/share/icons/hicolor/32x32/apps/
)

install(FILES extra/icon48/clamav-gui.png
    DESTINATION usr/share/icons/hicolor/48x48/apps/
)

install(FILES extra/icon64/clamav-gui.png
    DESTINATION usr/share/icons/hicolor/64x64/apps/
)

install(FILES extra/icon128/clamav-gui.png
    DESTINATION usr/share/icons/hicolor/128x128/apps/
)

install(FILES extra/icon256/clamav-gui.png
    DESTINATION usr/share/icons/hicolor/256x256/apps/
)

install(DIRECTORY man 
    DESTINATION usr/share/
)

install(FILES CHANGES README
    DESTINATION usr/share/doc/clamav-gui/
)

install(FILES extra/ClamAV-GUI-ServiceMenu.desktop extra/ClamAV-GUI.desktop 
    DESTINATION usr/share/applications/
)

install(FILES   ${CMAKE_BINARY_DIR}/clamav-gui-uk_UA.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-pt_PT.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-it_IT.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-fr_FR.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-es_ES.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-en_GB.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-de_DE.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-da_DK.qm
                ${CMAKE_BINARY_DIR}/clamav-gui-zh_CN.qm
                ${CMAKE_BINARY_DIR}/clamav-en_GB.qm
                ${CMAKE_BINARY_DIR}/clamav-da_DK.qm 
                ${CMAKE_BINARY_DIR}/clamav-es_ES.qm
                ${CMAKE_BINARY_DIR}/clamav-fr_FR.qm 
                ${CMAKE_BINARY_DIR}/clamav-pt_PT.qm
                ${CMAKE_BINARY_DIR}/clamav-it_IT.qm 
                ${CMAKE_BINARY_DIR}/clamav-uk_UA.qm
                ${CMAKE_BINARY_DIR}/clamav-zh_CN.qm
                ${CMAKE_BINARY_DIR}/clamav-de_DE.qm
    DESTINATION usr/share/clamav-gui/              
)


install(FILES icons/da_DK.png icons/de_DE.png 
              icons/en_AU.png icons/en_GB.png icons/en_IE.png 
              icons/en_NZ.png icons/en_US.png icons/es_ES.png 
              icons/fr_FR.png icons/it_IT.png icons/pt_AO.png 
              icons/pt_BR.png icons/pt_CV.png icons/pt_MO.png 
              icons/pt_MZ.png icons/pt_PT.png icons/pt_ST.png 
              icons/pt_TL.png icons/uk_UA.png icons/zh_CN.png
    DESTINATION usr/share/clamav-gui/languageicons/
)