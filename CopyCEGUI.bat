@echo off
echo copying CEGUI libs and stuff from: "H:\c++ resources\CEGUIForkIt\Install"

echo copying dlls

xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUIBase-9999.dll" "CEGUI\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUIBase-9999_d.dll" "CEGUI\bin\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUICommonDialogs-9999.dll" "CEGUI\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUICommonDialogs-9999_d.dll" "CEGUI\bin\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUICoreWindowRendererSet.dll" "CEGUI\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUICoreWindowRendererSet_d.dll" "CEGUI\bin\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUIExpatParser.dll" "CEGUI\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUIExpatParser_d.dll" "CEGUI\bin\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUIOgreRenderer-9999.dll" "CEGUI\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUIOgreRenderer-9999_d.dll" "CEGUI\bin\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUISILLYImageCodec.dll" "CEGUI\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\bin\CEGUISILLYImageCodec_d.dll" "CEGUI\bin\" /D /Y

echo copying libs

xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUIBase-9999.lib" "CEGUI\lib\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUIBase-9999_d.lib" "CEGUI\lib\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUICommonDialogs-9999.lib" "CEGUI\lib\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUICommonDialogs-9999_d.lib" "CEGUI\lib\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUICoreWindowRendererSet.lib" "CEGUI\lib\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUICoreWindowRendererSet_d.lib" "CEGUI\lib\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUIExpatParser.lib" "CEGUI\lib\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUIExpatParser_d.lib" "CEGUI\lib\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUIOgreRenderer-9999.lib" "CEGUI\lib\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUIOgreRenderer-9999_d.lib" "CEGUI\lib\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUISILLYImageCodec.lib" "CEGUI\lib\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\Install\lib\CEGUISILLYImageCodec_d.lib" "CEGUI\lib\" /D /Y

IF ON==ON (

echo copying .pdbs
    
xcopy "H:\c++ resources\CEGUIForkIt\bin\CEGUIBase-9999.pdb" "H:\c++ resources\cmake testoutput\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\bin\CEGUIBase-9999_d.pdb" "H:\c++ resources\cmake testoutput\bin\" /D /Y

xcopy "H:\c++ resources\CEGUIForkIt\bin\CEGUIOgreRenderer-9999.pdb" "H:\c++ resources\cmake testoutput\bin\" /D /Y
xcopy "H:\c++ resources\CEGUIForkIt\bin\CEGUIOgreRenderer-9999_d.pdb" "H:\c++ resources\cmake testoutput\bin\" /D /Y

)

echo don't forget to copy the additional requirements like libexpat.dll, pcre.dll and others 

pause
