FROM base/archlinux:latest
MAINTAINER developers@moneymanagerex.org
RUN pacman -Syuq --noprogressbar --needed --noconfirm git cmake make gcc gettext webkit2gtk wxgtk3 fakeroot \
    && pacman -Qdtq | xargs -r pacman -Rns && pacman -Sc --noconfirm

# unlock nobody required for makepkg
RUN usermod nobody -s /usr/bin/bash

# fix broken wxgtk3
RUN ln -sf /usr/bin/wx-config{-gtk3,} || true
