# PhantomBot Music Player — Windows 11

**PhantomBot Music Player** to desktopowa aplikacja C++ dla komputerów z Windows 11. Łączy estetyczny interfejs odtwarzacza PhantomBot z Discord Social SDK, zawiera przycisk **„Posłuchaj Muzyki”** otwierający `https://phantombot.pl/music` oraz przycisk **„Połącz z Discordem”** uruchamiający OAuth2 przez Discord.

## Wymagania dla Windows 11

Zainstaluj Visual Studio 2022 z workloadem **Desktop development with C++**, MSVC, Windows 10/11 SDK oraz narzędziami CMake. Potrzebny jest również Qt 6 z komponentem **Qt 6 Widgets**. Qt można zainstalować przez oficjalny instalator Qt albo przez vcpkg.

Discord Social SDK dla Windows jest już umieszczony w katalogu `discord_social_sdk`. Projekt używa plików `discord_partner_sdk.lib`, `discord_partner_sdk.dll` oraz `discord_krisp.dll` z wersji release.

## Konfiguracja aplikacji Discord

W Discord Developer Portal otwórz swoją aplikację i przejdź do zakładki OAuth2. W redirect URI dodaj dokładnie:

```text
http://127.0.0.1/callback
```

Application ID, którego używa aplikacja, to wartość zmiennej środowiskowej `DISCORD_APPLICATION_ID`. Nie wpisuj client secret do kodu ani nie commituj go do repozytorium.

## Uruchomienie w PowerShell

W PowerShell nie używa się polecenia `export`. Ustaw zmienną tak:

```powershell
$env:DISCORD_APPLICATION_ID = "1543541293102272522"
```

Wartość można sprawdzić poleceniem:

```powershell
$env:DISCORD_APPLICATION_ID
```

Ta zmienna działa w bieżącym oknie PowerShell. Aby ustawić ją trwale dla użytkownika Windows:

```powershell
[Environment]::SetEnvironmentVariable(
  "DISCORD_APPLICATION_ID",
  "1543541293102272522",
  "User"
)
```

Po trwałym ustawieniu uruchom nowe okno PowerShell lub Visual Studio.

## Budowanie przez Visual Studio i CMake

Otwórz **Developer PowerShell for VS 2022**, przejdź do katalogu projektu i skonfiguruj CMake. Jeśli Qt nie jest wykrywane automatycznie, podaj ścieżkę do katalogu Qt:

```powershell
cd C:\Users\jozwi\Downloads\PhantomBotMusicPlayer\PhantomBotMusicPlayer
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.8.0\msvc2022_64"
cmake --build build --config Release
```

Po udanym buildzie CMake skopiuje `discord_partner_sdk.dll` i `discord_krisp.dll` obok programu. Uruchomienie:

```powershell
$env:DISCORD_APPLICATION_ID = "1543541293102272522"
.\build\Release\PhantomBotMusicPlayer.exe
```

Jeżeli korzystasz z Qt zainstalowanego w innym miejscu, zmień wartość `CMAKE_PREFIX_PATH` na właściwy katalog `msvc2022_64`.

## Co robi aplikacja

Po uruchomieniu widoczny jest interfejs **PhantomBot Music Player** z krótkim opisem: „Muzyka, społeczność i Discord w jednym miejscu”. Przycisk **„Posłuchaj Muzyki”** otwiera stronę PhantomBot w domyślnej przeglądarce. Przycisk **„Połącz z Discordem”** tworzy klienta Discord Social SDK, generuje verifier PKCE, otwiera autoryzację OAuth2, odbiera redirect lokalny na `127.0.0.1` i wymienia kod na token przez `Client::GetToken`.

Aplikacja nie zapisuje tokenów na dysku. To bezpieczny wariant demonstracyjny; przy wydaniu produkcyjnym należy dodać bezpieczne odświeżanie tokenów, obsługę wygaśnięcia sesji, logowanie błędów oraz ustawienia prywatności.

## Struktura projektu

```text
.
├── CMakeLists.txt
├── README.md
├── src\main.cpp
└── discord_social_sdk\
    ├── include\
    ├── lib\release\
    │   └── discord_partner_sdk.lib
    └── bin\release\
        ├── discord_partner_sdk.dll
        └── discord_krisp.dll
```

## Uwaga o gotowym pliku EXE

Kod źródłowy jest przenośny, ale plik `.exe` dla Windows musi zostać zbudowany przez MSVC, ponieważ środowisko Linux nie może natywnie wygenerować binarnego programu Windows z bibliotekami Qt MSVC. Po wykonaniu opisanych poleceń Visual Studio utworzy właściwy `PhantomBotMusicPlayer.exe` dla Windows 11.
