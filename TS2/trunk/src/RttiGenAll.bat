
@pushd %SigEngine%\Src\Internal
@call RttiGenAll.bat
@popd

@cd GameApp
@call RttiGen.bat
@cd ..
