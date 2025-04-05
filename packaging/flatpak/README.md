# Flatpak
## Make flatpak file

* Install flatpak.

Debian/Ubuntu
``` sh
sudo apt-get install flatpak flatpak-builder
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install org.flatpak.Builder
flatpak install org.kde.Sdk/x86_64/5.15-23.08
flatpak install org.kde.Platform/x86_64/5.15-23.08
```

* Reboot after flatpak installation.

* Create flatpak file.

``` sh
sh ./build.sh
```

* Check flatpak linter errors.

## Install existing package

``` sh
flatpak install scantailor-experimental.flatpak 
```
