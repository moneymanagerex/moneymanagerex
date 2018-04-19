FROM centos:7
MAINTAINER developers@moneymanagerex.org
RUN yum -y --enablerepo=extras install epel-release \
 && yum -y install --setopt=tsflags=nodocs \
        cmake3 gcc-c++ gettext git make rpm-build redhat-lsb-core ccache \
        webkitgtk3-devel wxGTK3-devel libcurl-devel \
 && yum clean all

# cmake3 installed from EPEL
RUN for f in cmake make pack test; do \
      ln -sv /usr/bin/c${f}3 /usr/bin/c${f}; done

ENV MMEX_INST_CMD yum -y install ./mmex-*.rpm
