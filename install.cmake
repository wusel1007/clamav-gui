install(TARGETS ClamAV-GUI    
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

install(FILES   translations/clamav-gui-uk_UA.qm
                translations/clamav-gui-pt_PT.qm
                translations/clamav-gui-it_IT.qm
                translations/clamav-gui-fr_FR.qm
                translations/clamav-gui-es_ES.qm
                translations/clamav-gui-en_GB.qm
                translations/clamav-gui-de_DE.qm
                translations/clamav-gui-da_DK.qm
    DESTINATION usr/share/clamav-gui/              
)
