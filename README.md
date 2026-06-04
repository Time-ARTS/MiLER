# MiLER

Linjefølger for MILLER fase 2.

Prosjektet består av bildebehandling, styringslogikk og motorstyring som sammen gjør at roboten kan følge en linje autonomt.

---

## config

Håndterer lasting av konfigurasjon fra `config.json`.

Alle innstillinger samles her slik at systemet kan justeres uten å endre kildekoden.

Eksempler:

* motorpinner
* kamerainnstillinger
* PID-verdier
* systemmoduler
* debug-innstillinger

---

## run

Inngangspunktet for programmet.

Ansvar:

* laster konfigurasjon
* initialiserer kamera
* initialiserer motorstyring
* starter hovedløkken
* håndterer oppstart og avslutning

---

## loop

Prosjektets hovedsløyfe.

For hver iterasjon:

* henter bilde fra kamera
* sender bildet til `vision`
* mottar linjeinformasjon
* beregner styringssignal
* oppdaterer motorhastigheter

Dersom ingen linje oppdages kan systemet stoppe eller bruke siste gyldige verdi.

---

## vision

Ansvarlig for bildebehandling og linjedeteksjon.

Funksjonalitet:

* mottar kamerabilde
* filtrerer bort støy
* konverterer til binært bilde
* deler bildet inn i kvadranter
* finner linjens posisjon
* returnerer punkter som beskriver linjens retning

Brukes som grunnlag for all styring.

---

## quadrantHandler

Hjelpeklasse for `vision`.

Ansvar:

* håndtere individuelle bildekvadranter
* finne linjens sentrum i hver kvadrant
* returnere representative punkter

Gir mer stabile målinger enn å analysere hele bildet samlet.

---

## vectorTranslator

Oversetter punkter fra bildebehandlingen til styringsdata.

Ansvar:

* mottar punkter fra `vision`
* beregner linjens retning
* beregner avvik fra sentrum
* genererer styreverdi for roboten

Resultatet sendes videre til PID-regulatoren.

---

## PID

Regulerer styringen slik at roboten følger linjen stabilt.

Ansvar:

* beregne proporsjonal korreksjon
* beregne integralkomponent
* beregne derivatkomponent
* generere endelig korrigering

Verdiene for Kp, Ki og Kd defineres i konfigurasjonen.

---

## motorControl

Håndterer kommunikasjon med motorene.

Ansvar:

* motta ønskede motorhastigheter
* beregne venstre og høyre motoreffekt
* sende PWM-signaler til motorstyringen
* håndtere stopp og sikkerhetsfunksjoner

---

## shortKeyCommands

Gir mulighet for manuell styring og debugging under kjøring.

Funksjonalitet:

* aktivere/deaktivere moduler
* justere parametere
* skrive statusinformasjon
* kjøre testfunksjoner

Brukes hovedsakelig under utvikling og feilsøking.

---

## debugRenderer

Visualiserer data fra bildebehandlingen.

Kan vise:

* oppdaget linje
* kvadranter
* midtpunkter
* beregnede vektorer
* annen debug-informasjon

Brukes kun når debug-modus er aktiv.

---

## Prosessflyt

Kamera → Vision → QuadrantHandler → VectorTranslator → PID → MotorController

1. Kamera tar bilde.
2. Vision finner linjens posisjon.
3. QuadrantHandler beregner punkter.
4. VectorTranslator finner avvik og retning.
5. PID beregner korrigering.
6. MotorController oppdaterer motorene.
7. Prosessen gjentas kontinuerlig.
