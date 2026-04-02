#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="${SCRIPT_DIR}"
cd "${ROOT_DIR}"

case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) ;;
  *)
    echo "This build script is intended for Git Bash on Windows." >&2
    exit 1
    ;;
esac

QT_VERSION="${QT_VERSION:-6.8.3}"
QT_HOST="${QT_HOST:-windows}"
QT_TARGET="${QT_TARGET:-desktop}"
QT_ARCH="${QT_ARCH:-win64_msvc2022_64}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

BUILD_CACHE_DIR="${ROOT_DIR}/build_cache"
PYTHON_VENV_DIR="${BUILD_CACHE_DIR}/python-venv"
QT_CACHE_DIR="${BUILD_CACHE_DIR}/Qt"
BUILD_DIR="${ROOT_DIR}/build"
PACKAGE_DIR="${BUILD_DIR}/package"

mkdir -p "${BUILD_CACHE_DIR}" "${BUILD_DIR}"

find_python() {
  if command -v py >/dev/null 2>&1; then
    echo "py -3"
    return 0
  fi
  if command -v python >/dev/null 2>&1; then
    echo "python"
    return 0
  fi
  if command -v python3 >/dev/null 2>&1; then
    echo "python3"
    return 0
  fi
  return 1
}

PYTHON_CMD="$(find_python || true)"
if [[ -z "${PYTHON_CMD}" ]]; then
  echo "missing Python. Install Python for Windows first." >&2
  exit 1
fi

if [[ ! -x "${PYTHON_VENV_DIR}/Scripts/python.exe" ]]; then
  echo "Creating Python virtualenv in build_cache..."
  eval "${PYTHON_CMD} -m venv \"${PYTHON_VENV_DIR}\""
fi

VENV_PYTHON="${PYTHON_VENV_DIR}/Scripts/python.exe"
if [[ ! -x "${VENV_PYTHON}" ]]; then
  echo "failed to create venv Python at ${VENV_PYTHON}" >&2
  exit 1
fi

echo "Ensuring aqtinstall is available..."
"${VENV_PYTHON}" -m pip install --upgrade pip >/dev/null
"${VENV_PYTHON}" -m pip install --upgrade aqtinstall >/dev/null

QT_BIN_PATH="$(find "${QT_CACHE_DIR}" -path "*/bin/windeployqt.exe" -print -quit 2>/dev/null || true)"
if [[ -z "${QT_BIN_PATH}" ]]; then
  echo "Installing Qt ${QT_VERSION} (${QT_ARCH}) into build_cache..."
  "${VENV_PYTHON}" -m aqt install-qt "${QT_HOST}" "${QT_TARGET}" "${QT_VERSION}" "${QT_ARCH}" --outputdir "${QT_CACHE_DIR}"
  QT_BIN_PATH="$(find "${QT_CACHE_DIR}" -path "*/bin/windeployqt.exe" -print -quit 2>/dev/null || true)"
fi

if [[ -z "${QT_BIN_PATH}" ]]; then
  echo "failed to locate windeployqt.exe under ${QT_CACHE_DIR}" >&2
  exit 1
fi

QT_ROOT_DIR="$(cd "$(dirname "${QT_BIN_PATH}")/.." && pwd)"
WIN_QT_ROOT_DIR="$(cygpath -w "${QT_ROOT_DIR}")"
WIN_ROOT_DIR="$(cygpath -w "${ROOT_DIR}")"
WIN_BUILD_DIR="$(cygpath -w "${BUILD_DIR}")"
WIN_PACKAGE_DIR="$(cygpath -w "${PACKAGE_DIR}")"
WIN_WINDEPLOYQT="$(cygpath -w "${QT_BIN_PATH}")"

VSWHERE_EXE="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
if [[ ! -x "${VSWHERE_EXE}" ]]; then
  echo "missing vswhere.exe. Install Visual Studio with the C++ desktop toolchain." >&2
  exit 1
fi

VS_INSTALLATION_PATH="$("${VSWHERE_EXE}" -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | tr -d '\r')"
if [[ -z "${VS_INSTALLATION_PATH}" ]]; then
  echo "failed to locate a Visual Studio installation with C++ tools." >&2
  exit 1
fi

