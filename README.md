# SkullOS

## Not Just Another Unix Clone

This project contains a minimalistic operatingsystem, thats written in C and Assembly. It can be executeed using QEMU.

## Abhängigkeiten

- `nasm`
- `qemu`
- `x86_64-elf-gcc` (Cross-Compiler)

### Installation (macOS mit Homebrew)

```bash
brew install x86_64-elf-gcc
```

**Wichtiger Hinweis:** Nach der Installation müssen Sie möglicherweise das `bin`-Verzeichnis von Homebrew zu Ihrem `PATH` hinzufügen. Für Apple Silicon (M1/M2) lautet der Befehl:

```bash
export PATH="/opt/homebrew/bin:$PATH"
```

Sie können diesen Befehl zu Ihrer `.zshrc` oder `.bash_profile` hinzufügen, um ihn dauerhaft zu machen.

## Struktur

- `boot/`: Enthält den 16-Bit-Bootloader.
- `kernel/`: Enthält den Kernel in C.
- `gui/`: Enthält das GUI-Modul in C.
- `linker.ld`: Linker-Skript.
- `Makefile`: Build-Skript.

## Bauen und Starten

**Hinweis:** Führen Sie `make clean` aus, um alle alten Objektdateien zu entfernen, bevor Sie das Projekt bauen.

Um das Betriebssystem zu bauen, führe folgenden Befehl aus:

```bash
make clean && make && make run
```

Um das Betriebssystem in QEMU zu starten, führe folgenden Befehl aus:

```bash
qemu-system-i386 -drive format=raw,file=os.bin
```
