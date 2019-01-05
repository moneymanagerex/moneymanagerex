FROM vbatts/slackware:14.2
LABEL maintainer="developers@moneymanagerex.org"
RUN echo -e 'quiet=on\ncheck_certificate=off' >> ~/.wgetrc \
 && wget -O slackpkg+-1.7.0-noarch-5mt.txz \
      https://sourceforge.net/projects/slackpkgplus/files/slackpkg%2B-1.7.0-noarch-5mt.txz/download \
 && installpkg slackpkg+*.t?z \
 && sed -i 's/VERBOSE=1/VERBOSE=0/' /etc/slackpkg/slackpkgplus.conf \
 && slackpkg update gpg && slackpkg update \
 && slackpkg -default_answer=y -batch=on upgrade-all \
 && slackpkg update gpg && slackpkg update \
 && slackpkg -default_answer=y -batch=on install \
      atk-2 \
      binutils \
      cairo \
      ccache \
      cmake-3 \
      curl \
      cyrus-sasl \
      enchant \
      flex \
      fontconfig \
      freetype \
      gc \
      gcc-5 \
      gcc-g++ \
      gdk-pixbuf2 \
      gettext-tools \
      git \
      glib2 \
      glibc-2 \
      gst-plugins-base \
      gstreamer \
      gtk+2 \
      guile \
      harfbuzz \
      icu4c \
      kernel-headers \
      libarchive-3 \
      libcroco \
      libdrm \
      libffi \
      libgcrypt \
      libgpg-error \
      libICE \
      libmpc \
      libnotify \
      libsecret \
      libSM \
      libsoup \
      libX11 \
      libXau \
      libxcb \
      libXcomposite \
      libXcursor \
      libXdamage \
      libXdmcp \
      libXext \
      libXfixes \
      libXi \
      libXinerama \
      libxml2 \
      libXrandr \
      libXrender \
      libxshmfence \
      libxslt \
      libXt \
      libXxf86vm \
      lzo \
      make-4 \
      mesa \
      nettle \
      orc \
      pango \
      pixman \
      sqlite-3 \
 && sed -i 's/REPOPLUS=( slackpkgplus )/REPOPLUS=( slackpkgplus slackonly slackel )/' \
      /etc/slackpkg/slackpkgplus.conf \
 && echo "MIRRORPLUS['slackonly']=http://slackonly.com/pub/packages/14.2-x86_64/" \
      >> /etc/slackpkg/slackpkgplus.conf \
 && echo "MIRRORPLUS['slackel']=http://www.slackel.gr/repo/x86_64/current/" \
      >> /etc/slackpkg/slackpkgplus.conf \
 && slackpkg update gpg && slackpkg update \
 && slackpkg -default_answer=y -batch=on -checkgpg=off install \
      slackel:wxGTK3 \
      slackonly:libwebp \
      slackonly:rapidjson \
      slackonly:webkitgtk \
 && rm -rf /var/cache/packages/* /var/lib/slackpkg/* \
           /usr/share/locale/* /usr/man/* \
           /tmp/* ~/.wgetrc slackpkg+*.t?z

ENV MMEX_INST_CMD for p in *.tar.xz; do mv -v \$p \${p%.tar.xz}.txz; done && installpkg ./mmex-*.txz