VCVARS_PATH="${VS_INSTALLATION_PATH}/VC/Auxiliary/Build/vcvars64.bat"
if [[ ! -f "${VCVARS_PATH}" ]]; then
  echo "failed to locate vcvars64.bat: ${VCVARS_PATH}" >&2
  exit 1
fi

WIN_VCVARS_PATH="$(cygpath -w "${VCVARS_PATH}")"

find_ninja() {
  local candidate=""

  candidate="$(command -v ninja || true)"
  if [[ -n "${candidate}" ]]; then
    echo "${candidate}"
    return 0
  fi

  local vs_ninja="${VS_INSTALLATION_PATH}/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe"
  if [[ -f "${vs_ninja}" ]]; then
    echo "${vs_ninja}"
    return 0
  fi

  local vs_cmake_ninja="${VS_INSTALLATION_PATH}/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ninja.exe"
  if [[ -f "${vs_cmake_ninja}" ]]; then
    echo "${vs_cmake_ninja}"
    return 0
  fi

  local program_files_ninja="/c/Program Files/CMake/bin/ninja.exe"
  if [[ -f "${program_files_ninja}" ]]; then
    echo "${program_files_ninja}"
    return 0
  fi

  return 1
}

NINJA_CMD="$(find_ninja || true)"
if [[ -z "${NINJA_CMD}" ]]; then
  echo "missing ninja. Install Ninja or add it to PATH." >&2
  exit 1
fi
WIN_NINJA_CMD="$(cygpath -w "${NINJA_CMD}")"

BUILD_BAT="${BUILD_CACHE_DIR}/build_local.bat"
cat > "${BUILD_BAT}" <<EOF
@echo off
setlocal
call "${WIN_VCVARS_PATH}" || exit /b 1
set "PATH=%PATH%;$(dirname "${WIN_NINJA_CMD}")"
taskkill /F /IM pubg_rangefinder.exe >nul 2>nul
if exist "${WIN_BUILD_DIR}\pubg_rangefinder.exe" del /f /q "${WIN_BUILD_DIR}\pubg_rangefinder.exe" >nul 2>nul
if exist "${WIN_BUILD_DIR}\pubg_rangefinder.lib" del /f /q "${WIN_BUILD_DIR}\pubg_rangefinder.lib" >nul 2>nul
if exist "${WIN_BUILD_DIR}\pubg_rangefinder.pdb" del /f /q "${WIN_BUILD_DIR}\pubg_rangefinder.pdb" >nul 2>nul
if exist "${WIN_BUILD_DIR}\CMakeCache.txt" (
  findstr /C:"CMAKE_GENERATOR:INTERNAL=Visual Studio 17 2022" "${WIN_BUILD_DIR}\CMakeCache.txt" >nul 2>nul
  if not errorlevel 1 (
    del /f /q "${WIN_BUILD_DIR}\CMakeCache.txt" >nul 2>nul
    if exist "${WIN_BUILD_DIR}\CMakeFiles" rmdir /s /q "${WIN_BUILD_DIR}\CMakeFiles"
  )
)
cmake -S "${WIN_ROOT_DIR}" -B "${WIN_BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_MAKE_PROGRAM="${WIN_NINJA_CMD}" -DCMAKE_PREFIX_PATH="${WIN_QT_ROOT_DIR}" || exit /b 1
cmake --build "${WIN_BUILD_DIR}" || exit /b 1
if exist "${WIN_PACKAGE_DIR}" rmdir /s /q "${WIN_PACKAGE_DIR}"
mkdir "${WIN_PACKAGE_DIR}" || exit /b 1
copy /Y "${WIN_BUILD_DIR}\pubg_rangefinder.exe" "${WIN_PACKAGE_DIR}\pubg_rangefinder.exe" >nul || exit /b 1
if exist "${WIN_BUILD_DIR}\pubg_rangefinder.pdb" copy /Y "${WIN_BUILD_DIR}\pubg_rangefinder.pdb" "${WIN_PACKAGE_DIR}\pubg_rangefinder.pdb" >nul
"${WIN_WINDEPLOYQT}" --release --no-translations --no-opengl-sw "${WIN_PACKAGE_DIR}\pubg_rangefinder.exe" || exit /b 1
echo Runtime package directory: ${WIN_PACKAGE_DIR}
endlocal
EOF

echo "Building pubg_rangefinder..."
cmd.exe //c "$(cygpath -w "${BUILD_BAT}")"
