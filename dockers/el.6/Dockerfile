FROM centos:6
MAINTAINER developers@moneymanagerex.org
RUN rpm --import https://getfedora.org/static/0608B895.txt \
 && yum -y install http://download.fedoraproject.org/pub/epel/epel-release-latest-6.noarch.rpm \
 && yum -y install centos-release-scl \
 && yum -y install --setopt=tsflags=nodocs \
        cmake3 devtoolset-6-gcc-c++ gettext git devtoolset-6-make rpm-build redhat-lsb-core \
        webkitgtk-devel wxGTK3-devel ccache \
 && yum clean all

# cmake3 installed from EPEL
RUN for f in cmake make pack test; do \
      ln -sv /usr/bin/c${f}3 /usr/bin/c${f}; done

ENV MMEX_INST_CMD yum -y -q install ./mmex-*.rpm

# Enable SCLs
ENTRYPOINT ["/usr/bin/scl", "enable", "devtoolset-6", "--"]
CMD ["/usr/bin/scl", "enable", "devtoolset-6", "--", "/bin/bash"]
