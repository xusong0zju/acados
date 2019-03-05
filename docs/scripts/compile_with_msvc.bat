cd "C:\Users\SYSCOP\Documents\GitHub\acados\build"
rem del /Q /S *
SET VSCMD_DEBUG=basic
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
cmake -G "Visual Studio 15 2017 Win64"  -DACADOS_WITH_OSQP=OFF ..
MSBuild.exe acados.sln /m /p:Platform="x64"
