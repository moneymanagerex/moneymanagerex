FROM archlinux/base:latest
LABEL maintainer="developers@moneymanagerex.org"
RUN pacman -Syuq --noprogressbar --needed --noconfirm \
      ccache \
      cmake \
      fakeroot \
      file \
      gawk \
      gcc \
      gettext \
      git \
      lsb-release \
      make \
      pkg-config \
      rapidjson \
      webkit2gtk \
      wxgtk3 \
 && pacman -Qdtq | xargs -r pacman -Rns --noconfirm \
 && pacman -Scc --noconfirm

# fix broken wxgtk3
RUN ln -sf /usr/bin/wx-config{-gtk3,} || true

# unlock nobody required for makepkg
# and configure for ccache
RUN usermod nobody -a -G root -d /root -s /usr/bin/bash \
 && mkdir -m 0775 /root/.ccache \
 && sed -i '/^BUILDENV=/s/!ccache/ccache/' /etc/makepkg.conf

ENV MMEX_INST_CMD pacman -U --noconfirm mmex* 

