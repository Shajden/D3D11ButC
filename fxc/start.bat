.\bin\fxc.exe /T vs_5_0 /E main ..\hlsl-src\vs.hlsl /Fo .\shader-bin\vs.dxbc
.\bin\fxc.exe /T ps_5_0 /E main ..\hlsl-src\ps.hlsl /Fo .\shader-bin\ps.dxbc
copy .\shader-bin\vs.dxbc ..\vs.dxbc
copy .\shader-bin\ps.dxbc ..\ps.dxbc
PAUSE