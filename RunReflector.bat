@echo off
set REFLECTOR=C:\Users\varuu\development\Engine\build\reflector\Debug\Reflector.exe
set OUTPUT_DIR=C:\Users\varuu\development\Engine\build\.reflected
set ROOT=C:\Users\varuu\development\Engine

"%REFLECTOR%" "%OUTPUT_DIR%" ^
    "%ROOT%\EngineFiles\Core\GameObject\GameObject.hpp" ^
    "%ROOT%\EngineFiles\Core\GameObject\World\World.hpp" ^
    "%ROOT%\EngineFiles\Core\GameObject\Controller\Controller.hpp" ^
    "%ROOT%\EngineFiles\Core\GameObject\Entity\Entity.hpp" ^
    "%ROOT%\EngineFiles\Core\GameObject\Component\Component.hpp" ^
    "%ROOT%\EngineFiles\Core\GameObject\Component\SpriteComponent\SpriteComponent.hpp" ^
    "%ROOT%\EngineFiles\Core\GameObject\Component\MeshComponent\MeshComponent.hpp"

echo.
echo Reflector exited with code %ERRORLEVEL%
pause