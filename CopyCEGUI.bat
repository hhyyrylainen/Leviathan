@echo off
echo copying CEGUI libs and stuff from: ""

echo copying dlls

xcopy "\bin\CEGUIBase-9999.dll" "CEGUI\bin\" /D /Y
xcopy "\bin\CEGUIBase-9999_d.dll" "CEGUI\bin\" /D /Y

xcopy "\bin\CEGUICommonDialogs-9999.dll" "CEGUI\bin\" /D /Y
xcopy "\bin\CEGUICommonDialogs-9999_d.dll" "CEGUI\bin\" /D /Y

xcopy "\bin\CEGUICoreWindowRendererSet.dll" "CEGUI\bin\" /D /Y
xcopy "\bin\CEGUICoreWindowRendererSet_d.dll" "CEGUI\bin\" /D /Y

xcopy "\bin\CEGUIExpatParser.dll" "CEGUI\bin\" /D /Y
xcopy "\bin\CEGUIExpatParser_d.dll" "CEGUI\bin\" /D /Y

xcopy "\bin\CEGUIOgreRenderer-9999.dll" "CEGUI\bin\" /D /Y
xcopy "\bin\CEGUIOgreRenderer-9999_d.dll" "CEGUI\bin\" /D /Y

xcopy "\bin\CEGUISILLYImageCodec.dll" "CEGUI\bin\" /D /Y
xcopy "\bin\CEGUISILLYImageCodec_d.dll" "CEGUI\bin\" /D /Y

echo copying libs

xcopy "\lib\CEGUIBase-9999.lib" "CEGUI\lib\" /D /Y
xcopy "\lib\CEGUIBase-9999_d.lib" "CEGUI\lib\" /D /Y

xcopy "\lib\CEGUICommonDialogs-9999.lib" "CEGUI\lib\" /D /Y
xcopy "\lib\CEGUICommonDialogs-9999_d.lib" "CEGUI\lib\" /D /Y

xcopy "\lib\CEGUICoreWindowRendererSet.lib" "CEGUI\lib\" /D /Y
xcopy "\lib\CEGUICoreWindowRendererSet_d.lib" "CEGUI\lib\" /D /Y

xcopy "\lib\CEGUIExpatParser.lib" "CEGUI\lib\" /D /Y
xcopy "\lib\CEGUIExpatParser_d.lib" "CEGUI\lib\" /D /Y

xcopy "\lib\CEGUIOgreRenderer-9999.lib" "CEGUI\lib\" /D /Y
xcopy "\lib\CEGUIOgreRenderer-9999_d.lib" "CEGUI\lib\" /D /Y

xcopy "\lib\CEGUISILLYImageCodec.lib" "CEGUI\lib\" /D /Y
xcopy "\lib\CEGUISILLYImageCodec_d.lib" "CEGUI\lib\" /D /Y

IF ON==ON (

echo copying .pdbs
    
xcopy "\CEGUIBase-9999.pdb" "/home/hhyyrylainen/Projects/leviathan/build\bin\" /D /Y
xcopy "\CEGUIBase-9999_d.pdb" "/home/hhyyrylainen/Projects/leviathan/build\bin\" /D /Y

xcopy "\CEGUIOgreRenderer-9999.pdb" "/home/hhyyrylainen/Projects/leviathan/build\bin\" /D /Y
xcopy "\CEGUIOgreRenderer-9999_d.pdb" "/home/hhyyrylainen/Projects/leviathan/build\bin\" /D /Y

)

echo don't forget to copy the additional requirements like libexpat.dll, pcre.dll and others 

pause
