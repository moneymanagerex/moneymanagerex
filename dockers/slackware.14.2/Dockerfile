FROM vbatts/slackware:14.2
MAINTAINER developers@moneymanagerex.org
RUN echo -e 'quiet=on\ncheck_certificate=off' >> ~/.wgetrc \
 && wget -O slackpkg+-1.7.0-noarch-5mt.txz \
      https://sourceforge.net/projects/slackpkgplus/files/slackpkg%2B-1.7.0-noarch-5mt.txz/download \
 && installpkg slackpkg+*.t?z \
 && sed -i 's/VERBOSE=1/VERBOSE=0/' /etc/slackpkg/slackpkgplus.conf \
 && slackpkg update gpg && slackpkg update \
 && slackpkg -default_answer=y -batch=on upgrade-all \
 && slackpkg update gpg && slackpkg update \
 && slackpkg -default_answer=y -batch=on install \
      cmake-3 libarchive-3 nettle lzo libxml2 cyrus-sasl make-4 ccache \
      binutils gcc-5 gcc-g++ guile gc libffi libmpc glibc-2 gettext-tools \
      git flex kernel-headers libcroco \
      \
      gtk+2 glib2 pango atk-2 cairo gdk-pixbuf2 freetype fontconfig libX11 \
      libXxf86vm libSM libnotify libsoup libXinerama libXi libXt libXrandr \
      libXcursor libXcomposite pixman mesa libxcb libXrender libxshmfence \
      libXdamage libXfixes libXext libdrm libXau libXdmcp harfbuzz libICE \
      enchant gstreamer gst-plugins-base libsecret libxslt sqlite-3 icu4c \
      orc libgcrypt libgpg-error curl \
 && sed -i 's/REPOPLUS=( slackpkgplus )/REPOPLUS=( slackpkgplus slackonly slackel )/' \
      /etc/slackpkg/slackpkgplus.conf \
 && echo "MIRRORPLUS['slackonly']=http://slackonly.com/pub/packages/14.2-x86_64/" \
      >> /etc/slackpkg/slackpkgplus.conf \
 && echo "MIRRORPLUS['slackel']=http://www.slackel.gr/repo/x86_64/current/" \
      >> /etc/slackpkg/slackpkgplus.conf \
 && slackpkg update gpg && slackpkg update \
 && slackpkg -default_answer=y -batch=on -checkgpg=off install \
      slackonly:libwebp slackonly:webkitgtk slackel:wxGTK3 \
 && rm -rf /var/cache/packages/* /var/lib/slackpkg/* \
           /usr/share/locale/* /usr/man/* \
           /tmp/* ~/.wgetrc slackpkg+*.t?z

ENV MMEX_INST_CMD for p in *.tar.xz; do mv -v \$p \${p%.tar.xz}.txz; done && installpkg ./mmex-*.txz
