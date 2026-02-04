# ===== Set MSYS2 GCC path =====
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH

Write-Host "Using GCC:"
gcc -dumpmachine

# ===== Clean old files =====
Write-Host "Cleaning old objects..."
Remove-Item *.o -ErrorAction SilentlyContinue
Remove-Item finance_dsa.dll -ErrorAction SilentlyContinue

# ===== Compile all C files =====
Write-Host "Compiling..."
gcc -c -fPIC -O2 *.c

# ===== Link into DLL =====
Write-Host "Linking DLL..."
gcc -shared -o finance_dsa.dll *.o

Write-Host "`n✅ Build complete: finance_dsa.dll created!"
