# ReSpeaker XVF3800 — obudowa R2 do zatwierdzenia

Projekt dla Marcina, 18.11.2026. Zmiana R2: mleczne okienka LED są w bocznej
ściance, a na górze jest miejsce pod Pi. Standardowa okrągła płytka Ø100 mm
z 12 LED; nie dotyczy odmiany 44 mm mic pitch / 24 LED.

## Konstrukcja

- Górna i dolna pokrywa: około Ø112 × 20,2 mm.
- 12 identycznych mlecznych wkładek: odbiornik nad diodą, prowadnica omijająca
  kanał mikrofonu oraz okienko w bocznej ściance. Wkładki wkłada się od środka;
  kołnierz zatrzymuje je przed wypadnięciem na zewnątrz, małe żebra dają wcisk.
- Cztery osobne stopki TPU, rozstaw 60 × 44 mm. Prześwit pod Pi 6,6 mm,
  całkowita wysokość ze stopkami 26,8 mm. Stopki ograniczają przenoszenie drgań.
- Główny USB-C, mini-jack 3,5 mm (wycięcie Ø10,2 mm pod korpus wtyczki),
  dodatkowy USB XIAO, dostęp do Mute i Reset.
- Cztery kanały dźwiękowe Ø3 mm nad wlotami mikrofonów; Pi nie może leżeć
  bezpośrednio na pokrywie, bo zasłoniłoby te otwory.
- PCB: 3 śruby M2 × 6. Obudowa: 4 śruby M2,5 × 12 od spodu.
  Otwory pilotowe do nacięcia gwintu w tworzywie, bez metalowych wkładek.

## Najpierw pojedyncza próba

Wydrukuj `05_PROBKA_SZCZELINY_R2.stl` i `03_MLECZNY_SWIATLOWOD_1szt_R2.stl`.
Próbka jest prostym przymiarem szerokości i wysokości okienka, a nie wierną
kopią krzywizny ścianki. Sprawdź wcisk końcówki i jasność na jednej diodzie.
Użyj mlecznego/półprzezroczystego PETG; mocno kryjąca biel może tłumić światło.
Światło musi zmienić kierunek i przejść przez kilkanaście milimetrów wydruku.
Nie sprawdzono jeszcze jego jasności fizycznie. Jeśli efekt będzie za słaby,
prowadnicę należy wykonać z przezroczystego PETG i zmatowić tylko zewnętrzne
okienko albo zastosować osobny cienki mleczny dyfuzor na przezroczystej prowadnicy.
Nie zmniejszaj skali obudowy: przesunęłoby to mikrofony i złącza.

## Druk

- Pokrywy: PLA/PETG, warstwa 0,20 mm, 3 obrysy, 20–25% wypełnienia.
  STL mają zewnętrzne płaskie powierzchnie na stole. Obejrzyj mosty przy bocznych
  otworach; w razie potrzeby dodaj miejscowe podpory, nie w kanałach mikrofonów.
- Wkładki: mleczny PETG, warstwa 0,16–0,20 mm. Układ STL leży płasko;
  pod uniesioną zewnętrzną końcówką może być potrzebna mała podpora. Usuń ją
  ostrożnie, nie uszkadzając żeber wcisku. Zacznij od jednej sztuki.
- Stopki: TPU, wydrukuj 4 kopie `06_STOPKA_TPU_1szt_R2.stl`.
  Są ustawione szeroką powierzchnią na stole, z trzpieniem ku górze.
- Dołączone tacki 3MF zawierają geometrię bez profilu drukarki i bez G-code.

## Montaż

1. Włóż 12 prowadnic od wewnątrz w boczne okienka. Odbiorniki mają wypaść nad LED.
2. Wciśnij 4 miękkie stopki TPU w otwory górnej pokrywy. Kołnierzyk trzpienia
   ma przejść przez otwór; użyj wyczucia. To także wymaga próby na Twoim TPU.
3. Przykręć PCB do trzech dystansów dolnej pokrywy. Strona z logo Seeed i LED
   skierowana w górę, elektronika w dół. Nie dokręcaj na siłę.
4. Zamknij obudowę na rancie i połącz czterema śrubami od spodu.
5. Pi ustaw na stopkach, pozostawiając wolny prześwit dookoła mikrofonów.
   Podparcie nie jest dopasowanym zatrzaskiem do konkretnej obudowy Pi.

## Co potwierdzono i czego jeszcze nie

Raport verification.json opisuje kolizje z oficjalnym modelem oraz bryły CAD.
To nie zastępuje fizycznej przymiarki Twojej v1.1, testu jasności, mikrofonów
ani stabilności Pi. Szerokie/kątowe wtyczki mogą wymagać większego wycięcia.
Kanały mikrofonów kończą się 0,24 mm nad PCB; cienkie miękkie uszczelki wokół
wlotów mogą poprawić izolację akustyczną. Nie zaklejaj otworów. Obudowa nie jest
wodoszczelna. Wentylator Pi może być słyszalny mimo miękkich stopek.

## Źródła

Model standardowej płytki Seeed Ø100 mm z opcjonalnym XIAO. Plik producenta
nie potwierdza jednoznacznie rewizji v1.1 — przed pełnym drukiem porównaj złącza,
12 diod i trzy otwory mocujące ze swoją płytką.

- https://wiki.seeedstudio.com/respeaker_xvf3800_introduction/
- https://files.seeedstudio.com/wiki/respeaker_xvf3800_usb/3d/respeaker_mic_array_xvf3800_1_with-xiao-0820.stp
- https://files.seeedstudio.com/wiki/respeaker_xvf3800_usb/respeaker_xvf3800_2d_mechanical_drawing.pdf

Geometria elektroniki: Seeed Studio. Obudowa R2: projekt dla Kora Workshop.
Do odtworzenia: Python + CadQuery 2.7, `build.py`, następnie `publish_assets.py`.
