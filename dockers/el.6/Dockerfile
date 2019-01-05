FROM centos:6
LABEL maintainer="developers@moneymanagerex.org"
RUN rpm --import https://getfedora.org/static/0608B895.txt \
 && yum -y install http://download.fedoraproject.org/pub/epel/epel-release-latest-6.noarch.rpm \
 && yum -y install centos-release-scl \
 && yum -y install --setopt=tsflags=nodocs \
      ccache \
      cmake3 \
      devtoolset-6-gcc-c++ \
      devtoolset-6-make \
      gettext \
      git \
      redhat-lsb-core \
      rpm-build \
      webkitgtk-devel \
      wxGTK3-devel \
 && yum clean all

# cmake3 installed from EPEL
RUN for f in cmake make pack test; do \
      ln -sv /usr/bin/c${f}3 /usr/bin/c${f}; done

ENV MMEX_INST_CMD yum -y -q install ./mmex-*.rpm

# Enable SCLs
ENTRYPOINT ["/usr/bin/scl", "enable", "devtoolset-6", "--"]
CMD ["/usr/bin/scl", "enable", "devtoolset-6", "--", "/bin/bash"]
