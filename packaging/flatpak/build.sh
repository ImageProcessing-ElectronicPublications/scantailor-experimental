flatpak-builder --force-clean --user --repo=repo --install builddir io.github.ImageProcessing_ElectronicPublications.scantailor-experimental.yml
flatpak build-bundle repo scantailor-experimental.flatpak io.github.ImageProcessing_ElectronicPublications.scantailor-experimental --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
flatpak run --command=flatpak-builder-lint org.flatpak.Builder builddir ./builddir
