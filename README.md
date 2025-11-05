# MiLER
The MILLER bot i fase 2


## TODO:

- Build the code for the following:
  - [ ] constants
  - [ ] run
  - [ ] loop
  - [ ] vision
  - [ ] quadrantHandler
  - [ ] vectorTranslator
  - [ ] PID
  - [ ] MotorController

---

## constants

Inneholder alle konstanter og verdier som brukes i prosjektet.
Alle objektene (`PID`, `MotorController`, `vision`, osv.) baserer seg på disse verdiene slik at man enkelt kan endre oppførsel uten å endre kode direkte.

- motorhastigheter
- kameraoppsett
- PID-konstanter (Kp, Ki, Kd)
- intervaller og andre systemverdier

---

## run

Startkoden for linjefølgeren.
Den tester hver (si 30 sekunder som basis) om kamera ser en linje.
Dersom den gjør, start `loop`, ellers `continue`.

---

## loop

Kjører gjennom prosjektet

- henter frame fra kamera
- sender frame til `vision`
  - `vision` splitter opp kamera i kvadranter og finner midtpunkt av linjen
  - `vision` sender videre punktene til `vectorTranslator`
- `vectorTranslator` gjør punkter til vektorer og deretter til hastighet for motorside
- `MotorController` og `PID` kontrollerer motorene basert på input

---

## vision

Håndterer bildebehandling og linjedeteksjon.

- tar inn bilde (frame) fra kamera
- konverterer bildet til gråskala (strengt tatt ikke nødvendig) og filtrerer bort støy
- bruker `quadrantHandler` til å lage kvadranter (ved start)
- computerer midtpunktet i hver kvadrant
- sender punkt-par videre til `vectorTranslator`
- har en enkel funksjon for å sjekke om linje er synlig (`detect_line`) som brukes i `run`

---

## quadrantHandler

Hjelpefunksjon for `vision`.

- finner linjens midtpunkt i sin kvadrant
- returnerer ett punkt som representerer linjens posisjon
- brukes av `vision` for å få mer stabil linjedeteksjon

---

## vectorTranslator

Oversetter linjepunktene fra `vision` til bevegelsesdata.

- tar inn punkt-par fra `vision`
- beregner gjennomsnittlig x-posisjon og avvik fra bilens senter
- oversetter avviket til en styringsverdi (retning og styrke eller angulær hastighet/akselerasjon)
- sender resultatet videre til `PID` for korrigering

---

## PID

Brukes for å stabilisere styringen slik at roboten følger linjen jevnt.

- tar inn avvik fra `vectorTranslator`
- beregner korrigeringsverdi med PID-algoritmen
- bruker denne til å balansere forskjell mellom høyre og venstre motor
- verdier for Kp, Ki og Kd ligger i `constants`

---

## MotorController

Driver motorene basert på hastighetsinput fra `vectorTranslator` og justering fra `PID`.

- tar imot ønsket hastighet for hver motor
- sender signal til motorstyring
- skal kunne ta hensyn til retning og begrensninger